#include "./Lexer.hpp"
#include "./Scope.hpp"

Symbols::Symbols() : current(new Scope(std::map<std::string, token>(), nullptr))
{
    global = current;
    token newTokenGetBool(IDENTIFIER, tokenMk("getbool"), "getbool", BOOLEAN_RW);
    global->symbol_table["getbool"] = newTokenGetBool;

    token newTokenGetInteger(IDENTIFIER, tokenMk("getinteger"), "getinteger", INTEGER_RW);
    global->symbol_table["getinteger"] = newTokenGetInteger;

    token newTokenGetFloat(IDENTIFIER, tokenMk("getfloat"), "getfloat", FLOAT_RW);
    global->symbol_table["getfloat"] = newTokenGetFloat;

    token newTokenGetString(IDENTIFIER, tokenMk("getstring"), "getstring", STRING_RW);
    global->symbol_table["getstring"] = newTokenGetString;

    token newTokenPutBool(IDENTIFIER, tokenMk("putbool"), "putbool", BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_bool"), "1050", BOOLEAN_RW)});
    global->symbol_table["putbool"] = newTokenPutBool;

    token newTokenPutInteger(IDENTIFIER, tokenMk("putinteger"), "putinteger", BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_int"), "1051", INTEGER_RW)});
    global->symbol_table["putinteger"] = newTokenPutInteger;

    token newTokenPutFloat(IDENTIFIER, tokenMk("putfloat"), "putfloat", BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_float"), "1052", FLOAT_RW)});
    global->symbol_table["putfloat"] = newTokenPutFloat;

    token newTokenPutString(IDENTIFIER, tokenMk("putstring"), "putstring", BOOLEAN_RW, -1, {token(IDENTIFIER, tokenMk("internal_double"), "1053", STRING_RW)});
    global->symbol_table["putstring"] = newTokenPutString;

    token newTokenSqrt(IDENTIFIER, tokenMk("sqrt"), "sqrt", FLOAT_RW, -1, {token(IDENTIFIER, tokenMk("internal_sqrt"), "1054", INTEGER_RW)});
    global->symbol_table["sqrt"] = newTokenSqrt;
}
void Symbols::enterScope()
{
    level++;
    current = new Scope(std::map<std::string, token>(), current);
}
void Symbols::exitScope()
{
    if (current->previous)
    {
        level--;
        current = current->previous;
    }
    else
        throw std::runtime_error("Hit the exitScope call at outermost scope");
}
token Symbols::HashLookup(token &search_for, bool &global_def, bool definintion)
{
    if (definintion)
    {
        auto foundToken = current->symbol_table.find(search_for.tokenMark.stringValue);
        auto glbfoundToken = global->symbol_table.find(search_for.tokenMark.stringValue);
        if (foundToken != current->symbol_table.end() && foundToken->second.tokenHash==std::to_string(level) + search_for.tokenMark.stringValue)
        {
            throw std::runtime_error("Token already defined at current scope: " + std::string(search_for.tokenMark.stringValue));
            return search_for;
        }
        else if ((global_def || current == global) && glbfoundToken != global->symbol_table.end())
        {
            global_def = false;
            throw std::runtime_error("Token already defined at global scope: " + std::string(search_for.tokenMark.stringValue));
            return search_for;
        }
        else if ((global_def || current == global))
        {
            global_def = false;
            search_for.global_var = true;
            search_for.tokenHash = std::to_string(level) + search_for.tokenMark.stringValue;
            global->symbol_table[search_for.tokenMark.stringValue] = search_for;
            return global->symbol_table.find(search_for.tokenMark.stringValue)->second;
        }
        else
        {
            search_for.tokenHash = std::to_string(level) + search_for.tokenMark.stringValue;
            current->symbol_table[search_for.tokenMark.stringValue] = search_for;
            return current->symbol_table.find(search_for.tokenMark.stringValue)->second;
        }
    }
    else
    {
        auto temp = current;
        while (temp)
        {
            auto search_token = temp->symbol_table.find(search_for.tokenMark.stringValue);
            if (search_token != temp->symbol_table.end())
                return search_token->second;
            else if (search_token == temp->symbol_table.end())
                temp = temp->previous;
        }
        throw std::runtime_error("Token not found: " + std::string(search_for.tokenMark.stringValue));
    }
}
void Symbols::Completetoken(token &search_for)
{
    if (search_for.global_var)
    {
        auto glbfoundToken = global->symbol_table.find(search_for.tokenMark.stringValue);
        if (glbfoundToken != global->symbol_table.end())
            glbfoundToken->second = search_for;
        return;
    }
    else
    {

        auto temp = current;
        while (temp)
        {
            auto search_token = temp->symbol_table.find(search_for.tokenMark.stringValue);
            if (search_token != temp->symbol_table.end())
            {
                search_token->second = search_for;
                return;
            }
            else if (search_token == temp->symbol_table.end())
                temp = temp->previous;
        }
    }
}
void Symbols::Addtoken(token &search_for)
{
    current->symbol_table[search_for.tokenMark.stringValue]=search_for;
    return;
}
std::vector<token> Symbols::get_internal_args()
{
    std::vector<token> internal_args;
    if (level == 0)
        return internal_args;
    else
    {
        for (auto i : current->symbol_table)
        {
            if (i.second.llvm_value)
                internal_args.push_back(i.second);
        }
        return internal_args;
    }
}