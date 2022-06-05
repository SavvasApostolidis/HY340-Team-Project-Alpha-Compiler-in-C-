#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <list>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>

#define BUCKETS 100

typedef enum AlphaType
{
    LOCAL_VAR,
    GLOBAL_VAR,
    FORMAL,
    USER_FUNCTION,
    LIB_FUNCTION,
}type_t;

typedef enum scopespace
{
    programVar,
    functionLocal,
    formalArg
}scopespace_t;


typedef enum symbol
{
    var_s,
    programfunc_s,
    libraryfunc_s
}symbol_t;

class SymbolTableEntry
{
    public:
        std::string             name;
        type_t                  type;
        scopespace_t            space;
        symbol_t                sym_scope;
        unsigned int            taddress;
        unsigned int            scope;
        unsigned int            line;
        unsigned int            offset;
        unsigned int            totalLocal;
        int                     isActive;
        std::list<std::string>  arguments;
        std::list<unsigned int> target_code_return_list;

        bool                    function_has_return = true;

        SymbolTableEntry(std::string    name   ,
                         type_t         type   ,
                         unsigned int   scope  ,
                         unsigned int   line   ,
                         int isActivate
                         )
        {
            this->name      = name;
            this->type      = type;
            this->scope     = scope;
            this->line      = line;
            this->isActive  = isActivate;
        };

        SymbolTableEntry(std::string    name   ,
                         type_t         type   ,
                         symbol_t       sym_scope,
                         unsigned int   scope  ,
                         unsigned int   line   ,
                         int isActivate
                         )
        {
            this->name      = name;
            this->type      = type;
            this->sym_scope = sym_scope;
            this->scope     = scope;
            this->line      = line;
            this->isActive  = isActivate;
        };

};

class SymbolTable
{
    private:
        std::map<int , std::list<SymbolTableEntry*> > symbol_table;
        std::map<int , std::list<SymbolTableEntry*> > scope_list;
        std::vector<SymbolTableEntry*> library_functions;
    public:
        SymbolTable();

        int hash(std::string name);

        bool insert(SymbolTableEntry* new_entry);

        SymbolTableEntry* lookup(std::string name);

        SymbolTableEntry* lookup_scope(std::string name , int scope);

        SymbolTableEntry* lookup_lib_function(std::string name);

        bool hide(int scope);

        void print();
    private:
        SymbolTableEntry* search(std::list<SymbolTableEntry*>& symbols , std::string name);
};

#endif
