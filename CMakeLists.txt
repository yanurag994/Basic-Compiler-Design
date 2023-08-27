cmake_minimum_required(VERSION 3.11)

project(MyProject)

# Set C++ standard to 20
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS)

# Add the library target
add_library(Lexer STATIC Lexer.cpp Lexer.hpp)
add_library(Parser STATIC Parser_support.cpp Parser.cpp Parser.hpp)

# Find LLVM and include its directories
find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Add the executable target and link it with the library
add_executable(Compiler main.cpp)
target_link_libraries(Compiler PRIVATE Lexer Parser LLVM)