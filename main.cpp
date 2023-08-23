#include "./Lexer.hpp"
#include "./Parser.hpp"
#include <iostream>
#include <fstream> // Include the header for file operations

int main(int argc, char* argv[]) {
    if (argc < 3) { // Modified to require two arguments
        std::cerr << "Please provide both input and output file paths as arguments.\n";
        return 1;
    }

    Parser handle(argv[1]);

    if (handle.program()) {
        std::cout << "Success" << std::endl;

        // Create an output file stream
        std::ofstream outputFile(argv[2]);

        // Write the output to the file
        outputFile << handle.global_decl.str();
        outputFile << handle.output.str();

        // Close the output file stream
        outputFile.close();
    } else {
        std::cout << "Failed" << std::endl;
    }

    return 0;
}