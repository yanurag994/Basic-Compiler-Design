#include "./Lexer.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <map>
#include <set>
#include <vector>
#include <sstream>

int symbol_table_key_gen = 10000;

std::map<std::string, int> Symbols::initialize_token_table()
{
  std::map<std::string, int> symbol_table;
  symbol_table["program"] = PROGRAM_RW;
  symbol_table["is"] = IS_RW;
  symbol_table["if"] = IF_RW;
  symbol_table["then"] = THEN_RW;
  symbol_table["else"] = ELSE_RW;
  symbol_table["for"] = FOR_RW;
  symbol_table["end"] = END_RW;
  symbol_table["begin"] = BEGIN_RW;
  symbol_table["return"] = RETURN_RW;
  symbol_table["procedure"] = PROCEDURE_RW;
  symbol_table["while"] = WHILE_RW;
  symbol_table["global"] = GLOBAL_RW;
  symbol_table["variable"] = VARIABLE_RW;
  symbol_table["float"] = FLOAT_RW;
  symbol_table["integer"] = INTEGER_RW;
  symbol_table["string"] = STRING_RW;
  symbol_table["bool"] = BOOLEAN_RW;
  symbol_table["true"] = TRUE_RW;
  symbol_table["false"] = FALSE_RW;
  symbol_table["<"] = LESS_THAN;
  symbol_table[">"] = GREATER_THAN;
  symbol_table["=="] = EQUALITY;
  symbol_table["<="] = LESS_EQUAL;
  symbol_table[">="] = GREATER_EQUAL;
  symbol_table[":="] = EQUAL_ASSIGN;
  symbol_table[":"] = TYPE_SEPERATOR;
  symbol_table["!="] = NOT_EQUAL;
  return symbol_table;
}

void Symbols::enterScope()
{
  current->next = (!current->next) ? new Scope(initialize_token_table(), nullptr, current) : current->next;
  current = current->next;
  symbol_table = current->symbol_table;  
  return;
}
void Symbols::exitScope()
{
  if (current->previous)
  {
    current = current->previous;
    symbol_table = current->symbol_table;
  }
  else
    std::runtime_error("Hit the exitScope call at outermost scope");
}

void Symbols::enterSoftScope()
{
  current = new Scope(current->symbol_table, nullptr, current);
  symbol_table = current->symbol_table;
}

void Symbols::exitSoftScope()
{
  if (current->previous)
  {
    current = current->previous;
    symbol_table = current->symbol_table;
  }
  else
    std::runtime_error("Hit the exitSoftScope call at outermost scope");
}

char Lexer::getChar()
{
  if (filePtr.is_open() && !filePtr.eof())
  {
    char c;
    filePtr.get(c);
    return c;
  }
  return EOF;
}

void Lexer::ungetChar() { filePtr.unget(); }

int Lexer::getLineCnt() { return lineCnt; }

void Lexer::incLineCnt() { lineCnt++; }

void Lexer::reportError(std::string error)
{
  errorStatus = true;
  std::cout << "ERROR at line " << getLineCnt() << " " << error << std::endl;
}

void Lexer::reportWarning(std::string warning) { std::cout << "WARNING at linel" << getLineCnt() << " " << warning; }

bool Lexer::getErrorStatus() { return errorStatus; }

token_type Lexer::hashLook(std::string lexeme)
{
  auto symbol = symbols->symbol_table.find(lexeme);
  if (symbol == symbols->symbol_table.end())
  {
    symbols->symbol_table[lexeme] = symbol_table_key_gen++;
    return static_cast<token_type>(symbols->symbol_table.find(lexeme)->second);
  }
  return static_cast<token_type>(symbol->second);
}

