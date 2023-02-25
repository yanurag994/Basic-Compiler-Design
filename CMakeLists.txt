cmake_minimum_required(VERSION 3.11)

project(MyProject)

# Add the library target
add_library(Lexer STATIC Lexer.cpp Lexer.hpp)

# Add the executable target and link it with the library
add_executable(Compiler main.cpp)
target_link_libraries(Compiler PRIVATE Lexer)