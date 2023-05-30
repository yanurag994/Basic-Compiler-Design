#include "./Lexer.hpp"
#include "./Parser.hpp"
#include <iostream>

int main(int argc, char* argv[]){
  if (argc < 2){
    std::cerr << "Please provide a file path as an argument.\n";
    return 1;
  }

  Parser handle(argv[1]);
  if(handle.program())
    {
    std::cout<<"Success"<<std::endl;
    std::cout<<handle.output.str();
    }
  else
    std::cout<<"Failed"<<std::endl;
}