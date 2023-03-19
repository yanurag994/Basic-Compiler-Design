#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

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
    T_EOF
};

struct Scope
{
    std::map<std::string, int> symbol_table;
    Scope *next;
    Scope *previous;
    Scope(std::map<std::string, int> symbol_table, Scope *previous, Scope *next) : symbol_table(symbol_table), previous(previous), next(next){};
};

struct tokenMk
{
    int intValue;
    double doubleValue;
    char stringValue[1024];
};

struct token
{
    token_type type;
    token_type hash;
    tokenMk tokenMark;
};

struct tokenVariable : token
{
    token dataType;
};

struct tokenArray : tokenVariable
{
    int size;
    token baseType;
};

struct tokenProcedure : token
{
    std::vector<token> argType;
    token returnType;
};

class Symbols
{
private:
    Scope *current; // pointer to the first node in the list
    std::map<std::string, int> initialize_token_table();

public:
    std::map<std::string, int> &symbol_table;
    Symbols() : current(new Scope(initialize_token_table(), nullptr, nullptr)), symbol_table(current->symbol_table) {}
    void enterScope();
    void exitScope();
    void enterSoftScope();
    void exitSoftScope();
};

class Lexer
{
private:
    std::ifstream filePtr; // the input file
    std::string fileName;
    bool errorStatus = false;
    int lineCnt = 1; // the line count; initialized to zero
    Symbols *symbols;
    std::map<std::string, int> &symbol_table = symbols->symbol_table;

public:
    Lexer(std::string filename)
    {
        try
        {
            fileName = filename;
            filePtr.open(filename);
            if (filePtr.fail())
            {
                throw std::runtime_error("Failed to open file");
            }
            symbols = new Symbols();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
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
    void reportError(std::string);
    void reportWarning(std::string);
    bool getErrorStatus();
    token_type hashLook(std::string);
    bool isAlpha(char);
    bool isDigit(char);
    bool isAlnum(char);
    token scan();
};