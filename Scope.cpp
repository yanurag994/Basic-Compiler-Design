#include "./Lexer.hpp"
#include "./Scope.hpp"

Symbols::Symbols() : current(new Scope(std::map<std::string, token>(), nullptr))
{
    global = current;
    token newTokenGetBool(IDENTIFIER, tokenMk("getbool"), BOOLEAN_RW);
    global->symbol_table["getbool"] = newTokenGetBool;

    token newTokenGetInteger(IDENTIFIER, tokenMk("getinteger"), INTEGER_RW);
    global->symbol_table["getinteger"] = newTokenGetInteger;

    token newTokenGetFloat(IDENTIFIER, tokenMk("getfloat"), FLOAT_RW);
    global->symbol_table["getfloat"] = newTokenGetFloat;

    token newTokenGetString(IDENTIFIER, tokenMk("getstring"), STRING_RW);
    global->symbol_table["getstring"] = newTokenGetString;

    token newTokenPutBool(IDENTIFIER, tokenMk("putbool"), BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_bool"), BOOLEAN_RW)});
    global->symbol_table["putbool"] = newTokenPutBool;

    token newTokenPutInteger(IDENTIFIER, tokenMk("putinteger"), BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_int"), INTEGER_RW)});
    global->symbol_table["putinteger"] = newTokenPutInteger;

    token newTokenPutFloat(IDENTIFIER, tokenMk("putfloat"), BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_float"), FLOAT_RW)});
    global->symbol_table["putfloat"] = newTokenPutFloat;

    token newTokenPutString(IDENTIFIER, tokenMk("putstring"), BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_double"), STRING_RW)});
    global->symbol_table["putstring"] = newTokenPutString;

    token newTokenSqrt(IDENTIFIER, tokenMk("sqrt"), FLOAT_RW, -1, {token(IDENTIFIER, tokenMk("internal_sqrt"), INTEGER_RW)});
    global->symbol_table["sqrt"] = newTokenSqrt;
}
void Symbols::enterScope() { current = new Scope(std::map<std::string, token>(), current); }
void Symbols::exitScope()
{
    if (current->previous)
        current = current->previous;
    else
        throw std::runtime_error("Hit the exitScope call at outermost scope");
}
token Symbols::HashLookup(token &search_for, bool &global_def, bool definintion)
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
            global->symbol_table[search_for.tokenMark.stringValue] = search_for;
            return global->symbol_table.find(search_for.tokenMark.stringValue)->second;
        }
        else if ((!global_def && current != global) && foundToken == current->symbol_table.end())
        {
            current->symbol_table[search_for.tokenMark.stringValue] = search_for;
            return current->symbol_table.find(search_for.tokenMark.stringValue)->second;
        }
        else
        {
            throw std::runtime_error("Token already defined: " + std::string(search_for.tokenMark.stringValue));
            return search_for;
        }
    }
    else
    {
        auto temp = current;
        while (temp != nullptr)
        {
            auto search_token = temp->symbol_table.find(search_for.tokenMark.stringValue);
            if (search_token != temp->symbol_table.end())
                return search_token->second;
            else
                temp = temp->previous;
        }
        throw std::runtime_error("Token not found: " + std::string(search_for.tokenMark.stringValue));
        return search_for;
    }
}
void Symbols::Completetoken(token &search_for)
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
}