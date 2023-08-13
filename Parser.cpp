#include "./Parser.hpp"
#include <sstream>
#include <string>
#include <unistd.h>

bool Parser::scan_assume(token_type type, token &returned)
{
  if (cur_tk.type == type)
  {
    if (cur_tk.type == IDENTIFIER)
    {
      returned = symbols->HashLookup(cur_tk, global_flag);
    }
    if (cur_tk.type == INTEGER_VAL)
      std::cout << "Parsed token " << cur_tk.tokenMark.intValue << std::endl;
    else if (cur_tk.type == FLOAT_VAL)
      std::cout << "Parsed token " << cur_tk.tokenMark.doubleValue << std::endl;
    else if (cur_tk.type > 255) // Reserved words
      std::cout << "Parsed token " << cur_tk.tokenMark.stringValue << std::endl;
    else if (cur_tk.type < 255) // Single Chars
      std::cout << "Parsed token " << (char)cur_tk.type << std::endl;
    if (type != (token_type)'.')
      cur_tk = lexer_handle.scan();
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

bool Parser::optional_scan_assume(token_type type, token &returned)
{
  if (cur_tk.type == type)
  {
    if (cur_tk.type == IDENTIFIER)
    {
      returned = symbols->HashLookup(cur_tk, global_flag);
    }
    if (cur_tk.type == 275)
      std::cout << "Parsed token " << cur_tk.tokenMark.intValue << std::endl;
    else if (cur_tk.type == 276)
      std::cout << "Parsed token " << cur_tk.tokenMark.doubleValue << std::endl;
    else if (cur_tk.type > 255)
      std::cout << "Parsed token " << cur_tk.tokenMark.stringValue << std::endl;
    else if (cur_tk.type < 255)
      std::cout << "Parsed token " << (char)cur_tk.type << std::endl;
    if (type != (token_type)'.')
      cur_tk = lexer_handle.scan();
    return true;
  }
  return false;
}

bool Parser::scan_assume(token_type type)
{
  if (cur_tk.type == type)
  {
    if (cur_tk.type == INTEGER_VAL)
      std::cout << "Parsed token " << cur_tk.tokenMark.intValue << std::endl;
    else if (cur_tk.type == FLOAT_VAL)
      std::cout << "Parsed token " << cur_tk.tokenMark.doubleValue << std::endl;
    else if (cur_tk.type > 255) // Reserved words
      std::cout << "Parsed token " << cur_tk.tokenMark.stringValue << std::endl;
    else if (cur_tk.type < 255) // Single Chars
      std::cout << "Parsed token " << (char)cur_tk.type << std::endl;
    if (type != (token_type)'.')
      cur_tk = lexer_handle.scan();
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
    if (cur_tk.type == 275)
      std::cout << "Parsed token " << cur_tk.tokenMark.intValue << std::endl;
    else if (cur_tk.type == 276)
      std::cout << "Parsed token " << cur_tk.tokenMark.doubleValue << std::endl;
    else if (cur_tk.type > 255)
      std::cout << "Parsed token " << cur_tk.tokenMark.stringValue << std::endl;
    else if (cur_tk.type < 255)
      std::cout << "Parsed token " << (char)cur_tk.type << std::endl;
    if (type != (token_type)'.')
      cur_tk = lexer_handle.scan();
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

bool Parser::typeCheck(token *first, token *second, token_type op)
{
  return true;
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
    while (statement() && scan_assume((token_type)';'))
      ;
    return scan_assume(END_RW) && scan_assume(PROGRAM_RW);
  }
  return resync(PROGRAM_RW);
}

bool Parser::declaration()
{
  global_flag = optional_scan_assume(GLOBAL_RW);
  if (optional_scan_assume(PROCEDURE_RW) && procedure_declaration())
    return true;
  token var;
  if (optional_scan_assume(VARIABLE_RW) && variable_declaration(var) && scan_assume((token_type)';'))
  {
    symbols->Completetoken(var);
    return true;
  }
  return false;
}

bool Parser::procedure_declaration() // Complete
{
  return (procedure_header() && procedure_body()) ? true : resync(PROCEDURE_RW, true);
}

bool Parser::procedure_header()
{
  token proc;
  if (scan_assume(IDENTIFIER, proc) && scan_assume(TYPE_SEPERATOR) && type_mark(proc.dataType))
  {
    symbols->enterScope();
    if (scan_assume((token_type)'(') && parameter_list(proc.argType) && scan_assume((token_type)')'))
    {
      symbols->CompleteDeclPrevtoken(proc);
      std::cout << "In proc decl tokenHash is " << proc.tokenHash << " for " << proc.tokenMark.stringValue << " with dataType" << proc.dataType;
      for (auto i : proc.argType)
        std::cout << " arg " << i.tokenMark.stringValue << " with Hash " << i.tokenHash << " is of dtype " << i.dataType;
      std::cout << std::endl;
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
  while (declaration())
    ;
  if (scan_assume(BEGIN_RW))
    while (statement() && scan_assume((token_type)';'))
      ;
  if (scan_assume(END_RW) && scan_assume(PROCEDURE_RW) && scan_assume((token_type)';'))
  {
    symbols->exitScope();
    return true;
  }
  lexer_handle.reportWarning("Resync not possible at this stage");
  return false;
}

bool Parser::variable_declaration(token &var)
{
  if (scan_assume(IDENTIFIER, var) && scan_assume(TYPE_SEPERATOR) && type_mark(var.dataType))
  {
    std::cout << "In var decl tokenHash is " << var.tokenHash << " for " << var.tokenMark.stringValue << " with dataType" << var.dataType << std::endl;
    if (optional_scan_assume((token_type)'['))
    {
      auto exp = expression();
      // std::cout << var->tokenMark.stringValue << " " << var->dataType<< " len " << var->size<<std::endl;
      return (exp && scan_assume((token_type)']')) ? true : resync((token_type)']');
    }
    // std::cout << var->tokenMark.stringValue << " " << var->dataType<<std::endl;
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
  token var;
  if (optional_scan_assume(IF_RW) && if_statement())
    return true;
  if (optional_scan_assume(FOR_RW) && loop_statement())
    return true;
  if (optional_scan_assume(RETURN_RW) && return_statement())
    return true;
  if (optional_scan_assume(IDENTIFIER, var))
  {
    std::cout << "In statement tokenHash is " << var.tokenHash << " for " << var.tokenMark.stringValue << " with dataType" << var.dataType << std::endl;
    if (optional_scan_assume(EQUAL_ASSIGN) && assignment_statement())
      return true;
    if (optional_scan_assume((token_type)'[') && scan_assume(INTEGER_VAL) && scan_assume((token_type)']') && scan_assume(EQUAL_ASSIGN) && assignment_statement())
      return true;
    if (optional_scan_assume((token_type)'(') && procedure_call())
      return true;
  }
  return false;
}

bool Parser::if_statement()
{
  if (scan_assume((token_type)'(') && cond_expression() && scan_assume((token_type)')') && scan_assume(THEN_RW))
  {
    while (statement() && scan_assume((token_type)';'))
      ;
    if (optional_scan_assume(ELSE_RW))
      while (statement() && scan_assume((token_type)';'))
        ;
    if (scan_assume(END_RW) && scan_assume(IF_RW))
      return true;
  }
  return false;
}

bool Parser::expression()
{
  optional_scan_assume((token_type)'!');
  if (arithOp())
    return (optional_scan_assume((token_type)'|') || optional_scan_assume((token_type)'&')) && expression();
  else
    return true;
}

bool Parser::arithOp()
{
  if (relation())
    return (optional_scan_assume((token_type)'+') || optional_scan_assume((token_type)'-') || optional_scan_assume((token_type)'*') || optional_scan_assume((token_type)'/')) && arithOp();
  else
    return true;
}

bool Parser::relation()
{
  if (term())
  {
    if (optional_scan_assume(LESS_THAN) || optional_scan_assume(LESS_EQUAL) || optional_scan_assume(GREATER_EQUAL) || optional_scan_assume(GREATER_THAN) || optional_scan_assume(EQUALITY) || optional_scan_assume(NOT_EQUAL))
      return relation();
    else
      return true;
  }
  else
  {
    return false;
  }
}

bool Parser::term()
{
  return factor();
}

bool Parser::factor()
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
    else if (optional_scan_assume(IDENTIFIER, var) && destination())
    {
      std::cout << "In if condition tokenHash is " << var.tokenHash << " for " << var.tokenMark.stringValue << " with dataType" << var.dataType << std::endl;
      return true;
    }
    if (optional_scan_assume((token_type)'(') && expression() && scan_assume((token_type)')'))
      return true;
    return false;
  }
}

bool Parser::argument_list()
{
  return expression() ? (optional_scan_assume((token_type)',') ? argument_list() : true) : true;
}

bool Parser::destination()
{
  if (optional_scan_assume((token_type)'['))
  {
    while (expression())
      ;
    if (scan_assume((token_type)']'))
      return true;
  }
  if (optional_scan_assume((token_type)'('))
    procedure_call();
  return true;
}

bool Parser::assignment_statement()
{
  return expression();
}

bool Parser::cond_expression()
{
  return expression();
}

bool Parser::loop_statement()
{
  if (scan_assume((token_type)'(') && scan_assume(IDENTIFIER) && scan_assume(EQUAL_ASSIGN) && assignment_statement() && scan_assume((token_type)';') && expression() && scan_assume((token_type)')'))
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
  return expression();
}

bool Parser::procedure_call()
{
  return (optional_scan_assume((token_type)')') || (argument_list() && scan_assume((token_type)')')));
}