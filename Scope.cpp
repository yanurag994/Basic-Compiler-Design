#include <map>
#include "./Lexer.hpp"
#include "./Scope.hpp"

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
};