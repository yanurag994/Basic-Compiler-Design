#include "./Lexer.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <map>
#include <set>
#include <vector>
#include <sstream>

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

void Lexer::reportError(const std::string &error)
{
  errorStatus = true;
  std::cerr << "ERROR at line " << getLineCnt() << ": " << error << std::endl;
}

void Lexer::reportWarning(const std::string &warning) { std::cout << "WARNING at linel" << getLineCnt() << " " << warning; }

bool Lexer::getErrorStatus() { return errorStatus; }

token_type Lexer::tokenTypeLookup(const std::string &lexeme)
{
  auto symbol = symbol_table.find(lexeme);
  return (symbol != symbol_table.end()) ? static_cast<token_type>(symbol->second) : IDENTIFIER;
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
  case ']':
  case '+':
  case '-':                        // ... and other single char tokens
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
    tk.type = tokenTypeLookup(std::string(tk.tokenMark.stringValue));
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
    tk.type = tokenTypeLookup(std::string(tk.tokenMark.stringValue));
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