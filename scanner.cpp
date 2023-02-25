#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <map>
#include <set>
#include <vector>
#include <sstream>

using namespace std;
enum token_type
{
  IF_RW = 256,
  BEGIN_RW,
  THEN_RW,
  ELSE_RW,
  END_RW,
  RETURN_RW,
  FOR_RW,
  WHILE_RW,
  GLOBAL_RW,
  PROCEDURE_RW,
  LESS_THAN,
  GREATER_THAN,
  EQUAL_ASSIGN,
  GREATER_EQUAL,
  LESS_EQUAL,
  TYPE_SEPERATOR,
  EQUALITY,
  NUMBER,
  INTEGER_VAL,
  FLOAT_VAL,
  STRING_VAL,
  INTEGER_RW,
  FLOAT_RW,
  STRING_RW,
  BOOLEAN_RW,
  IDENTIFIER,
  UNKNOWN,
  PROGRAM_RW,
  IS_RW,
  VARIABLE_RW
};
int symbol_table_key_gen = 10000;

class symbols
{
  std::map<string, int> symbol_table;
  token_type hashLook(string);
  void enterScope();
  void exitScope();
};
std::map<string, int> symbol_table;

class token
{
public:
  int type;
  union
  {
    char stringValue[256] = {}; // holds lexeme value if string/identifier
    int intValue;               // holds lexeme value if integer
    double doubleValue;         // holds lexeme value if double
  } tokenMark;
  int get_token_type()
  {
    auto symbol = symbol_table.find(tokenMark.stringValue);
    if (symbol == symbol_table.end())
      return false;
    type = symbol->second;
    return true;
  }
  void set_token_type()
  {
    // symbol_table[tokenMark.stringValue] = symbol_table_key_gen++;
    // type = symbol_table[tokenMark.stringValue];
    type = IDENTIFIER;
    return;
  };
};

class inFile
{
private:
  std::ifstream filePtr; // the input file
  string fileName;
  int lineCntHist = 0;
  bool errorStatus = false;
  std::basic_istream<char>::pos_type seekpos;
  int lineCnt = 1; // the line count; initialized to zero
public:
  bool attachFile(string filename)
  {
    fileName = filename;
    filePtr.open(fileName);
    return true;
  }; // open the named file
  char getChar()
  {
    if (filePtr.is_open() && !filePtr.eof())
    {
      char c;
      filePtr.get(c);
      return c;
    }
    return EOF;
  }                                     // get the next character
  void ungetChar() { filePtr.unget(); } // push character back to the input file string
  int getLineCnt() { return lineCnt; }
  void incLineCnt() { lineCnt++; }
  void recordState()
  {
    lineCntHist = lineCnt;
    seekpos = filePtr.tellg();
  }
  void rollbackState()
  {
    lineCnt = lineCntHist;
    filePtr.seekg(seekpos);
  };
  void reportError(string error)
  {
    errorStatus = true;
    std::cout << "ERROR at line " << getLineCnt() << " " << error << std::endl;
  }
  void reportWarning(string warning)
  {
    std::cout << warning;
  }
  bool getErrorStatus() { return errorStatus; }
};

bool parser_statement(inFile &file_holder);
bool parser_declaration(inFile &file_holder);
bool parser_procedure_declaration(inFile &file_holder);
bool parser_procedure_body(inFile &file_holder);
bool parser_program_header(inFile &file_holder);

void unscan(inFile &file_holder)
{
  file_holder.rollbackState();
  return;
}

