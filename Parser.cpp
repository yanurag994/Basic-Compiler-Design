#include "./Parser.hpp"
#include <sstream>
#include <string>
#include <unistd.h>

bool Parser::scan_assume(token_type type, token &returned, bool definition = false)
{
  if (optional_scan_assume(type, returned, definition))
  {
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
    else
      returned = cur_tk;
    if (type != (token_type)'.')
    {
      cur_tk = lexer_handle.scan();
    }
    return true;
  }
  return false;
}

bool Parser::scan_assume(token_type type)
{
  if (optional_scan_assume(type))
  {
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

bool Parser::typeCheck(token &first, token &second, token &result, token_type op, std::stringstream &tmp)
{
  std::string op1_Hash = getLLVMform(first), op2_Hash = getLLVMform(second);
  if (first.pointer)
  {
    op1_Hash = symbols->genTempvarHash();
    tmp << op1_Hash << " = load " << getLLVMType(first.dataType) << ", " << getLLVMType(first.dataType) << "* " << first.tokenHash << std::endl;
  }
  if (second.pointer)
  {
    op2_Hash = symbols->genTempvarHash();
    tmp << op2_Hash << " = load " << getLLVMType(second.dataType) << ", " << getLLVMType(second.dataType) << "* " << second.tokenHash << std::endl;
  }
  if ((op == (token_type)'+') && (first.dataType == INTEGER_RW || first.type == INTEGER_VAL) && (second.dataType == INTEGER_RW || second.type == INTEGER_VAL))
  {
    tmp << result.tokenHash << " = add i32 " << op1_Hash << ", " << op2_Hash << std::endl;
    return true; // Integer result
  }
  if ((op == (token_type)'-') && (first.dataType == INTEGER_RW || first.type == INTEGER_VAL) && (second.dataType == INTEGER_RW || second.type == INTEGER_VAL))
  {
    tmp << result.tokenHash << " = sub i32 " << op1_Hash << ", " << op2_Hash << std::endl;
    return true; // Integer result
  }
  if ((op == (token_type)'*') && (first.dataType == INTEGER_RW || first.type == INTEGER_VAL) && (second.dataType == INTEGER_RW || second.type == INTEGER_VAL))
  {
    tmp << result.tokenHash << " = mul i32 " << op1_Hash << ", " << op2_Hash << std::endl;
    return true; // Integer result
  }
  if ((op == (token_type)'/') && (first.dataType == INTEGER_RW || first.type == INTEGER_VAL) && (second.dataType == INTEGER_RW || second.type == INTEGER_VAL))
  {
    tmp << result.tokenHash << " = div i32 " << op1_Hash << ", " << op2_Hash << std::endl;
    return true; // Integer result
  }
  if ((op == (token_type)'+') && (first.dataType == FLOAT_VAL || first.dataType == FLOAT_RW || first.dataType == INTEGER_VAL || first.dataType == INTEGER_RW) && (second.dataType == FLOAT_VAL || second.dataType == FLOAT_RW || second.dataType == INTEGER_VAL || second.dataType == INTEGER_RW))
  {
    tmp << result.tokenHash << " = fadd float " << op1_Hash << ", " << op2_Hash << std::endl;
    return true; // Float result
  }
  if ((op == (token_type)'-') && (first.dataType == FLOAT_VAL || first.dataType == FLOAT_RW || first.dataType == INTEGER_VAL || first.dataType == INTEGER_RW) && (second.dataType == FLOAT_VAL || second.dataType == FLOAT_RW || second.dataType == INTEGER_VAL || second.dataType == INTEGER_RW))
  {
    tmp << result.tokenHash << " = fsub float " << op1_Hash << ", " << op2_Hash << std::endl;
    return true; // Float result
  }
  if ((op == (token_type)'*') && (first.dataType == FLOAT_VAL || first.dataType == FLOAT_RW || first.dataType == INTEGER_VAL || first.dataType == INTEGER_RW) && (second.dataType == FLOAT_VAL || second.dataType == FLOAT_RW || second.dataType == INTEGER_VAL || second.dataType == INTEGER_RW))
  {
    tmp << result.tokenHash << " = fmul float " << op1_Hash << ", " << op2_Hash << std::endl;
    return true; // Float result
  }
  if ((op == (token_type)'/') && (first.dataType == FLOAT_VAL || first.dataType == FLOAT_RW || first.dataType == INTEGER_VAL || first.dataType == INTEGER_RW) && (second.dataType == FLOAT_VAL || second.dataType == FLOAT_RW || second.dataType == INTEGER_VAL || second.dataType == INTEGER_RW))
  {
    tmp << result.tokenHash << " = fdic float " << op1_Hash << ", " << op2_Hash << std::endl;
    return true; // Float result
  }
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
    std::stringstream &curr_stream = decl.global_var ? global_decl : output;
    if (decl.global_var)
      curr_stream << "@" << decl.tokenMark.stringValue << " = global ";
    else
    {
      decl.pointer = true;
      curr_stream << decl.tokenHash << " = alloca ";
    }
    if (decl.size != -1)
      curr_stream << "[" << decl.size << " x " << getLLVMType(decl.dataType) << "]";
    else
      curr_stream << getLLVMType(decl.dataType);
    if (decl.global_var)
    {
      if (decl.size != -1)
        curr_stream << " zeroinitializer" << std::endl;
      else
        curr_stream << " " << getLLVMIntitializer(decl.dataType) << std::endl;
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
  return (procedure_header(proc) && procedure_body(proc)) ? true : resync(PROCEDURE_RW, true);
}

bool Parser::procedure_header(token &proc)
{
  if (scan_assume(IDENTIFIER, proc, true) && scan_assume(TYPE_SEPERATOR) && type_mark(proc.dataType))
  {
    proc.procedure = true;
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
        output << getLLVMType(i->dataType) << " " << i->tokenHash << "_arg";
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
    arg.pointer = true;
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

bool Parser::procedure_body(token &proc)
{
  output << "{" << std::endl;
  for (auto i : proc.argType)
  {
    output << i.tokenHash << " = alloca ";
    std::stringstream temp;
    if (i.size != -1)
      temp << "[" << i.size << " x " << getLLVMType(i.dataType) << "]";
    else
      temp << getLLVMType(i.dataType);
    output << temp.str() << std::endl;
    ;
    output << "store " << temp.str() << " " << i.tokenHash << "_arg, " << temp.str() << "* " << i.tokenHash << std::endl;
  }
  while (declaration())
    ;
  if (scan_assume(BEGIN_RW))
  {
    for (auto i : symbols->global->symbol_table)
    {
      if (!i.second.procedure)
      {
        output << i.second.tokenHash << " = load " << getLLVMType(i.second.dataType) << ", " << getLLVMType(i.second.dataType) << "* @" << i.second.tokenMark.stringValue << std::endl;
      }
    }
    while (statement() && scan_assume((token_type)';'))
      ;
  }
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
  if (optional_scan_assume(IDENTIFIER, var) && assignment_statement(var))
    return true;
  return false;
}

bool Parser::if_statement()
{
  token result; // Declare location to hold result of conditional expression
  if (scan_assume((token_type)'(') && cond_expression(result) && scan_assume((token_type)')') && scan_assume(THEN_RW))
  {
    int if_label = label_gen++, else_no_label = label_gen++;
    output << "br i1 " << result.tokenHash << ", label %" << if_label << ", label %" << else_no_label << std::endl;
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
      token result;
      if (expression(result, exp))
      {
        output << exp.str();
        if (result.type == IDENTIFIER)
        {
          if (result.pointer)
          {
            std::string temp_Hash = symbols->genTempvarHash();
            output << temp_Hash << " = load " << getLLVMType(result.dataType) << ", " << getLLVMType(result.dataType) << "* " << result.tokenHash << std::endl;
            output << "store " << getLLVMType(result.dataType) << " " << temp_Hash << ", " << getLLVMType(dest.dataType) << "* " << dest.tokenHash << std::endl;
          }
          else
          {
            output << "store " << getLLVMType(result.dataType) << " " << result.tokenHash << ", " << getLLVMType(dest.dataType) << "* " << dest.tokenHash << std::endl;
          }
        }
        else if (result.type == INTEGER_VAL)
          output
              << "store " << getLLVMType(result.type) << " " << result.tokenMark.intValue << ", " << getLLVMType(dest.dataType) << "* " << dest.tokenHash << std::endl;
        else if (result.type == FLOAT_VAL)
          output
              << "store " << getLLVMType(result.type) << " " << result.tokenMark.doubleValue << ", " << getLLVMType(dest.dataType) << "* " << dest.tokenHash << std::endl;
        else if (result.type == STRING_VAL)
          output
              << "store " << getLLVMType(result.type) << " " << result.tokenMark.stringValue << ", " << getLLVMType(dest.dataType) << "* " << dest.tokenHash << std::endl;
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
    token result;
    while (expression(result, exp))
      ;
    if (scan_assume((token_type)']'))
      return true;
  }
  return true;
}

bool Parser::loop_statement()
{
  token var, exp_result;
  std::stringstream exp;
  if (scan_assume((token_type)'(') && scan_assume(IDENTIFIER, var) && assignment_statement(var) && scan_assume((token_type)';') && expression(exp_result, exp) && scan_assume((token_type)')'))
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
  token result;
  if (expression(result, exp))
  {
    output << exp.str();
    if (result.type == IDENTIFIER)
    {
      if (result.pointer)
      {
        std::string temp_Hash = symbols->genTempvarHash();
        output << temp_Hash << " = load " << getLLVMType(result.dataType) << ", " << getLLVMType(result.dataType) << "* " << result.tokenHash << std::endl;
        output << "ret " << getLLVMType(result.dataType) << " " << temp_Hash << std::endl;
      }
      else
        output << "ret " << getLLVMType(result.dataType) << " " << result.tokenHash << std::endl;
    }
    else if (result.type == INTEGER_VAL)
      output << "ret " << getLLVMType(result.type) << " " << result.tokenMark.intValue << std::endl;
    else if (result.type == FLOAT_VAL)
      output << "ret " << getLLVMType(result.type) << " " << result.tokenMark.doubleValue << std::endl;
    else if (result.type == STRING_VAL)
      output << "ret " << getLLVMType(result.type) << " " << result.tokenMark.stringValue << std::endl;
    return true;
  }
  return false;
}

bool Parser::procedure_call(token &proc, std::stringstream &call)
{
  token result;
  result = proc;
  result.tokenHash = symbols->genTempvarHash();
  if (optional_scan_assume((token_type)')') || (argument_list(proc, call) && scan_assume((token_type)')')))
  {
    call << result.tokenHash << " = call " << getLLVMType(proc.dataType) << " @" << proc.tokenMark.stringValue << "(";
    call << ")" << std::endl;
    proc = result;
    return true;
  }
  else
    return false;
}

bool Parser::argument_list(token &proc, std::stringstream &args) // Processes for procedure call must perfrom type checking
{
  token result;
  if (expression(result, args))
  {
    if (optional_scan_assume((token_type)','))
    {
      return argument_list(proc, args);
    }
    else
    {
      return true;
    }
  }
  else
  {
    return true;
  }
}

bool Parser::cond_expression(token &result)
{
  std::stringstream exp;
  if (expression(result, exp))
  {
    output << exp.str() << std::endl;
    return true;
  }
  else
    return false;
}

bool Parser::expression(token &exp_result, std::stringstream &exp)
{
  optional_scan_assume((token_type)'!'); // Might be replaced if not is a keyword
  if (arithOp(exp_result, exp))
  {
    if (optional_scan_assume((token_type)'|') || optional_scan_assume((token_type)'&'))
      return expression(exp_result, exp);
    else
      return true;
  }
  else
    return true;
}

bool Parser::arithOp(token &var, std::stringstream &exp, token_type prev_op)
{
  token var_1, var_2, op;
  if (relation(var_1, exp))
  {
    token result;
    if (var.type != UNKNOWN)
    {
      result.tokenHash = symbols->genTempvarHash();
      result.type = IDENTIFIER;
      result.dataType = INTEGER_RW;
      typeCheck(var, var_1, result, prev_op, exp);
    }
    else
    {
      result = var_1;
    }
    if ((optional_scan_assume((token_type)'+', op) || optional_scan_assume((token_type)'-', op) || optional_scan_assume((token_type)'*', op) || optional_scan_assume((token_type)'/', op)))
    {
      if (arithOp(result, exp, op.type))
      {
        var = result;
        return true;
      }
      else
      {
        var = result;
        return false;
      }
    }
    else
    {
      var = result;
      return true;
    }
  }
  else
    return false;
}

bool Parser::relation(token &var, std::stringstream &exp)
{
  token var_1, var_2, op;
  if (factor(var_1, exp))
  {
    if (optional_scan_assume((token_type)'<', op) || optional_scan_assume(LESS_EQUAL, op) || optional_scan_assume(GREATER_EQUAL, op) || optional_scan_assume((token_type)'>', op) || optional_scan_assume(EQUALITY, op) || optional_scan_assume(NOT_EQUAL, op))
    {
      // evaluate resultant token values and types and return that
      if (relation(var_2, exp))
      {
        var.type = IDENTIFIER;
        var.tokenHash = symbols->genTempvarHash();
        var.dataType = BOOLEAN_RW;
        std::string op1_Hash = getLLVMvar_val(var_1), op2_Hash = getLLVMvar_val(var_2);
        if (var_1.pointer)
        {
          op1_Hash = symbols->genTempvarHash();
          exp << op1_Hash << " = load " << getLLVMType(var_1.dataType) << ", " << getLLVMType(var_1.dataType) << "* " << var_1.tokenHash << std::endl;
        }
        if (var_2.pointer)
        {
          op2_Hash = symbols->genTempvarHash();
          exp << op2_Hash << " = load " << getLLVMType(var_2.dataType) << ", " << getLLVMType(var_2.dataType) << "* " << var_2.tokenHash << std::endl;
        }
        exp << var.tokenHash << " = icmp " << getLLVMop(op) << " " << getLLVMType(var_1.dataType) << " " << op1_Hash << ", " << op2_Hash;
        return true;
      }
      else
        return false;
    }
    else
    {
      var = var_1;
      return true;
    }
  }
  else
  {
    return false;
  }
}

bool Parser::factor(token &var, std::stringstream &exp)
{

  if (optional_scan_assume(TRUE_RW, var) || optional_scan_assume(FALSE_RW, var) || optional_scan_assume(STRING_VAL, var))
    return true;
  else
  {
    optional_scan_assume((token_type)'-');
    if (optional_scan_assume(INTEGER_VAL, var) || optional_scan_assume(FLOAT_VAL, var))
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
