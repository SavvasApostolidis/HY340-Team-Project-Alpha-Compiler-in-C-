#include "SymbolTable.h"

std::ofstream symbol_table_file("symbolTable.txt");

SymbolTable::SymbolTable()
{
    for(int i=0; i < BUCKETS; i++)
        this->symbol_table[i] = std::list<SymbolTableEntry*>();
    for(int i=0; i < BUCKETS; i++)
        this->scope_list[i] = std::list<SymbolTableEntry*>();

    (this->library_functions).push_back(new SymbolTableEntry("print" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("input" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("objectmemberkeys" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("objecttotalmembers" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("objectcopy" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("totalarguments" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("argument" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("typeof" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("strtonum" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("sqrt" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("cos" , LIB_FUNCTION , 0 , 0 , 1));
    (this->library_functions).push_back(new SymbolTableEntry("sin" , LIB_FUNCTION , 0 , 0 , 1));                           
}

int SymbolTable::hash(std::string name)
{
    int hash_number = 0;

    for(unsigned int i = 0; i < name.length(); i++)
    {
        hash_number += name[i];
    }

    return (hash_number % BUCKETS);
}

bool SymbolTable::insert(SymbolTableEntry* new_entry)
{
    int hash; 
    int scope;
    std::string var_name  = std::string(new_entry->name);

    hash = this->hash(var_name);
    scope = new_entry->scope;

    (this->symbol_table).at(hash).push_front(new_entry);
    
    if(scope > BUCKETS)
        if((this->scope_list).find(scope) == (this->scope_list).end())
            this->scope_list[scope] = std::list<SymbolTableEntry*>();

    (this->scope_list).at(scope).push_front(new_entry);   

    return true;
}

SymbolTableEntry* SymbolTable::lookup(std::string name)
{
    return ( this->search( (this->symbol_table).at(this->hash(name))  ,  name) );        
}

SymbolTableEntry* SymbolTable::lookup_scope(std::string name , int scope)
{   
    if(scope > BUCKETS)
        if((this->scope_list).find(scope) == (this->scope_list).end())
            return nullptr;

    return ( this->search( (this->scope_list).at(scope)  , name) );    
}

SymbolTableEntry* SymbolTable::lookup_lib_function(std::string name)
{
    for (auto function : this->library_functions)
    {
        if(function->name == name)
        { 
            return function;
        }
    }
    return nullptr;
}

SymbolTableEntry* SymbolTable::search(std::list<SymbolTableEntry*>& symbols , std::string name_s)
{
    if(symbols.empty())
        return nullptr;

    for (auto symbol : symbols)
    {
        if(symbol->isActive && symbol->name == name_s)
        { 
            return symbol;
        }
    }
    return nullptr;        
}

bool SymbolTable::hide(int scope)
{
    if((this->scope_list).find(scope) == (this->scope_list).end())
        return false;

    if((this->scope_list).empty())
        return false;

    for (auto x: (this->scope_list).at(scope) )
        x->isActive = 0;

    return true;    
}

void SymbolTable::print()
{
    for(auto buckets : (this->scope_list))
    {   
        if(!buckets.second.empty())
            symbol_table_file << "\n-----------     Scope#" << buckets.first << "     -----------\n\n";
            
        for (std::list<SymbolTableEntry*>::reverse_iterator symbol_entry = buckets.second.rbegin(); symbol_entry != buckets.second.rend(); ++symbol_entry)
        {
            symbol_table_file << "\"" << (*symbol_entry)->name << "\"" << " ";             
            
            switch((*symbol_entry)->type)
            {
                case LIB_FUNCTION:
                {
                    symbol_table_file << "[library function]";
                    break;
                }
                case USER_FUNCTION:
                {
                    symbol_table_file << "[user function]";
                    break;
                }
                case GLOBAL_VAR:
                {
                    symbol_table_file << "[global variable]";
                    break;
                }
                case LOCAL_VAR:
                {
                    symbol_table_file << "[local variable]";
                    break;
                }case FORMAL:
                {
                    symbol_table_file << "[formal argument]";
                    break;
                }
            }

            
            symbol_table_file << " " << "( line " << (*symbol_entry)->line << " )" 
            << "  ( scope "       << (*symbol_entry)->scope  << " )";

            if(!((*symbol_entry)->type == LIB_FUNCTION) && !((*symbol_entry)->type == USER_FUNCTION))
                symbol_table_file << "  ( offset "      << (*symbol_entry)->offset << " )"
                          << "  ( scope space " << (*symbol_entry)->space  << " )" << "\n";
            else
                symbol_table_file << "\n";            
        }
    }
}

