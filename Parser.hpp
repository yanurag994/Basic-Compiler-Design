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
    int Hashgen = 1000;
    Scope *current; // pointer to the first node in the list
public:
    Symbols() : current(new Scope(std::map<std::string, token>(), nullptr)) {}
    void enterScope() { current = new Scope(std::map<std::string, token>(), current); };
    void enterSoftScope() { current = new Scope(current->symbol_table, current); };
    void exitScope()
    {
        if (current->previous)
        {
            current = current->previous;
        }
        else
        {
            throw std::runtime_error("Hit the exitScope call at outermost scope");
        }
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
    }
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