bool Lexer::isAlpha(char c) { return (c == '_') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

bool Lexer::isDigit(char c) { return c >= '0' && c <= '9'; }

bool Lexer::isAlnum(char c) { return isAlpha(c) || isDigit(c); }

token Lexer::scan()
{
  token tk;
  char nxtChar = getChar();
  while (true)
  {
    bool flag = false;
    while (nxtChar == '\n' || nxtChar == '\t' || nxtChar == '\r' || nxtChar == ' ')
    {
      flag = true;
      if (nxtChar == '\n')
        incLineCnt();
      nxtChar = getChar();
    }
    // build a loop here to process comments
    if (nxtChar == '/')
    {
      nxtChar = getChar();
      if (nxtChar == '/')
      {
        flag = true;
        while (nxtChar != '\n' && nxtChar != EOF) // Add check for EOF to avoid infinite loop)
        {
          if (nxtChar == '\n')
            incLineCnt();
          nxtChar = getChar();
        }
      }
      else if (nxtChar == '*')
      {
        flag = true;
        int level = 1; // Initialize comment nesting level to 1
        while (level > 0)
        {
          while (nxtChar != '*' && nxtChar != '/' && nxtChar != EOF)
          {
            nxtChar = getChar();
          }
          if (nxtChar == EOF) // Add check for EOF to avoid infinite loop
          {
            throw std::runtime_error("Error: Unexpected end of file in comment");
          }
          else if (nxtChar == '/')
          {
            nxtChar = getChar();
            if (nxtChar == '*')
            {
              level++;
              nxtChar = getChar();
              continue;
            }
          }
          else if (nxtChar == '*')
          {
            nxtChar = getChar();
            if (nxtChar == '/')
            {
              level--;
              nxtChar = getChar();
              continue;
            }
          }
          else
          {
            nxtChar = getChar();
          }
        }
        nxtChar = getChar();
      }
      else
      {
        ungetChar(); // Add case for single slash which is not a comment
        break;
      }
    }
    if (flag == false) // Break the loop if no comment or whitespace is encountered
      break;
  }

  switch (nxtChar)
  {
  case '"':
    tk.type = STRING_VAL;
    nxtChar = getChar();
    tk.tokenMark.stringValue[0] = nxtChar;
    unsigned i;
    nxtChar = getChar();
    for (i = 1; nxtChar != '"'; i++)
    {
      tk.tokenMark.stringValue[i] = nxtChar;
      nxtChar = getChar();
    }
    tk.tokenMark.stringValue[i] = '\0';
    break;

  case '.':
  case ';':
  case ',':
  case '(':
  case ')':
  case '*':
  case '/':
  case '[':
  case ']':                        // ... and other single char tokens
    tk.type = (token_type)nxtChar; // tk.type = nxtChar; // ASCII value is used as token type
    break;                         // ASCII value used as token type

  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'g':
  case 'h':
  case 'i':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'o':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't':
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':
  {
    tk.tokenMark.stringValue[0] = tolower(nxtChar);
    unsigned i;
    nxtChar = getChar();
    for (i = 1; isAlnum(nxtChar); i++)
    {
      tk.tokenMark.stringValue[i] = tolower(nxtChar); // gather lowercase
      nxtChar = getChar();
    }
    ungetChar();
    tk.tokenMark.stringValue[i] = '\0';
    tk.type = hashLook(std::string(tk.tokenMark.stringValue));
    if (tk.type > 9999)
    {
      tk.hash = tk.type;
      tk.type = IDENTIFIER;
    }
    break;
  }

  case '<':
  case '=':
  case '>':
  case ':':
  {
    tk.tokenMark.stringValue[0] = nxtChar;
    unsigned i;
    nxtChar = getChar();
    if (nxtChar == '=')
    {
      tk.tokenMark.stringValue[1] = nxtChar;
      tk.tokenMark.stringValue[2] = '\0';
    }
    else
    {
      ungetChar();
      tk.tokenMark.stringValue[1] = '\0';
    }
    tk.type = hashLook(std::string(tk.tokenMark.stringValue));
    break;
  }

  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9': //.... and other digits
  case '+':
  case '-':
  {
    if (nxtChar == '+' || nxtChar == '-')
    {
      tk.type = (token_type)nxtChar;
      nxtChar = getChar();
      if (!isDigit(nxtChar))
      {
        ungetChar();
        break;
      }
    }
    tk.type = NUMBER;
    double temp;
    temp = nxtChar - '0';
    while (isdigit(nxtChar = getChar())) // convert digit char to number
      temp = temp * 10 + nxtChar - '0';
    if (nxtChar == '.')
    {
      double fractionalPart = 0.1;
      while (isdigit(nxtChar = getChar()))
      {
        temp += fractionalPart * (nxtChar - '0');
        fractionalPart /= 10;
      }
      tk.tokenMark.doubleValue = temp;
      tk.type = FLOAT_VAL;
    }
    else
    {
      tk.tokenMark.intValue = (int)temp;
      tk.type = INTEGER_VAL;
    }
    ungetChar();
    break;
  }

  case EOF:
    reportError("Unexpected End Of File reached. Expected program to end with \'.\'");
    tk.type = T_EOF;
    break;

  default: // anything else is not recognized
    std::stringstream ss;
    for (int i = 0; nxtChar != ' ' && nxtChar != '\t' && nxtChar != '\n' && nxtChar != '\r'; i++)
    {
      tk.tokenMark.stringValue[i] = nxtChar;
      ss << nxtChar;
      nxtChar = getChar();
    }
    reportError("Illegal character: " + ss.str() + ", found. Ignored");
    tk.type = UNKNOWN;
    ungetChar();
    break;
  }
  // std::cout << "Token type" << tk.type << " " << tk.tokenMark.stringValue << std::endl;
  return tk;
}