#include "./Lexer.hpp"
#include <vector>
#include <sstream>
#include <cstring>
#include <iostream>
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/raw_ostream.h>

class TokenNotFoundError : public std::runtime_error
{
public:
    TokenNotFoundError(const std::string &token)
        : std::runtime_error("Token not found: " + token) {}
};

class TokenRedefinitionError : public std::runtime_error
{
public:
    TokenRedefinitionError(const std::string &token)
        : std::runtime_error("Token already defined: " + token) {}
};

struct Scope
{
    std::map<std::string, token> symbol_table;
    Scope *previous;
    Scope(std::map<std::string, token> symbol_table, Scope *previous)
        : symbol_table(symbol_table), previous(previous) {}
};

class Symbols // Implements stack of Scopes
{
private:
    int Hashgen = 1100;
    Scope *current; // pointer to the first node in the list
public:
    Scope *global;
    Symbols() : current(new Scope(std::map<std::string, token>(), nullptr))
    {
        global = current;
        token newTokenGetBool(IDENTIFIER, tokenMk("getbool"), "1000", BOOLEAN_RW);
        global->symbol_table["getbool"] = newTokenGetBool;

        token newTokenGetInteger(IDENTIFIER, tokenMk("getinteger"), "1001", INTEGER_RW);
        global->symbol_table["getinteger"] = newTokenGetInteger;

        token newTokenGetFloat(IDENTIFIER, tokenMk("getfloat"), "1002", FLOAT_RW);
        global->symbol_table["getfloat"] = newTokenGetFloat;

        token newTokenGetString(IDENTIFIER, tokenMk("getstring"), "1003", STRING_RW);
        global->symbol_table["getstring"] = newTokenGetString;

        token newTokenPutBool(IDENTIFIER, tokenMk("putbool"), "1004", BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_bool"), "1050", BOOLEAN_RW)});
        global->symbol_table["putbool"] = newTokenPutBool;

        token newTokenPutInteger(IDENTIFIER, tokenMk("putinteger"), "1005", BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_int"), "1051", INTEGER_RW)});
        global->symbol_table["putinteger"] = newTokenPutInteger;

        token newTokenPutFloat(IDENTIFIER, tokenMk("putfloat"), "1006", BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_float"), "1052", FLOAT_RW)});
        global->symbol_table["putfloat"] = newTokenPutFloat;

        token newTokenPutString(IDENTIFIER, tokenMk("putstring"), "1007", BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_double"), "1053", STRING_RW)});
        global->symbol_table["putstring"] = newTokenPutString;

        token newTokenSqrt(IDENTIFIER, tokenMk("sqrt"), "1008", FLOAT_RW, -1, {token(IDENTIFIER, tokenMk("internal_sqrt"), "1054", INTEGER_RW)});
        global->symbol_table["sqrt"] = newTokenSqrt;
    }
    void enterScope() { current = new Scope(std::map<std::string, token>(), current); };
    void enterSoftScope() { current = new Scope(current->symbol_table, current); };
    std::string genTempvarHash()
    {
        return "%t_" + std::to_string(Hashgen++);
    }
    void exitScope()
    {
        if (current->previous)
            current = current->previous;
        else
            throw std::runtime_error("Hit the exitScope call at outermost scope");
    };
    token HashLookup(token &search_for, bool &global_def, bool definintion = false)
    {
        auto foundToken = current->symbol_table.find(search_for.tokenMark.stringValue);
        auto glbfoundToken = global->symbol_table.find(search_for.tokenMark.stringValue);
        if (definintion)
        {
            // Define the new token
            if ((global_def || current == global) && glbfoundToken == global->symbol_table.end())
            {
                global_def = false;
                search_for.global_var = true;
                search_for.tokenHash = "%g_" + std::to_string(Hashgen++);
                global->symbol_table[search_for.tokenMark.stringValue] = search_for;
                return global->symbol_table.find(search_for.tokenMark.stringValue)->second;
            }
            else if ((!global_def && current != global) && foundToken == current->symbol_table.end())
            {
                search_for.tokenHash = "%l_" + std::to_string(Hashgen++);
                current->symbol_table[search_for.tokenMark.stringValue] = search_for;
                return current->symbol_table.find(search_for.tokenMark.stringValue)->second;
            }
            else
            {
                throw TokenRedefinitionError(search_for.tokenMark.stringValue);
            }
        }
        else
        {
            if (foundToken != current->symbol_table.end())
                return foundToken->second;
            else if (glbfoundToken != global->symbol_table.end())
                return glbfoundToken->second;
            else
                throw TokenNotFoundError(search_for.tokenMark.stringValue);
        }
    };
    void Completetoken(token &search_for)
    {
        auto glbfoundToken = global->symbol_table.find(search_for.tokenMark.stringValue);
        auto foundToken = current->symbol_table.find(search_for.tokenMark.stringValue);
        if (glbfoundToken != global->symbol_table.end())
            glbfoundToken->second = search_for;
        if (foundToken != current->symbol_table.end())
            foundToken->second = search_for;
        if (glbfoundToken == global->symbol_table.end() && foundToken == current->symbol_table.end())
            current->symbol_table[search_for.tokenMark.stringValue] = search_for;
        return;
    };
    void CompleteDeclPrevtoken(token &search_for)
    {
        auto glbfoundToken = global->symbol_table.find(search_for.tokenMark.stringValue);
        if (glbfoundToken != global->symbol_table.end())
        {
            glbfoundToken->second = search_for;
            return;
        }
        auto foundToken = current->previous->symbol_table.find(search_for.tokenMark.stringValue);
        foundToken->second = search_for;
        return;
    };
};

class Parser
{
private:
    llvm::LLVMContext context;
    llvm::Module module;
    llvm::IRBuilder<> builder;
    llvm::Type *getLLVMType(token_type type);
    Lexer lexer_handle;
    token cur_tk;
    token prev_tk;
    bool global_flag = false;
    bool returned_flag_for_if = false;
    bool scan_assume(token_type);
    bool optional_scan_assume(token_type);
    bool scan_assume(token_type, token &, bool);
    bool optional_scan_assume(token_type, token &, bool);
    bool resync(token_type, bool);
    bool typeCheck(token &, token &, token &, token_type);
    bool program_header();
    bool program_body();
    bool declaration();
    bool procedure_declaration(token &);
    bool procedure_header(token &);
    bool parameter_list(std::vector<token> &);
    bool parameter(token &);
    bool procedure_body(token &);
    bool variable_declaration(token &);
    bool type_mark(token_type &);
    bool statement();
    bool procedure_call(token &, llvm::Value *&);
    bool assignment_statement(token &);
    bool destination(token &);
    bool if_statement();
    bool loop_statement();
    bool return_statement();
    bool expression(token &, llvm::Value *&);
    bool cond_expression(llvm::Value *&);
    bool arithOp(token &, llvm::Value *&);
    bool relation(token &, llvm::Value *&);
    bool factor(token &, llvm::Value *&);
    bool argument_list(llvm::Function *calleeFunc, std::vector<llvm::Value *> &args);
    void printf();
    void putinteger();
    void putfloat();
    void putstring();
    void putbool();
    void scanf();
    llvm::Value *getinteger();
    llvm::Value *getfloat();
    llvm::Value *getstring();
    llvm::Value *getbool();
    llvm::Value *sqrt();
    
public:
    std::stringstream global_decl;
    bool program();
    Symbols *symbols;
    Parser(const std::string &inputFileName) : lexer_handle(inputFileName), builder(context), module(inputFileName, context)
    {
        symbols = new Symbols();
        cur_tk = lexer_handle.scan();
    };
    void initialize()
    {
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        printf();
        scanf();
        putinteger();
        putfloat();
        putstring();
        putbool();
        getinteger();
        getfloat();
        getbool();
        sqrt();
    };
    void execute(){};
};
