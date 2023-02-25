#include "./Lexer.hpp"

int main(){
  Lexer handle("correct/math.src");
  token tk;
  do{
    tk=handle.scan();
    std::cout << tk.type<<" ; "<<tk.tokenMark.intValue<<" ; "<<tk.tokenMark.stringValue<<std::endl;
  }while(tk.type!=eof);
}