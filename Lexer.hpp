#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <sstream>
#include <llvm/IR/Value.h>

enum token_type
{
    IF_RW = 256,
    BEGIN_RW,
    THEN_RW,
    ELSE_RW,
    END_RW,
    RETURN_RW,
    FOR_RW,
    WHILE_RW,
    GLOBAL_RW,
    PROCEDURE_RW,
    LESS_THAN,
    GREATER_THAN,
    EQUAL_ASSIGN,
    GREATER_EQUAL,
    LESS_EQUAL,
    TYPE_SEPERATOR,
    EQUALITY,
    NOT_EQUAL,
    NUMBER,
    INTEGER_VAL,
    FLOAT_VAL,
    STRING_VAL,
    INTEGER_RW,
    FLOAT_RW,
    STRING_RW,
    BOOLEAN_RW,
    IDENTIFIER,
    UNKNOWN,
    PROGRAM_RW,
    IS_RW,
    VARIABLE_RW,
    TRUE_RW,
    FALSE_RW,
    T_EOF,
};

struct tokenMk
{
    int intValue;
    double doubleValue;
    char stringValue[1024];
    tokenMk() : intValue(), doubleValue(), stringValue() {}
    tokenMk(const char *stringValue) : intValue(), doubleValue()
    {
        std::strncpy(this->stringValue, stringValue, sizeof(this->stringValue) - 1);
    }
    tokenMk(int intValue) : intValue(intValue) {}
    tokenMk(double doubleValue) : doubleValue(doubleValue) {}
};

struct basetoken
{
    bool global_var = false;
    token_type type;
    tokenMk tokenMark;
    std::string tokenHash;
    token_type dataType; // Holds datatype for variable and return type for procedure
    llvm::Value *llvm_value = nullptr;
    int size = -1;
    basetoken() : type(UNKNOWN), tokenHash() {}
    basetoken(token_type type, const tokenMk &tokenMark, std::string tokenHash, token_type dataType, int size = -1)
        : type(type), tokenMark(tokenMark), tokenHash(tokenHash), dataType(dataType), size(size) {}
};

struct token : basetoken
{
    std::vector<token> argType; // Populate only if a 
    std::vector<token> internal_args; // Populate only if a procedure
    token() : argType() {}
    token(token_type type, const tokenMk &tokenMark, std::string tokenHash, token_type dataType, int size = -1, const std::vector<token> &argType = {})
        : basetoken(type, tokenMark, tokenHash, dataType, size), argType(argType) {}
};

class Lexer
{
private:
    std::ifstream filePtr; // the input file
    std::string fileName;
    int lineCnt = 1; // the line count; initialized to zero
    std::map<std::string, token_type> symbol_table;

public:
    bool errorStatus = false;
    Lexer(std::string filename)
    {
        try
        {
            fileName = filename;
            filePtr.open(filename);
            if (filePtr.fail())
            {
                filePtr.close(); // Close the file
                throw std::runtime_error("Failed to open file");
            }
            symbol_table["program"] = PROGRAM_RW;
            symbol_table["is"] = IS_RW;
            symbol_table["if"] = IF_RW;
            symbol_table["then"] = THEN_RW;
            symbol_table["else"] = ELSE_RW;
            symbol_table["for"] = FOR_RW;
            symbol_table["end"] = END_RW;
            symbol_table["begin"] = BEGIN_RW;
            symbol_table["return"] = RETURN_RW;
            symbol_table["procedure"] = PROCEDURE_RW;
            symbol_table["while"] = WHILE_RW;
            symbol_table["global"] = GLOBAL_RW;
            symbol_table["variable"] = VARIABLE_RW;
            symbol_table["float"] = FLOAT_RW;
            symbol_table["integer"] = INTEGER_RW;
            symbol_table["string"] = STRING_RW;
            symbol_table["bool"] = BOOLEAN_RW;
            symbol_table["true"] = TRUE_RW;
            symbol_table["false"] = FALSE_RW;
            symbol_table["<"] = LESS_THAN;
            symbol_table[">"] = GREATER_THAN;
            symbol_table["=="] = EQUALITY;
            symbol_table["<="] = LESS_EQUAL;
            symbol_table[">="] = GREATER_EQUAL;
            symbol_table[":="] = EQUAL_ASSIGN;
            symbol_table[":"] = TYPE_SEPERATOR;
            symbol_table["!="] = NOT_EQUAL;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }
    }
    ~Lexer()
    {
        filePtr.close();
    }
    char getChar();
    void ungetChar();
    int getLineCnt();
    void incLineCnt();
    void reportError(const std::string &);
    void reportWarning(const std::string &);
    bool getErrorStatus();
    token_type tokenTypeLookup(const std::string &);
    bool isAlpha(char);
    bool isDigit(char);
    bool isAlnum(char);
    token scan();
};