#include <iostream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <map>

using namespace std;

std::map<std::string, int> tokenType = {
    {"PLUS", 0},
    {"MINUS", 1},
    {"IF_RW", 2},
    {"LOOP_RW", 3},
    {"END_RW", 4},
    {"L_PAREN", 5},
    {"R_PAREN", 6},
    {"L_BRACKET", 7},
    {"R_BRACKET", 8},
    {"NUMBER", 9},
    {"FLOAT", 10},
    {"BOOL", 11},
    {"UNKNOWN", 12},
    {"IDENTIFIER", 13},
    {"T_IDENTIFIER", 14}
};

class token
{
  public:
  int type;
  union {
  char stringValue[256]; // holds lexeme value if string/identifier
  int intValue; // holds lexeme value if integer
  double doubleValue; // holds lexeme value if double
  } val;
  int get_token_type(){type=tokenType[val.stringValue];}
  void set_token_type(int type)
  {
    type= type;
    return;
  }
};

class inFile{
  private:
  std::ifstream filePtr; // the input file
  string fileName;
  int lineCnt = 0; // the line count; initialized to zero
  public:
  bool atypeachFile(string filename) {
    fileName=filename;
    filePtr.open(fileName);
    return true;};// open the named file
  char getChar() {
    if(filePtr.is_open() && !filePtr.eof()){
        char c;
        filePtr.get(c);
        if(c=='\n')
          lineCnt++;
        return c;
    }
    return EOF;
  }// get the next character
  void ungetChar(){filePtr.unget();} // push character back to the input file string
  int getLineCnt(){return lineCnt;}
  void incLineCnt(){lineCnt++;}
};

int scan(inFile file_holder){
  char nxtChar = file_holder.getChar();
  while (nxtChar=='\n' ||nxtChar=='\t'|| nxtChar=='\r'){
  if (nxtChar == '\n')
   file_holder.incLineCnt();
  }
  nxtChar = file_holder.getChar();
  // build a loop here to process comments
  switch(nxtChar) {
    token tk;
    case '/': // could either begin comment or T_DIVIDE op
    nxtChar = file_holder.getChar();
    if (nxtChar == '/' || nxtChar == '*')
    ; // here you would skip over the comment
    else
    file_holder.ungetChar(); // fall-through to single-char token case

    case ';': case ',': case '=': // ... and other single char tokens
      //tk.type = nxtChar; // ASCII value is used as token type
      return tk.type; // ASCII value used as token type

    case 'A': case 'B': case 'C': // ... and other upper letypeers
      tk.val.stringValue[0] = nxtChar;
      unsigned i;
      for (i = 1; isupper(nxtChar = file_holder.getChar()); i++) // gather uppercase
      tk.val.stringValue[i] = nxtChar;
      file_holder.ungetChar();
      tk.val.stringValue[i] = '\0'; // lookup reserved word
      tk.get_token_type();
    return tk.type;

    case 'a': case 'b': case 'c': // ... and other lower letypeers
      tk.type = tokenType["NUMBER"];
      tk.val.stringValue[0] = nxtChar;
      for (i = 1; islower(nxtChar = file_holder.getChar()); i++)
      tk.val.stringValue[i] = nxtChar; // gather lowercase
      file_holder.ungetChar();
      tk.val.stringValue[i] = '\0';
      if (lookup_symtab(tk.val.stringValue) == NULL)
      add_symtab(tk.val.stringValue); // get symbol for ident
    return tk.type;

    case '0': case '1': case '2': case '3': //.... and other digits
      tk.type = tokenType["NUMBER"];
      tk.val.intValue = nxtChar - '0';
      while (isdigit(nxtChar = file_holder.getChar())) // convert digit char to number
      tk.val.intValue = tk.val.intValue * 10 + nxtChar - '0';
      file_holder.ungetChar();
    return tk.type;

    case EOF:
      return T_END;
 
    default: // anything else is not recognized
      tk.val.intValue = nxtChar;
      tk.type = tokenType["UNKNOWN"];
    return tk.type;
 } 



}

int main() {
return 0;
}