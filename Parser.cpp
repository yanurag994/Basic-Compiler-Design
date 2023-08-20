#include "./Parser.hpp"
#include <sstream>
#include <string>
#include <unistd.h>

bool Parser::scan_assume(token_type type, token &returned, bool definition = false)
{
  if (cur_tk.type == type)
  {
    if (cur_tk.type == IDENTIFIER)
      returned = symbols->HashLookup(cur_tk, global_flag, definition);
    if (cur_tk.type == INTEGER_VAL || cur_tk.type == FLOAT_VAL)
      returned = cur_tk;
    if (type != (token_type)'.')
    {
      prev_tk = cur_tk;
      cur_tk = lexer_handle.scan();
    }
    return true;
  }
  else
  {
    std::stringstream ss;
    ss << "At token " << cur_tk.type << ", expecting token " << type << std::endl;
    lexer_handle.reportError(ss.str());
    return false;
  }
}

bool Parser::optional_scan_assume(token_type type, token &returned, bool definition = false)
{
  if (cur_tk.type == type)
  {
    if (cur_tk.type == IDENTIFIER)
      returned = symbols->HashLookup(cur_tk, global_flag, definition);
    if (cur_tk.type == INTEGER_VAL || cur_tk.type == FLOAT_VAL)
      returned = cur_tk;
    if (type != (token_type)'.')
    {
      prev_tk = cur_tk;
      cur_tk = lexer_handle.scan();
    }
    return true;
  }
  return false;
}

bool Parser::scan_assume(token_type type)
{
  if (cur_tk.type == type)
  {
    if (type != (token_type)'.')
    {
      prev_tk = cur_tk;
      cur_tk = lexer_handle.scan();
    }
    return true;
  }
  else
  {
    std::stringstream ss;
    ss << "At token " << cur_tk.type << ", expecting token " << type << std::endl;
    lexer_handle.reportError(ss.str());
    return false;
  }
}

bool Parser::optional_scan_assume(token_type type)
{
  if (cur_tk.type == type)
  {
    if (type != (token_type)'.')
    {
      prev_tk = cur_tk;
      cur_tk = lexer_handle.scan();
    }
    return true;
  }
  return false;
}

bool Parser::resync(token_type type, bool ahead = false)
{
  while (cur_tk.type != type && cur_tk.type != T_EOF)
    cur_tk = lexer_handle.scan();
  if (cur_tk.type == type)
  {
    cur_tk = lexer_handle.scan();
    std::cout << "Skipping till token " << cur_tk.type << std::endl;
    return true;
  }
  std::cout << "Unable to continue parsing. Reached EOF while attempting to resync" << std::endl;
  return false;
}

bool Parser::typeCheck(token &first, token &second, token_type op)
{
  if ((op == (token_type)'+' || op == (token_type)'-') && (first.dataType == INTEGER_VAL || first.dataType == INTEGER_RW) && (second.dataType == INTEGER_VAL || second.dataType == INTEGER_RW))
    return true; // Integer result
  if ((op == (token_type)'+' || op == (token_type)'-') && (first.dataType == FLOAT_VAL || first.dataType == FLOAT_RW || first.dataType == INTEGER_VAL || first.dataType == INTEGER_RW) && (second.dataType == FLOAT_VAL || second.dataType == FLOAT_RW || second.dataType == INTEGER_VAL || second.dataType == INTEGER_RW))
    return true; // Float result
  return false;
}

bool Parser::typeCheck(token &first, token &second, token &result, token_type op)
{
  if ((op == (token_type)'+' || op == (token_type)'-') && (first.dataType == INTEGER_VAL || first.dataType == INTEGER_RW) && (second.dataType == INTEGER_VAL || second.dataType == INTEGER_RW))
    return true; // Integer result
  if ((op == (token_type)'+' || op == (token_type)'-') && (first.dataType == FLOAT_VAL || first.dataType == FLOAT_RW || first.dataType == INTEGER_VAL || first.dataType == INTEGER_RW) && (second.dataType == FLOAT_VAL || second.dataType == FLOAT_RW || second.dataType == INTEGER_VAL || second.dataType == INTEGER_RW))
    return true; // Float result
  return false;
}

bool Parser::program() // Complete
{
  return (program_header() && program_body() && scan_assume((token_type)'.')) ? true : false;
}

bool Parser::program_header() // Complete
{
  return (scan_assume(PROGRAM_RW) && scan_assume(IDENTIFIER) && scan_assume(IS_RW)) ? true : resync(IS_RW);
}

bool Parser::program_body() // Complete
{
  while (declaration())
    ;
  if (scan_assume(BEGIN_RW))
  {
    output << "define i32 @main() {" << std::endl;
    while (statement() && scan_assume((token_type)';'))
      ;
    if (scan_assume(END_RW) && scan_assume(PROGRAM_RW))
    {
      output << "ret i32 0" << std::endl
             << "}" << std::endl;
      return true;
    }
    else
      return false;
  }
  return resync(PROGRAM_RW);
}

