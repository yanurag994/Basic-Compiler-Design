#include "./Lexer.hpp"

class Parser
{
private:
    Lexer lexer_handle;
    token cur_tk;
    bool scan_assume(token_type, token *returned = nullptr);
    bool optional_scan_assume(token_type, token *returned = nullptr);
    bool resync(token_type, bool);
    bool typeCheck(token, token, token_type);
    bool program_header();
    bool program_body();
    bool declaration();
    bool procedure_declaration();
    bool procedure_header();
    bool parameter_list();
    bool parameter();
    bool procedure_body();
    bool variable_declaration();
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

public:
    bool program();
    Parser(std::string filename) : lexer_handle(filename) { cur_tk = lexer_handle.scan(); }
};