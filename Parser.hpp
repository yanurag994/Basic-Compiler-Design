#include "./Lexer.hpp"

class Parser
{
public:
    bool parse_file();

private:
    Lexer lexer_handle;
    bool program();
    bool program_header();
    bool program_body();
    bool declaration();
    bool procedure_declaration();
    bool procedure_header();
    bool parameter_list();
    bool parameter();
    bool procedure_body();
    bool variable_declaration();
    bool type_mark();
    bool bound();
    bool statement();
    bool procedure_call();
    bool assignment_statement();
    bool destination();
    bool if_statement();
    bool loop_statement();
    bool return_statement();
    bool identifier();
    bool expression();
    bool arithOp();
    bool relation();
    bool term();
    bool factor();
    bool name();
    bool argument_list();
    bool number();
    bool string();
};