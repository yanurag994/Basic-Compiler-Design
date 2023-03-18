cmake_minimum_required(VERSION 3.11)

project(MyProject)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-g")

# Add the library target
add_library(Lexer STATIC Lexer.cpp Lexer.hpp)
add_library(Parser STATIC Parser.cpp Parser.hpp)

# Add the executable target and link it with the library
add_executable(Compiler main.cpp)
target_link_libraries(Compiler PRIVATE Lexer Parser)