#include "./Lexer.hpp"
#include <vector>
#include <sstream>

int symbol_table_hash_gen;

class Parser
{
private:
    Lexer lexer_handle;
    token cur_tk;
    std::vector<token> def_token_stack;
    bool scan_assume(token_type, token *returned = nullptr);
    bool optional_scan_assume(token_type, token *returned = nullptr);
    bool resync(token_type, bool);
    bool typeCheck(token, token, token_type);
    bool program_header();
    bool program_body();
    bool declaration();
    bool procedure_declaration();
    bool procedure_header();
    bool parameter_list(std::vector<tokenVariable> *);
    bool parameter(std::vector<tokenVariable> *);
    bool procedure_body();
    bool variable_declaration(tokenVariable *);
    bool type_mark(token *returned = nullptr);
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
    void token_lookup()
    {
        //Nothing;
    }

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

struct Scope
{
    std::map<std::string, int> symbol_table;
    Scope *next;
    Scope *previous;
    Scope(std::map<std::string, int> symbol_table, Scope *previous, Scope *next) : symbol_table(symbol_table), previous(previous), next(next){};
};

class Symbols
{
private:
    Scope *current; // pointer to the first node in the list

public:
    std::map<std::string, int> &symbol_table;
    Symbols() : current(new Scope(std::map<std::string, int>(), nullptr, nullptr)), symbol_table(current->symbol_table) {}
    void enterScope();
    void exitScope();
    void enterSoftScope();
    void exitSoftScope();
};