token scan(inFile &file_holder)
{
  file_holder.recordState();
  token tk;
  char nxtChar = file_holder.getChar();
  while(true){
    bool flag=false;
    while (nxtChar == '\n' || nxtChar == '\t' || nxtChar == '\r' || nxtChar == ' ')
    {
      flag=true;
      if (nxtChar == '\n')
        file_holder.incLineCnt();
      nxtChar = file_holder.getChar();
    }
  // build a loop here to process comments
  while (nxtChar == '/')
  {
    nxtChar = file_holder.getChar();
    if (nxtChar == '/')
    {
      flag=true;
      while (nxtChar != '\n')
      {
        nxtChar = file_holder.getChar();
      }
      file_holder.incLineCnt();
    }
    else if (nxtChar == '*'){
      flag=true;
      while (true)
      {
        while (nxtChar != '*')
          nxtChar = file_holder.getChar();
        nxtChar = file_holder.getChar();
        if (nxtChar == '/')
          nxtChar = file_holder.getChar();
        break;
      }}
    nxtChar = file_holder.getChar();
    if(flag==false) break;
  }
  }

  while (nxtChar == '\n' || nxtChar == '\t' || nxtChar == '\r' || nxtChar == ' ')
  {
    if (nxtChar == '\n')
      file_holder.incLineCnt();
    nxtChar = file_holder.getChar();
  }

  switch (nxtChar)
  {
  case '"':
    tk.type = STRING_VAL;
    nxtChar = file_holder.getChar();
    tk.tokenMark.stringValue[0] = nxtChar;
    unsigned i;
    nxtChar = file_holder.getChar();
    for (i = 1; nxtChar != '"'; i++)
    {
      tk.tokenMark.stringValue[i] = nxtChar;
      nxtChar = file_holder.getChar();
    }
    break;

  case ';':
  case ',':
  case '(':
  case ')':
  case '+':
  case '-':
  case '*':
  case '/':                        // ... and other single char tokens
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
  case '<':
  case '=':
  case '>':
  case ':':
  {
    std::set<char> allowed_chars = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '='};
    tk.tokenMark.stringValue[0] = tolower(nxtChar);
    unsigned i;
    nxtChar = file_holder.getChar();
    for (i = 1; true; i++)
    {
      if (allowed_chars.find(nxtChar) == allowed_chars.end())
        break;
      tk.tokenMark.stringValue[i] = tolower(nxtChar); // gather lowercase
      nxtChar = file_holder.getChar();
    }
    file_holder.ungetChar();
    tk.tokenMark.stringValue[i] = '\0';
    if (tk.get_token_type() == false)
      tk.set_token_type(); // get symbol for ident
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
    tk.type = NUMBER;
    double temp;
    temp = nxtChar - '0';
    while (isdigit(nxtChar = file_holder.getChar())) // convert digit char to number
      temp = temp * 10 + nxtChar - '0';
    if (nxtChar == '.')
    {
      double fractionalPart = 0.1;
      while (isdigit(nxtChar = file_holder.getChar()))
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
    file_holder.ungetChar();
    break;
  }

  case '.':
    tk.type = EOF;
    break;

  case EOF:
    // generate error and return
    tk.type = EOF;
    break;
  default: // anything else is not recognized
    std::stringstream ss;
    for (int i = 0; nxtChar != ' ' && nxtChar != '\t' && nxtChar != '\n' && nxtChar != '\r'; i++)
    {
      tk.tokenMark.stringValue[i] = nxtChar;
      ss << nxtChar;
      nxtChar = file_holder.getChar();
    }
    file_holder.reportError("Illegal character: " + ss.str() + ", found. Ignored");
    tk.type = UNKNOWN;
    file_holder.ungetChar();
    break;
  }
  std::cout << "Token type" << tk.type << " " << tk.tokenMark.stringValue << std::endl;
  return tk;
}

void initialize_token_table()
{
  symbol_table["program"] = PROGRAM_RW;
  symbol_table["is"] = IS_RW;
  symbol_table["if"] = IF_RW;
  symbol_table["then"] = THEN_RW;
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
  symbol_table["bool"] = BOOLEAN_RW;
  symbol_table["<"] = LESS_THAN;
  symbol_table[">"] = GREATER_THAN;
  symbol_table["=="] = EQUALITY;
  symbol_table["<="] = LESS_EQUAL;
  symbol_table[">="] = GREATER_EQUAL;
  symbol_table[":="] = EQUAL_ASSIGN;
  symbol_table[":"] = TYPE_SEPERATOR;
  return;
}

bool parser_type_mark(inFile &file_holder)
{
  int type = scan(file_holder).type;
  if (type == INTEGER_RW || type == FLOAT_RW || type == STRING_RW || type == BOOLEAN_RW)
    return true;
  return false;
}

bool parser_variable_declaration(inFile &file_holder)
{
  if (scan(file_holder).type == VARIABLE_RW)
  {
    if (scan(file_holder).type == IDENTIFIER)
      if (scan(file_holder).type == TYPE_SEPERATOR)
        if (parser_type_mark(file_holder))
          if (scan(file_holder).type == '[')
          {
            if (scan(file_holder).type == INTEGER_VAL)
              if (scan(file_holder).type == ']')
                return true;
            return false;
          }
          else
          {
            unscan(file_holder);
            return true;
          }
  }
  else
  {
    unscan(file_holder);
  }
  return false;
}

bool parser_parameter(inFile &file_holder)
{
  if (parser_variable_declaration(file_holder))
    return true;
  return false;
}

bool parser_parameter_list(inFile &file_holder)
{ // Need corrections
  if (parser_parameter(file_holder))
    return true;
  if (scan(file_holder).type == ',')
  {
    if (parser_parameter_list(file_holder))
      return true;
    return false;
  }
  else
  {
    unscan(file_holder);
    return false;
  }
}

bool parser_procedure_header(inFile &file_holder)
{
  if (scan(file_holder).type == PROCEDURE_RW)
  {
    if (scan(file_holder).type == IDENTIFIER)
      if (scan(file_holder).type == TYPE_SEPERATOR)
        if (parser_type_mark(file_holder))
          if (scan(file_holder).type == (int)'(')
            if (parser_parameter_list(file_holder))
              if (scan(file_holder).type == (int)')')
                return true;
  }
  else
  {
    unscan(file_holder);
  }
  return false;
}

bool parser_expression(inFile &file_holder)
{
  return false;
}

bool parser_argument_list(inFile &file_holder)
{
  if (parser_expression(file_holder))
    if (scan(file_holder).type != ',')
    {
      unscan(file_holder);
      return true;
    }
    else
    {
      if (parser_argument_list(file_holder))
        return true;
    }
  return false;
}

