#include "./Lexer.hpp"
#include <vector>
#include <sstream>
#include <cstring>
#include <iostream>

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
        token newTokenGetBool(IDENTIFIER, tokenMk("getbool"), "%l_"+std::to_string(1000), BOOLEAN_RW);
        newTokenGetBool.procedure=true;
        global->symbol_table["getbool"] = newTokenGetBool;

        token newTokenGetInteger(IDENTIFIER, tokenMk("getinteger"), "%l_"+std::to_string(1001), INTEGER_RW);
        newTokenGetInteger.procedure=true;
        global->symbol_table["getinteger"] = newTokenGetInteger;

        token newTokenGetFloat(IDENTIFIER, tokenMk("getfloat"), "%l_"+std::to_string(1002), FLOAT_RW);
        newTokenGetFloat.procedure=true;
        global->symbol_table["getfloat"] = newTokenGetFloat;

        token newTokenGetString(IDENTIFIER, tokenMk("getstring"), "%l_"+std::to_string(1003), STRING_RW);
        newTokenGetString.procedure=true;
        global->symbol_table["getstring"] = newTokenGetString;

        token newTokenPutBool(IDENTIFIER, tokenMk("putbool"), "%l_"+std::to_string(1004), BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_bool"), std::to_string(1050), BOOLEAN_RW)});
        newTokenPutBool.procedure=true;
        global->symbol_table["putbool"] = newTokenPutBool;

        token newTokenPutInteger(IDENTIFIER, tokenMk("putinteger"), "%l_"+std::to_string(1005), BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_int"), std::to_string(1051), INTEGER_RW)});
        newTokenPutInteger.procedure=true;
        global->symbol_table["putinteger"] = newTokenPutInteger;

        token newTokenPutFloat(IDENTIFIER, tokenMk("putfloat"), "%l_"+std::to_string(1006), BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_float"), std::to_string(1052), FLOAT_RW)});
        newTokenPutFloat.procedure=true;
        global->symbol_table["putfloat"] = newTokenPutFloat;

        token newTokenPutString(IDENTIFIER, tokenMk("putstring"), "%l_"+std::to_string(1007), BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_double"), std::to_string(1053), STRING_RW)});
        newTokenPutString.procedure=true;
        global->symbol_table["putstring"] = newTokenPutString;

        token newTokenSqrt(IDENTIFIER, tokenMk("sqrt"), "%l_"+std::to_string(1008), FLOAT_RW, -1, {token(IDENTIFIER, tokenMk("internal_sqrt"), std::to_string(1054), INTEGER_RW)});
        newTokenSqrt.procedure=true;
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
    Lexer lexer_handle;
    token cur_tk;
    token prev_tk;
    bool global_flag = false;
    bool scan_assume(token_type);
    bool optional_scan_assume(token_type);
    bool scan_assume(token_type, token &, bool);
    bool optional_scan_assume(token_type, token &, bool);
    bool resync(token_type, bool);
    bool typeCheck(token &, token &, token &, token_type, std::stringstream &);
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
    bool procedure_call(token &, std::stringstream &);
    bool assignment_statement(token &);
    bool destination(token &);
    bool if_statement();
    bool loop_statement();
    bool return_statement();
    bool expression(token &, std::stringstream &);
    bool cond_expression(token &);
    bool arithOp(token &, std::stringstream &, token_type = UNKNOWN);
    bool relation(token &, std::stringstream &);
    bool factor(token &, std::stringstream &);
    bool argument_list(token &, std::stringstream &);

public:
    std::vector<std::stringstream> buffer;
    std::stringstream output;
    std::stringstream global_decl;
    bool program();
    Symbols *symbols;
    int label_gen = 10;
    Parser(std::string filename) : lexer_handle(filename)
    {
        auto globalstream = std::stringstream();
        buffer.push_back(std::move(globalstream));
        symbols = new Symbols();
        cur_tk = lexer_handle.scan();
    }
};

inline std::string getLLVMType(token_type dataType = UNKNOWN)
{
    if (dataType == INTEGER_RW || dataType == INTEGER_VAL)
        return "i32";
    if (dataType == FLOAT_RW || dataType == FLOAT_VAL)
        return "float";
    if (dataType == STRING_RW || dataType == INTEGER_VAL)
        return "i32";
    if (dataType == BOOLEAN_RW)
        return "i8";
    return "unknown";
}

inline std::string getLLVMIntitializer(token_type dataType = UNKNOWN)
{
    if (dataType == INTEGER_RW)
        return "0";
    if (dataType == FLOAT_RW)
        return "0.000000e+00";
    if (dataType == STRING_RW)
        return "i32";
    if (dataType == BOOLEAN_RW)
        return "0";
    return "unknown";
}

inline std::string getLLVMform(token &tk)
{
    if (tk.type == IDENTIFIER)
        return tk.tokenHash;
    if (tk.type == INTEGER_VAL)
        return std::to_string(tk.tokenMark.intValue);
    return "unknown";
}

inline std::string getLLVMvar_val(token &tk)
{
    if (tk.type == IDENTIFIER)
        return tk.tokenHash;
    if (tk.type == INTEGER_VAL)
        return std::to_string(tk.tokenMark.intValue);
    if (tk.type == FLOAT_VAL)
        return std::to_string(tk.tokenMark.doubleValue);
    if (tk.type == BOOLEAN_RW)
        return std::to_string(tk.tokenMark.intValue);
    if (tk.type == STRING_VAL)
        return tk.tokenMark.stringValue;
    return "unknown";
}

inline std::string getLLVMop(token &tk)
{
    if (tk.type == EQUALITY)
        return "eq";
    if (tk.type == (token_type)'<')
        return "slt";
    if (tk.type == LESS_EQUAL)
        return "sle";
    if (tk.type == (token_type)'>')
        return "sgt";
    if (tk.type == GREATER_EQUAL)
        return "sge";
    if (tk.type == NOT_EQUAL)
        return "ne";
    return "unknown";
}