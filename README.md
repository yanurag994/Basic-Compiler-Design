# Compiler Features and Instructions

This document outlines the features, requirements, build instructions, and run instructions for a compiler that generates LLVM IR. The compiler processes input files and produces LLVM IR code as output.

## Features

1. **Array Bounds Check Issue:** The compiler has a known limitation where array bounds checking might not work as expected when the index is an expression.

## Requirements

- C++14
- LLVM

## Build Instructions

1. Navigate to the root directory of the project.
2. Open a terminal window.
3. Run the following commands:

       cmake . -DCMKAE_BUILD_TYPE=Release
       make

## Run Instructions

       ./Compiler <input_file_path>

  Above command will output the LLVM IR to the console and attempt to execute it. If you want to write the LLVM IR to an output file and attempt execution, use the following command:
  
       ./Compiler <input_file_path> <output_file_path>

  Replace <input_file_path> with the path to your input file and <output_file_path> with the desired output file path.

## Note on Segmentation Fault

Please be aware that the execution engine might terminate with a segmentation fault due to the use of the malloc instruction without proper memory freeing.
