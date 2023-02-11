#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <map>

using namespace std;
enum token_type
{
  IF_RW=256,
  BEGIN_RW,
  THEN_RW,
  ELSE_RW,
  END_RW,
  RETURN_RW,
  FOR_RW,
  WHILE_RW,
  GLOBAL_RW,
  PLUS,
  MINUS,
  MULTIPLY,
  DIVIDE,
  NUMBER,
  INTEGER,
  FLOAT,
  BOOLEAN,
  IDENTIFIER,
  UNKNOWN
};

std::map<string, int> symbol_table;

class token
{
public:
  int type;
  union
  {
    char stringValue[256]; // holds lexeme value if string/identifier
    int intValue;          // holds lexeme value if integer
    double doubleValue;    // holds lexeme value if double
  } tokenMark;
  int get_token_type()
  {
      auto symbol = symbol_table.find(tokenMark.stringValue);
      if(symbol==symbol_table.end())
        return false;
      type = symbol->second;
    return true;
  }
  void set_token_type()
  {
    symbol_table[tokenMark.stringValue]=10000;
    type=symbol_table[tokenMark.stringValue];
    return;
  };
};

class inFile
{
private:
  std::ifstream filePtr; // the input file
  string fileName;
  int lineCnt = 0; // the line count; initialized to zero
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
};

int scan(inFile &file_holder)
{
  char nxtChar = file_holder.getChar();
  while (nxtChar == '\n' || nxtChar == '\t' || nxtChar == '\r')
  {
    if (nxtChar == '\n')
      file_holder.incLineCnt();
  }
  nxtChar = file_holder.getChar();
  // build a loop here to process comments
  token tk;
  switch (nxtChar)
  {
  case '/':
  {
    nxtChar = file_holder.getChar();
    if (nxtChar == '/')
      while (nxtChar != '\n')
        nxtChar = file_holder.getChar();
    else if (nxtChar == '*')
      while (true)
      {
        while (nxtChar != '*')
          nxtChar = file_holder.getChar();
        if (nxtChar == '/')
        {
          nxtChar = file_holder.getChar();
          break;
        }
      }
    else
      file_holder.ungetChar();
  }

  case ';':  case ',':  case '=':                        // ... and other single char tokens
    tk.type = (token_type)nxtChar; // tk.type = nxtChar; // ASCII value is used as token type
    return tk.type;                // ASCII value used as token type

  case 'a':  case 'b':  case 'c':  case 'd':  case 'e':  case 'f':  case 'g':  case 'h':  case 'i':  case 'j':  case 'k':  case 'l':  case 'm':  case 'n':  case 'o':  case 'p':  case 'q':  case 'r':  case 's':  case 't':  case 'u':  case 'v':  case 'w':  case 'x':  case 'y':  case 'z':
    {
    tk.tokenMark.stringValue[0] = nxtChar;
    unsigned i;
    for (i = 1; islower(nxtChar = file_holder.getChar()); i++)
      tk.tokenMark.stringValue[i] = nxtChar; // gather lowercase
    file_holder.ungetChar();
    tk.tokenMark.stringValue[i] = '\0';
    if (tk.get_token_type() == false)
      tk.set_token_type(); // get symbol for ident
    return tk.type;
    }

  case '0':  case '1':  case '2':  case '3':  case '4':  case '5':  case '6':  case '7':  case '8':  case '9': //.... and other digits
    {
    tk.type = NUMBER;
    tk.tokenMark.intValue = nxtChar - '0';
    while (isdigit(nxtChar = file_holder.getChar())) // convert digit char to number
      tk.tokenMark.intValue = tk.tokenMark.intValue * 10 + nxtChar - '0';
    file_holder.ungetChar();
    return tk.type;
    }

  default: // anything else is not recognized
    tk.tokenMark.intValue = nxtChar;
    tk.type = UNKNOWN;
    return tk.type;
  }
}

void initialize_token_table()
{
  symbol_table["if"] = IF_RW;
  symbol_table["for"] = FOR_RW;
  symbol_table["while"] = WHILE_RW;
  symbol_table["global"] = GLOBAL_RW;
  symbol_table["variable"] = IDENTIFIER;
  symbol_table["float"]=FLOAT;
  symbol_table["integer"]=INTEGER;
  symbol_table["bool"]=BOOLEAN;
  return;
}

int main()
{
  inFile scan_file;
  scan_file.attachFile("correct/math.src");
  initialize_token_table();
  std::cout<<scan(scan_file);
  return 0;
}