bool Parser::declaration()
{
  global_flag = optional_scan_assume(GLOBAL_RW);
  token decl;
  if (optional_scan_assume(PROCEDURE_RW) && procedure_declaration(decl))
    return true;
  if (optional_scan_assume(VARIABLE_RW) && variable_declaration(decl) && scan_assume((token_type)';'))
  {
    if (decl.global_var)
      output << "@" << decl.tokenMark.stringValue << " = global ";
    else
      output << "%lb_" << decl.tokenHash << " = alloca ";
    if (decl.size != -1)
      output << "[" << decl.size << " x " << getLLVMType(decl.dataType) << "]";
    else
      output << getLLVMType(decl.dataType);
    if (decl.global_var)
    {
      if (decl.size != -1)
        output << " zeroinitializer" << std::endl;
      else
        output << " " << getLLVMIntitializer(decl.dataType) << std::endl;
    }
    else
      output << std::endl;
    symbols->Completetoken(decl);
    return true;
  }
  return false;
}

bool Parser::procedure_declaration(token &proc) // Complete
{
  return (procedure_header(proc) && procedure_body()) ? true : resync(PROCEDURE_RW, true);
}

bool Parser::procedure_header(token &proc)
{
  if (scan_assume(IDENTIFIER, proc, true) && scan_assume(TYPE_SEPERATOR) && type_mark(proc.dataType))
  {
    symbols->enterScope();
    std::stringstream temp;
    buffer.push_back(std::move(temp));
    output.swap(buffer[buffer.size() - 2]);
    output << "define " << getLLVMType(proc.dataType) << " @" << proc.tokenMark.stringValue << "(";
    if (scan_assume((token_type)'(') && parameter_list(proc.argType) && scan_assume((token_type)')'))
    {
      symbols->CompleteDeclPrevtoken(proc); // For outer Scope
      symbols->Completetoken(proc);         // To allow recursion
      for (auto i = proc.argType.begin(); i != proc.argType.end(); ++i)
      {
        output << getLLVMType(i->dataType) << " %lb_" << i->tokenHash;
        if (i != std::prev(proc.argType.end()))
          output << ",";
      }
      output << ")" << std::endl;
      return true;
    }
    return false;
  }
  else
    return resync((token_type)')');
}

bool Parser::parameter_list(std::vector<token> &argType)
{
  if (optional_scan_assume(VARIABLE_RW))
  {
    token arg;
    auto valid = parameter(arg);
    symbols->Completetoken(arg);
    argType.push_back(arg);
    if (optional_scan_assume((token_type)','))
      return parameter_list(argType);
    return true;
  }
  else
    return true;
}

bool Parser::parameter(token &param)
{
  auto temp = variable_declaration(param);
  return temp;
}

bool Parser::procedure_body()
{
  output << "{" << std::endl;
  while (declaration())
    ;
  if (scan_assume(BEGIN_RW))
    while (statement() && scan_assume((token_type)';'))
      ;
  if (scan_assume(END_RW) && scan_assume(PROCEDURE_RW) && scan_assume((token_type)';'))
  {
    symbols->exitScope();
    output << "}" << std::endl;
    buffer[0] << output.str();
    buffer.pop_back();
    output.str("");
    output.swap(buffer[buffer.size() - 1]);
    return true;
  }
  lexer_handle.reportWarning("Resync not possible at this stage");
  return false;
}

bool Parser::variable_declaration(token &var)
{
  if (scan_assume(IDENTIFIER, var, true) && scan_assume(TYPE_SEPERATOR) && type_mark(var.dataType))
  {
    token temp;
    if (optional_scan_assume((token_type)'[') && scan_assume(INTEGER_VAL, temp))
    {
      var.size = temp.tokenMark.intValue;
      return (scan_assume((token_type)']')) ? true : resync((token_type)']');
    }
    return true;
  }
  else
    return false;
}

bool Parser::type_mark(token_type &dType)
{
  token returned;
  if (optional_scan_assume(INTEGER_RW, returned) || optional_scan_assume(FLOAT_RW, returned) || optional_scan_assume(STRING_RW, returned) || optional_scan_assume(BOOLEAN_RW, returned))
  {
    dType = returned.type;
    return true;
  }
  else
  {
    lexer_handle.reportError("Expected a type identifier");
    return false;
  }
}

bool Parser::statement()
{
  if (optional_scan_assume(IF_RW) && if_statement())
    return true;
  if (optional_scan_assume(FOR_RW) && loop_statement())
    return true;
  if (optional_scan_assume(RETURN_RW) && return_statement())
    return true;
  token var;
  std::stringstream temp;
  if (optional_scan_assume(IDENTIFIER, var))
  {
    if (optional_scan_assume((token_type)'(') && procedure_call(var,temp))
      return true;
    if (assignment_statement(var))
      return true;
  }
  return false;
}

