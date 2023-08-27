#pragma once
#include <map>

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
    Scope *current; // pointer to the first node in the list
    Scope *global;
public:
    Symbols();
    void enterScope();
    void exitScope();
    token HashLookup(token &, bool &, bool = false);
    void Completetoken(token &);
};
