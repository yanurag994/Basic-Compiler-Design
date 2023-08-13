#include "./Lexer.hpp"
#include <vector>
#include <sstream>
#include <cstring>

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
    Scope *global;
    Scope *current; // pointer to the first node in the list
public:
    Symbols() : current(new Scope(std::map<std::string, token>(), nullptr))
    {
        global = current;
        token newTokenGetBool(IDENTIFIER, tokenMk("getbool"), 1000, BOOLEAN_RW);
        global->symbol_table["getbool"] = newTokenGetBool;

        token newTokenGetInteger(IDENTIFIER, tokenMk("getinteger"), 1001, INTEGER_RW);
        global->symbol_table["getinteger"] = newTokenGetInteger;

        token newTokenGetFloat(IDENTIFIER, tokenMk("getfloat"), 1002, FLOAT_RW);
        global->symbol_table["getfloat"] = newTokenGetFloat;

        token newTokenGetString(IDENTIFIER, tokenMk("getstring"), 1003, STRING_RW);
        global->symbol_table["getstring"] = newTokenGetString;

        token newTokenPutBool(IDENTIFIER, tokenMk("putbool"), 1004, BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_bool"), 1050, BOOLEAN_RW)});
        global->symbol_table["putbool"] = newTokenPutBool;

        token newTokenPutInteger(IDENTIFIER, tokenMk("putinteger"), 1005, BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_int"), 1051, INTEGER_RW)});
        global->symbol_table["putinteger"] = newTokenPutInteger;

        token newTokenPutFloat(IDENTIFIER, tokenMk("putfloat"), 1006, BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_float"), 1052, FLOAT_RW)});
        global->symbol_table["putfloat"] = newTokenPutFloat;

        token newTokenPutString(IDENTIFIER, tokenMk("putstring"), 1007, BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_double"), 1053, STRING_RW)});
        global->symbol_table["putstring"] = newTokenPutString;

        token newTokenSqrt(IDENTIFIER, tokenMk("sqrt"), 1008, FLOAT_RW, -1, {token(IDENTIFIER, tokenMk("internal_sqrt"), 1054, INTEGER_RW)});
        global->symbol_table["sqrt"] = newTokenSqrt; 
    }
    void enterScope() { current = new Scope(current->symbol_table, current); };
    void enterSoftScope() { current = new Scope(current->symbol_table, current); };
    void exitScope()
    {
        if (current->previous)
            current = current->previous;
        else
            throw std::runtime_error("Hit the exitScope call at outermost scope");
    };
    token HashLookup(token &search_for)
    {
        auto foundToken = current->symbol_table.find(search_for.tokenMark.stringValue);
        if (foundToken == current->symbol_table.end())
        {
            search_for.tokenHash = Hashgen++;
            current->symbol_table[search_for.tokenMark.stringValue] = search_for;
        }
        return current->symbol_table.find(search_for.tokenMark.stringValue)->second;
    };
    void Completetoken(token &search_for)
    {
        auto foundToken = current->symbol_table.find(search_for.tokenMark.stringValue);
        foundToken->second = search_for;
        return;
    };
    void CompleteDeclPrevtoken(token &search_for)
    {
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
    bool scan_assume(token_type);
    bool optional_scan_assume(token_type);
    bool scan_assume(token_type, token &returned);
    bool optional_scan_assume(token_type, token &returned);
    bool resync(token_type, bool);
    bool typeCheck(token *, token *, token_type);
    bool program_header();
    bool program_body();
    bool declaration();
    bool procedure_declaration();
    bool procedure_header();
    bool parameter_list(std::vector<token> &argType);
    bool parameter(token &);
    bool procedure_body();
    bool variable_declaration(token &);
    bool type_mark(token_type &);
    bool statement();
    bool procedure_call();
    bool assignment_statement();
    bool destination();
    bool if_statement();
    bool loop_statement();
    bool return_statement();
    bool expression();
    bool cond_expression();
    bool arithOp();
    bool relation();
    bool term();
    bool factor();
    bool argument_list();

public:
    std::stringstream output;
    bool program();
    Symbols *symbols;
    Parser(std::string filename) : lexer_handle(filename)
    {
        symbols = new Symbols();
        cur_tk = lexer_handle.scan();
    }
};
