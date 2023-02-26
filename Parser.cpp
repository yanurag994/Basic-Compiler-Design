#include "./Parser.hpp"
#include <sstream>
#include <string>

using namespace std;

bool Parser::scan_assume(token_type type){
  if (cur_tk.type == type)
  {
    std::cout << "Parsed token " << cur_tk.type << std::endl;
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
    std::cout << "Parsed token " << cur_tk.type << std::endl;
    cur_tk = lexer_handle.scan();
    return true;
  }
  return false;
}

bool Parser::program(){
  if (program_header())
    program_body();
  if (scan_assume(T_EOF))
    return true;
  return false;
}

bool Parser::resync(token_type type){
  while (cur_tk.type != type && cur_tk.type != T_EOF)
    cur_tk = lexer_handle.scan();
  if (cur_tk.type == type)
  {
    cur_tk = lexer_handle.scan();
    std::cout << "Skipping till token " << cur_tk.type << std::endl;
    return true;
  }
  std::cout << "Unable to continue parsing. Reached EOF while searching for resync token" << std::endl;
  return false;
}

bool Parser::program_header()
{
  if (scan_assume(PROGRAM_RW) && scan_assume(IDENTIFIER) && scan_assume(IS_RW))
    return true;
  else
    return resync(IS_RW);
}

bool Parser::program_body()
{
  while (declaration())
    ;
  if (lexer_handle.scan().type != BEGIN_RW)
    return false;
  while (statement() && lexer_handle.scan().type == ';')
    ;
  if (lexer_handle.scan().type == END_RW)
    if (lexer_handle.scan().type == PROGRAM_RW)
      return true;
  return false;
}

bool Parser::declaration()
{
  optional_scan_assume(GLOBAL_RW);
  if (optional_scan_assume(PROCEDURE_RW) && procedure_declaration())
    return true;
  if (optional_scan_assume(VARIABLE_RW) && variable_declaration() && scan_assume((token_type)';'))
    return true;
  return false;
}

bool Parser::variable_declaration()
{
  if (scan_assume(IDENTIFIER) && scan_assume(TYPE_SEPERATOR) && type_mark())
  {
    if (optional_scan_assume((token_type)'['))
    {
      if (scan_assume(INTEGER_VAL) && scan_assume((token_type)']'))
        return true;
      else
        return resync((token_type)']');
    }
    return true;
  }
  else
    return false;
}

bool Parser::procedure_header()
{
  if (scan_assume(IDENTIFIER) && scan_assume(TYPE_SEPERATOR) && type_mark() && scan_assume((token_type)'('))
  {
    parameter_list();
    if (scan_assume((token_type)')'))
      return true;
    return resync((token_type)')');
  }
  else
    return resync((token_type)')');
}

bool Parser::procedure_declaration()
{
  if (procedure_header())
  {
    if (procedure_body())
    {
      std::cout << "Passed procedure body";
      return true;
    }
  }
  return false;
}

bool Parser::type_mark()
{
  if (optional_scan_assume(INTEGER_RW) || optional_scan_assume(FLOAT_RW) || optional_scan_assume(STRING_RW) || optional_scan_assume(BOOLEAN_RW))
    return true;
  lexer_handle.reportError("Expected a type identifier");
  return false;
}

bool Parser::parameter()
{
  if (variable_declaration())
    return true;
  return false;
}

bool Parser::parameter_list()
{ // Need corrections
  if (scan_assume(VARIABLE_RW) && parameter())
  {
    if (optional_scan_assume((token_type)','))
      if (parameter_list())
        return true;
    return false;
  }
  else
  {
    return false;
  }
}

bool Parser::procedure_body()
{
  while (declaration());
  if (scan_assume(BEGIN_RW))
    while (statement() && scan_assume((token_type)';'))
      ;
  if (scan_assume(END_RW) && scan_assume(PROCEDURE_RW) && scan_assume((token_type)';'))
    return true;
  return false;
}

bool Parser::expression()
{
  scan_assume(IDENTIFIER);
  return true;
}

bool Parser::argument_list()
{
  if (expression())
    if (lexer_handle.scan().type != ',')
    {
      return true;
    }
    else
    {
      if (argument_list())
        return true;
    }
  return false;
}

bool Parser::destination()
{
  if (lexer_handle.scan().type != IDENTIFIER)
  {

    return false;
  }
  while (expression())
    ;
  return true;
}

bool Parser::assignment_statement()
{
  if (!destination())
    if (lexer_handle.scan().type == EQUAL_ASSIGN)
      if (expression())
        return true;
  return false;
}

bool Parser::if_statement()
{
  if (scan_assume((token_type)'(') && cond_expression() && scan_assume((token_type)')') && scan_assume(THEN_RW)){
    while (statement() && scan_assume((token_type)';'));
    if (optional_scan_assume(ELSE_RW))
      while (statement() && scan_assume((token_type)';'));
    if (scan_assume(END_RW)&&scan_assume(IF_RW)&&scan_assume((token_type)';'))
      return true;
  }
  return false;
}

bool Parser::cond_expression()
{
  return true;
}
bool Parser::loop_statement()
{
  if (lexer_handle.scan().type != FOR_RW)
  {

    return false;
  }
  if (assignment_statement() && lexer_handle.scan().type == ';')
  {
    while (statement() && lexer_handle.scan().type == ';')
      ;
    if (lexer_handle.scan().type == END_RW && lexer_handle.scan().type == FOR_RW)
      return true;
  }
  return false;
}

bool Parser::return_statement()
{
  if (expression() && scan_assume((token_type)';'))
    return true;
  return false;
}

bool Parser::procedure_call()
{
  if (scan_assume(IDENTIFIER) && argument_list())
    return true;
  return false;
}

bool Parser::statement()
{
  if(optional_scan_assume(IF_RW)&&if_statement())
    return true;
  if(optional_scan_assume(FOR_RW)&&loop_statement())
    return true;
  if(optional_scan_assume(RETURN_RW)&& return_statement())
    return true;
  if(optional_scan_assume(IDENTIFIER)&& assignment_statement())
    return true;
  return false;
}

