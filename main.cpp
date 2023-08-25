#include "./Lexer.hpp"
#include "./Parser.hpp"
#include <iostream>
#include <fstream> // Include the header for file operations


int main(int argc, char *argv[])
{
    if (argc < 3)
    { // Modified to require two arguments
        std::cerr << "Please provide both input and output file paths as arguments.\n";
        return 1;
    }
    //Parser handle(argv[1]);
    Parser handle(argv[1], argv[2]);
    handle.initialize();
    handle.program();
    handle.execute();
    return 0;
};