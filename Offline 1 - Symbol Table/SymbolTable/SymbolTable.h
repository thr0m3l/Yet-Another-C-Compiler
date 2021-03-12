#include "ScopeTable.h"
using namespace std;

class SymbolTable
{
    ScopeTable *current;

    void enterScope(int id, int buckets)
    {
        ScopeTable *st = new ScopeTable(id, buckets, current);
        current = st;

        cout << "New ScopeTable with id " << id << " created" << endl;
    }

    void exitScope()
    {
        if (current == NULL)
        {
            cout << "No ScopeTable in the SymbolTable" << endl;
            return;
        }

        cout << "ScopeTable with id " << current->getID() << " removed" << endl;

        current = current->getParentScope();
    }

    bool insertSymbol(string name, string type)
    {
        if (current == NULL)
        {
            cout << "No ScopeTable in the SymbolTable" << endl;
            return false;
        }

        return current->insertSymbol(name, type);
    }

    bool deleteSymbol(string name)
    {
        if (current == NULL)
        {
            cout << "No ScopeTable in the SymbolTable" << endl;
            return false;
        }

        return current->deleteSymbol(name);
    }

    SymbolInfo *lookup(string name)
    {
        if (current == NULL)
        {
            cout << "No ScopeTable in the SymbolTable" << endl;
            return NULL;
        }

        ScopeTable *temp = current;
        SymbolInfo *symbol = NULL;

        while (temp != NULL)
        {
            symbol = temp->lookUp(name);

            if (symbol != NULL)
            {
                return symbol;
            }
            temp = temp->getParentScope();
        }

        return NULL;
    }

    void printCurrent()
    {
        if (current == NULL)
        {
            cout << "No ScopeTable in the SymbolTable" << endl;
            return;
        }

        current->print();
    }

    void printAll()
    {
        ScopeTable *itr = current;

        while (itr != NULL)
        {
            itr->print();
            cout << endl;
            itr = itr->getParentScope();
        }
    }
};