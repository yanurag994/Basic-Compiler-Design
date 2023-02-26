#include "./Lexer.hpp"
#include "./Parser.hpp"

int main(){
  Lexer handle("correct/math.src");
  std::cout<<"Testing Lexer"<<std::endl;
  token tk;
  do{
    tk=handle.scan();
    std::cout << tk.type<<" ; "<<tk.tokenMark.intValue<<" ; "<<tk.tokenMark.stringValue<<std::endl;
  }while(tk.type!=T_EOF);
}