#include "./Parser.hpp"

bool Parser::type_mark()
{
  int type = lexer_handle.scan().type;
  if (type == INTEGER_RW || type == FLOAT_RW || type == STRING_RW || type == BOOLEAN_RW)
    return true;
  return false;
}

bool Parser::variable_declaration()
{
  if (lexer_handle.scan().type == VARIABLE_RW)
  {
    if (lexer_handle.scan().type == IDENTIFIER)
      if (lexer_handle.scan().type == TYPE_SEPERATOR)
        if (type_mark())
          if (lexer_handle.scan().type == '[')
          {
            if (lexer_handle.scan().type == INTEGER_VAL)
              if (lexer_handle.scan().type == ']')
                return true;
            return false;
          }
          else
          {
            unlexer_handle.scan();
            return true;
          }
  }
  else
  {
    unlexer_handle.scan();
  }
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
  if (parameter())
    return true;
  if (lexer_handle.scan().type == ',')
  {
    if (parameter_list())
      return true;
    return false;
  }
  else
  {
    unlexer_handle.scan();
    return false;
  }
}

bool Parser::procedure_header()
{
  if (lexer_handle.scan().type == PROCEDURE_RW)
  {
    if (lexer_handle.scan().type == IDENTIFIER)
      if (lexer_handle.scan().type == TYPE_SEPERATOR)
        if (type_mark())
          if (lexer_handle.scan().type == (int)'(')
            if (parameter_list())
              if (lexer_handle.scan().type == (int)')')
                return true;
  }
  else
  {
    unlexer_handle.scan();
  }
  return false;
}

bool Parser::expression()
{
  return false;
}

bool Parser::argument_list()
{
  if (expression())
    if (lexer_handle.scan().type != ',')
    {
      unlexer_handle.scan();
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
    unlexer_handle.scan();
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
  if (lexer_handle.scan().type != IF_RW)
  {
    unlexer_handle.scan();
    return false;
  }
  if (lexer_handle.scan().type == '(')
    if (expression())
      if (lexer_handle.scan().type == THEN_RW)
        if (lexer_handle.scan().type == ')')
        {
          while (statement() && lexer_handle.scan().type == ';')
            ;
          if (lexer_handle.scan().type != ELSE_RW)
          {
            unlexer_handle.scan();
            if (lexer_handle.scan().type == END_RW)
              if (lexer_handle.scan().type == IF_RW)
                return true;
            return false;
          }
          while (statement() && lexer_handle.scan().type == ';')
            ;
          if (lexer_handle.scan().type == END_RW)
            if (lexer_handle.scan().type == IF_RW)
              return true;
          return false;
        }
  return false;
}

bool Parser::loop_statement()
{
  if (lexer_handle.scan().type != FOR_RW)
  {
    unlexer_handle.scan();
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
  if (lexer_handle.scan().type != RETURN_RW)
  {
    unlexer_handle.scan();
    return false;
  }
  if (expression())
    return true;
  return false;
}

bool Parser::procedure_call()
{
  if (lexer_handle.scan().type != IDENTIFIER)
  {
    unlexer_handle.scan();
    return false;
  }
  if ((argument_list()))
    return true;
  return false;
}

bool Parser::statement()
{
  if (assignment_statement() || if_statement() || loop_statement() || procedure_call() || return_statement())
    return true;
  return false;
}

bool Parser::procedure_body()
{
  while (declaration())
    std::cout << "Cool";
  if (lexer_handle.scan().type == BEGIN_RW)
    while (statement() && lexer_handle.scan().type == ';')
      ;
  if (lexer_handle.scan().type == END_RW && lexer_handle.scan().type == PROCEDURE_RW && lexer_handle.scan().type == ';')
    return true;
  return false;
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

bool Parser::program_header()
{
  if (lexer_handle.scan().type == PROGRAM_RW)
    if (lexer_handle.scan().type == IDENTIFIER)
      if (lexer_handle.scan().type == IS_RW)
        return true;
  return false;
}

bool Parser::declaration()
{
  if (lexer_handle.scan().type != GLOBAL_RW)
    unlexer_handle.scan();
  if (procedure_declaration())
    return true;
  if (variable_declaration() && lexer_handle.scan().type == ';')
    return true;
  return false;
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

bool Parser::program()
{
  if (program_header())
    program_body();
  if (lexer_handle.scan().type == EOF)
    return true;
  return false;
}