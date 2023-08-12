#include "./Parser.hpp"
#include <sstream>
#include <string>
#include <unistd.h>

void Symbols::enterScope()
{
  current = new Scope(std::map<std::string, token>(), current);
}

void Symbols::enterSoftScope()
{
  current = new Scope(current->symbol_table, current);
}

void Symbols::exitScope()
{
  if (current->previous)
  {
    current = current->previous;
  }
  else
  {
    throw std::runtime_error("Hit the exitScope call at outermost scope");
  }
}

void Symbols::HashLookup(token &search_for)
{
  auto foundToken = current->symbol_table.find(search_for.tokenMark.stringValue);
  if (foundToken == current->symbol_table.end())
  {
    search_for.tokenHash = Hashgen++;
    current->symbol_table[search_for.tokenMark.stringValue] = search_for;
  }
  else
  {
    search_for.tokenHash = foundToken->second.tokenHash;
  }
}

bool Parser::scan_assume(token_type type, token *returned) // Complete
{
  if (cur_tk.type == type)
  {
    if (cur_tk.type == IDENTIFIER)
    {
      symbols->HashLookup(cur_tk);
      returned = &cur_tk;
      std::cout << "Hash in scan assume is " << returned->tokenHash << " for " << returned->tokenMark.stringValue << std::endl;
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

bool Parser::optional_scan_assume(token_type type, token *returned) // Complete
{
  if (cur_tk.type == type)
  {
    if (cur_tk.type == IDENTIFIER)
    {
      symbols->HashLookup(cur_tk);
      returned = &cur_tk;
      std::cout << "Hash in optional scan assume is " << returned->tokenHash << " for " << returned->tokenMark.stringValue << std::endl;
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

bool Parser::resync(token_type type, bool ahead = false) // Complete
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
  optional_scan_assume(GLOBAL_RW);
  if (optional_scan_assume(PROCEDURE_RW) && procedure_declaration())
    return true;
  tokenVariable *var = new tokenVariable();
  if (optional_scan_assume(VARIABLE_RW) && variable_declaration(var) && scan_assume((token_type)';'))
    return true;
  return false;
}

bool Parser::procedure_declaration() // Complete
{
  return (procedure_header() && procedure_body()) ? true : resync(PROCEDURE_RW, true);
}

bool Parser::procedure_header() // Complete
{
  symbols->enterScope();
  tokenProcedure *proc = new tokenProcedure();
  if (scan_assume(IDENTIFIER, proc) && scan_assume(TYPE_SEPERATOR) && type_mark(&(proc->retType)) && scan_assume((token_type)'(') && parameter_list(&(proc->argType)) && scan_assume((token_type)')'))
  {
    std::cout << proc->tokenMark.stringValue << " " << proc->retType.tokenMark.stringValue << " ";
    // for (auto i : proc->argType)
    // std::cout << i.dataType.tokenMark.stringValue << " ";
    std::cout << std::endl;
    return true;
  }
  else
    return resync((token_type)')');
}

bool Parser::parameter_list(std::vector<token *> *args) // Complete
{
  return optional_scan_assume(VARIABLE_RW) ? parameter(args) && (optional_scan_assume((token_type)',') ? parameter_list(args) : true) : true;
}

bool Parser::parameter(std::vector<token *> *args) // Complete
{
  tokenVariable *var;
  auto temp = variable_declaration(var);
  args->push_back(var);
  return temp;
}

bool Parser::procedure_body() // Complete
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

bool Parser::variable_declaration(tokenVariable *var) // Complete
{
  if (scan_assume(IDENTIFIER, var) && scan_assume(TYPE_SEPERATOR) && type_mark(var))
  {
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

/*bool Parser::type_mark(std::vector<tokenVariable> *returned) // Complete
{
  if (optional_scan_assume(INTEGER_RW, returned) || optional_scan_assume(FLOAT_RW, returned) || optional_scan_assume(STRING_RW, returned) || optional_scan_assume(BOOLEAN_RW, returned))
  {
    return true;
  }
  else
  {
    lexer_handle.reportError("Expected a type identifier");
    return false;
  }
}*/

bool Parser::type_mark(token *returned) // Complete
{
  if (optional_scan_assume(INTEGER_RW, returned) || optional_scan_assume(FLOAT_RW, returned) || optional_scan_assume(STRING_RW, returned) || optional_scan_assume(BOOLEAN_RW, returned))
  {
    return true;
  }
  else
  {
    lexer_handle.reportError("Expected a type identifier");
    return false;
  }
}

bool Parser::statement() // Complete
{
  token *temp;
  if (optional_scan_assume(IF_RW) && if_statement())
    return true;
  if (optional_scan_assume(FOR_RW) && loop_statement())
    return true;
  if (optional_scan_assume(RETURN_RW) && return_statement())
    return true;
  if (optional_scan_assume(IDENTIFIER, temp))
  {
    if (optional_scan_assume(EQUAL_ASSIGN) && assignment_statement())
      return true;
    if (optional_scan_assume((token_type)'[') && scan_assume(INTEGER_VAL) && scan_assume((token_type)']') && scan_assume(EQUAL_ASSIGN) && assignment_statement())
      return true;
    if (optional_scan_assume((token_type)'(') && procedure_call())
      return true;
  }
  return false;
}

bool Parser::if_statement() // Complete
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

bool Parser::factor() // Complete
{
  if (optional_scan_assume(TRUE_RW) || optional_scan_assume(FALSE_RW) || optional_scan_assume(STRING_VAL))
    return true;
  else
  {
    optional_scan_assume((token_type)'-');
    if (optional_scan_assume(INTEGER_VAL) || optional_scan_assume(FLOAT_VAL) || (optional_scan_assume(IDENTIFIER) && destination()))
      return true;
    if (optional_scan_assume((token_type)'(') && expression() && scan_assume((token_type)')'))
      return true;
    return false;
  }
}

bool Parser::argument_list() // Complete
{
  return expression() ? (optional_scan_assume((token_type)',') ? argument_list() : true) : true;
}

bool Parser::destination() // Complete
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

bool Parser::cond_expression() // Complete
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

bool Parser::return_statement() // Complete
{
  return expression();
}

bool Parser::procedure_call() // Complete
{
  return (optional_scan_assume((token_type)')') || (argument_list() && scan_assume((token_type)')')));
}