bool parser_destination(inFile &file_holder)
{
  if (scan(file_holder).type != IDENTIFIER)
  {
    unscan(file_holder);
    return false;
  }
  while (parser_expression(file_holder))
    ;
  return true;
}

bool parser_assignment_statement(inFile &file_holder)
{
  if (!parser_destination(file_holder))
    if (scan(file_holder).type == EQUAL_ASSIGN)
      if (parser_expression(file_holder))
        return true;
  return false;
}

bool parser_if_statement(inFile &file_holder)
{
  if (scan(file_holder).type != IF_RW)
  {
    unscan(file_holder);
    return false;
  }
  if (scan(file_holder).type == '(')
    if (parser_expression(file_holder))
      if (scan(file_holder).type == THEN_RW)
        if (scan(file_holder).type == ')')
        {
          while (parser_statement(file_holder) && scan(file_holder).type == ';')
            ;
          if (scan(file_holder).type != ELSE_RW)
          {
            unscan(file_holder);
            if (scan(file_holder).type == END_RW)
              if (scan(file_holder).type == IF_RW)
                return true;
            return false;
          }
          while (parser_statement(file_holder) && scan(file_holder).type == ';')
            ;
          if (scan(file_holder).type == END_RW)
            if (scan(file_holder).type == IF_RW)
              return true;
          return false;
        }
  return false;
}

bool parser_loop_statement(inFile &file_holder)
{
  if (scan(file_holder).type != FOR_RW)
  {
    unscan(file_holder);
    return false;
  }
  if (parser_assignment_statement(file_holder) && scan(file_holder).type == ';')
  {
    while (parser_statement(file_holder) && scan(file_holder).type == ';')
      ;
    if (scan(file_holder).type == END_RW && scan(file_holder).type == FOR_RW)
      return true;
  }
  return false;
}

bool parser_return_statement(inFile &file_holder)
{
  if (scan(file_holder).type != RETURN_RW)
  {
    unscan(file_holder);
    return false;
  }
  if (parser_expression(file_holder))
    return true;
  return false;
}

bool parser_procedure_call(inFile &file_holder)
{
  if (scan(file_holder).type != IDENTIFIER)
  {
    unscan(file_holder);
    return false;
  }
  if ((parser_argument_list(file_holder)))
    return true;
  return false;
}

bool parser_statement(inFile &file_holder)
{
  if (parser_assignment_statement(file_holder) || parser_if_statement(file_holder) || parser_loop_statement(file_holder) || parser_procedure_call(file_holder) || parser_return_statement(file_holder))
    return true;
  return false;
}

bool parser_procedure_body(inFile &file_holder)
{
  while (parser_declaration(file_holder))
    std::cout << "Cool";
  if (scan(file_holder).type == BEGIN_RW)
    while (parser_statement(file_holder) && scan(file_holder).type == ';')
      ;
  if (scan(file_holder).type == END_RW && scan(file_holder).type == PROCEDURE_RW && scan(file_holder).type == ';')
    return true;
  return false;
}

bool parser_procedure_declaration(inFile &file_holder)
{
  if (parser_procedure_header(file_holder))
  {
    if (parser_procedure_body(file_holder))
    {
      std::cout << "Passed procedure body";
      return true;
    }
  }
  return false;
}

bool parser_program_header(inFile &file_holder)
{
  if (scan(file_holder).type == PROGRAM_RW)
    if (scan(file_holder).type == IDENTIFIER)
      if (scan(file_holder).type == IS_RW)
        return true;
  return false;
}

bool parser_declaration(inFile &file_holder)
{
  if (scan(file_holder).type != GLOBAL_RW)
    unscan(file_holder);
  if (parser_procedure_declaration(file_holder))
    return true;
  if (parser_variable_declaration(file_holder) && scan(file_holder).type == ';')
    return true;
  return false;
}

bool parser_program_body(inFile &file_holder)
{
  while (parser_declaration(file_holder))
    ;
  if (scan(file_holder).type != BEGIN_RW)
    return false;
  while (parser_statement(file_holder) && scan(file_holder).type == ';')
    ;
  if (scan(file_holder).type == END_RW)
    if (scan(file_holder).type == PROGRAM_RW)
      return true;
  return false;
}

bool parser_program(inFile &file_holder)
{
  if (parser_program_header(file_holder))
    parser_program_body(file_holder);
  if (scan(file_holder).type == EOF)
    return true;
  return false;
}

int main()
{
  inFile scan_file;
  scan_file.attachFile("correct/math.src");
  initialize_token_table();
  std::cout << parser_program(scan_file);
  token tk;
  /*   do
    {
      tk = scan(scan_file);
      std::cout << tk.type << " ; " << tk.tokenMark.intValue << " " << tk.tokenMark.doubleValue << " " << tk.tokenMark.stringValue << std::endl;
    } while (tk.type != EOF);
    unscan(scan_file);
      tk = scan(scan_file);
      std::cout << tk.type << " ; " << tk.tokenMark.intValue << " " << tk.tokenMark.doubleValue << " " << tk.tokenMark.stringValue << std::endl;
     */
  return 0;
}