bool Parser::if_statement()
{
  std::string result_tag; // Declare location to hold result of conditional expression
  if (scan_assume((token_type)'(') && cond_expression(result_tag) && scan_assume((token_type)')') && scan_assume(THEN_RW))
  {
    int if_label = label_gen++, else_no_label = label_gen++;
    output << "br i1 " << result_tag << ", label %" << if_label << ", label %" << else_no_label << std::endl;
    output << std::endl
           << "; <label>:" << if_label << std::endl;
    while (statement() && scan_assume((token_type)';'))
      ;
    output << std::endl
           << "; <label>:" << else_no_label << std::endl;
    if (optional_scan_assume(ELSE_RW))
      while (statement() && scan_assume((token_type)';'))
        ;
    if (scan_assume(END_RW) && scan_assume(IF_RW))
      return true;
  }
  return false;
}

bool Parser::assignment_statement(token &dest)
{
  if (destination(dest))
  {
    if (scan_assume(EQUAL_ASSIGN))
    {
      std::stringstream exp;
      if (expression(exp))
      {
        output << "%lb_" << dest.tokenHash << " = " << exp.str() << std::endl;
        return true;
      }
    }
  }
  return true;
}

bool Parser::destination(token &var)
{
  if (optional_scan_assume((token_type)'['))
  {
    std::stringstream exp;
    while (expression(exp))
      ;
    if (scan_assume((token_type)']'))
      return true;
  }
  return true;
}

bool Parser::loop_statement()
{
  token var;
  std::stringstream exp;
  if (scan_assume((token_type)'(') && scan_assume(IDENTIFIER, var) && assignment_statement(var) && scan_assume((token_type)';') && expression(exp) && scan_assume((token_type)')'))
  {
    while (statement() && scan_assume((token_type)';'))
      ;
    if (scan_assume(END_RW) && scan_assume(FOR_RW))
      return true;
  }
  return false;
}

bool Parser::return_statement()
{
  std::stringstream exp;
  if (expression(exp))
  {
    // output << "ret " << getLLVMType() << std::endl;
    return true;
  }
  return false;
}

bool Parser::procedure_call(token &proc, std::stringstream &call)
{
  if (optional_scan_assume((token_type)')') || (argument_list(proc) && scan_assume((token_type)')')))
  {
    call << "call " << getLLVMType(proc.dataType) << " @ " << proc.tokenMark.stringValue << " (";
    return true;
  }
  else
    return false;
}

bool Parser::cond_expression(std::string &result_tag)
{
  result_tag = "%lb_" + std::to_string(symbols->Hashgen++);
  // output << result_tag << std::endl;
  std::stringstream exp;
  return expression(exp);
}

bool Parser::expression(std::stringstream &exp)
{
  optional_scan_assume((token_type)'!'); // Might be replaced if not is a keyword
  if (arithOp(exp))
    return (optional_scan_assume((token_type)'|') || optional_scan_assume((token_type)'&')) && expression(exp);
  else
    return true;
}

bool Parser::arithOp(std::stringstream & exp)
{
  if (relation(exp))
    return (optional_scan_assume((token_type)'+') || optional_scan_assume((token_type)'-') || optional_scan_assume((token_type)'*') || optional_scan_assume((token_type)'/')) && arithOp(exp);
  else
    return true;
}

bool Parser::relation(std::stringstream &exp)
{
  if (term(exp))
  {
    if (optional_scan_assume(LESS_THAN) || optional_scan_assume(LESS_EQUAL) || optional_scan_assume(GREATER_EQUAL) || optional_scan_assume(GREATER_THAN) || optional_scan_assume(EQUALITY) || optional_scan_assume(NOT_EQUAL))
    {
      return relation(exp);
    }
    else
      return true;
  }
  else
  {
    return false;
  }
}

bool Parser::term(std::stringstream &exp)
{
  return factor(exp);
}

bool Parser::factor(std::stringstream &exp)
{
  if (optional_scan_assume(TRUE_RW) || optional_scan_assume(FALSE_RW) || optional_scan_assume(STRING_VAL))
    return true;
  else
  {
    optional_scan_assume((token_type)'-');
    token var;
    if (optional_scan_assume(INTEGER_VAL) || optional_scan_assume(FLOAT_VAL))
    {
      return true;
    }
    else if (optional_scan_assume(IDENTIFIER, var))
    {
      if (optional_scan_assume((token_type)'(') && procedure_call(var, exp))
        return true;
      if (destination(var))
        return true;
      return false;
    }
    return false;
  }
}

bool Parser::argument_list(token &var) // Processes for procedure call must perfrom type checking
{
  for (auto exp_arg : var.argType)
    std::cout << exp_arg.type;
  std::cout << std::endl;
  std::stringstream exp;
  return expression(exp) ? (optional_scan_assume((token_type)',') ? argument_list(var) : true) : true;
}