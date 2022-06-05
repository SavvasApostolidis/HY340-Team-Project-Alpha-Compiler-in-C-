#include "Quad.h"

extern int scope;

extern int br_cont_counter_allowed;
extern int function_name_counter;

extern int call_was_used;

extern bool redifine_name_function;
extern bool redifined_arguments_function;
extern bool is_anonymous_function;
extern bool check_for_name;

extern SymbolTable *symbolTable;
extern SymbolTableEntry *function_curr;

extern std::stack<SymbolTableEntry *> functions_stack;

std::ofstream quads_file("quads.txt");

extern FILE*  syntax_file;

//Array with the emit quads
std::vector<quad> quads = std::vector<quad>();

extern unsigned int currentQuad;

unsigned int variable_temp_counter = 0;

extern unsigned int functionLocalOffset;
extern unsigned int programVarOffset;
extern unsigned int formalArgOffset;
extern unsigned int scopeSpaceCounter;

void functionMissuse(SymbolTableEntry *lvalue, std::string message)
{
    if (lvalue != nullptr)
    {
        if (lvalue->type == LIB_FUNCTION || lvalue->type == USER_FUNCTION)
        {
            ERROR(message, yylineno);
            exit(0);
        }
    }
}

bool searchForSameArguments(std::string new_argument)
{
    return (std::find((function_curr->arguments).begin(), (function_curr->arguments).end(), new_argument) != (function_curr->arguments).end());
}

void addFunctionArgument(char *name)
{
    SymbolTableEntry *new_argument_entry;
    AlphaType type = FORMAL;

    std::string new_argument = std::string(name);

    if (symbolTable->lookup_lib_function(new_argument) == nullptr)
    {
        if (!searchForSameArguments(new_argument))
        {
            (function_curr->arguments).push_back(new_argument);

            new_argument_entry = new SymbolTableEntry(new_argument, type, var_s ,  scope , yylineno, 1);
            setSpaceAndOffset(new_argument_entry);
            increaseCurrentScopeOffset();

            symbolTable->insert(new_argument_entry);
        }
        else
        {
            redifined_arguments_function = true;
            ERROR("formal argument " + new_argument + " redeclared", yylineno);
            exit(0);
        }
    }
    else
    {
        redifined_arguments_function = true;
        ERROR("trying to pass a library function as argument", yylineno);
        exit(0);
    }
}

SymbolTableEntry *handleFunctionPrefix(char *id)
{
    SymbolTableEntry *new_function_entry;

    type_t type = USER_FUNCTION;
    symbol_t symscope = programfunc_s;

    std::string function_name = std::string(id);

    new_function_entry = new SymbolTableEntry(function_name, type, programfunc_s,scope, yylineno, 1);
    new_function_entry->sym_scope = symscope;

    return new_function_entry;
}

void handleFunctionId(char *id)
{
    std::string function_name = std::string(id);

    if (symbolTable->lookup_lib_function(function_name) != nullptr)
        redifine_name_function = true;

    if (symbolTable->lookup_scope(function_name, scope) != nullptr)
        redifine_name_function = true;
}

void handleFunctionInsert()
{
    if (is_anonymous_function)
    {
        if (!redifined_arguments_function)
            symbolTable->insert(function_curr);
        else
            redifined_arguments_function = false;
        is_anonymous_function = false;
    }
    else
    {
        if (!redifine_name_function)
        {
            if (!redifined_arguments_function)
                symbolTable->insert(function_curr);
            else
                redifined_arguments_function = false;
        }
        else
        {
            redifine_name_function = false;
            ERROR("trying to redifine the variable/function " + function_curr->name, yylineno);
            exit(0);
        }
    }
}

void handleIdVariable(SymbolTableEntry **lvalue, char *id)
{
    if (check_for_name)
    {
        SymbolTableEntry *new_variable = nullptr;
        SymbolTableEntry *refer_variable;
        AlphaType type;

        bool is_refers = false;

        std::string new_variable_name = std::string(id);

        fprintf(syntax_file, "lvalue: [ ID -> %s] \n" , id);

        //an htan lib function perastike apeu8eias pros ta panw
        if ((*lvalue = symbolTable->lookup_lib_function(new_variable_name)) != nullptr)
        {
            fprintf(syntax_file, "\tVariable with name %s refers to a library function\n" , id);
        }
        else
        {
            for (int i = scope; i >= 0; i--)
            {
                if ((refer_variable = symbolTable->lookup_scope(new_variable_name, i)) != nullptr)
                {
                    if (refer_variable->type == USER_FUNCTION)
                    {
                        fprintf(syntax_file, "\tVariable with name %s refers to a user functionin line : %d\n" , id , refer_variable->line);
                        is_refers = true;
                        break;
                    }
                    else if (refer_variable->scope == scope && refer_variable->type == LOCAL_VAR)
                    {
                        fprintf(syntax_file , "\tVariable with name %s refers to a local variable in line : %d\n" , id , refer_variable->line );
                        is_refers = true;
                        break;
                    }
                    else if (refer_variable->scope == scope && refer_variable->type == FORMAL)
                    {
                        fprintf(syntax_file , "\tVariable with name %s refers to an argument in line : %d\n" , id , refer_variable->line);
                        is_refers = true;
                        break;
                    }
                    else if (refer_variable->scope == 0)
                    {
                        fprintf(syntax_file, "\tVariable with name %s refers to a global variable in line : %d\n" , id , refer_variable->line);
                        is_refers = true;
                        break;
                    }
                    else if (!functions_stack.empty())
                    {
                        if ((refer_variable->scope < scope && (functions_stack.top())->scope < refer_variable->scope) && refer_variable->type == FORMAL)
                        {
                            fprintf(syntax_file, "\tVariable with name %s refers to an argument in line : %d\n" , id , refer_variable->line);
                            is_refers = true;
                            break;
                        }
                        if ((refer_variable->scope < scope && (functions_stack.top())->scope < refer_variable->scope) && refer_variable->type == LOCAL_VAR)
                        {
                            fprintf(syntax_file, "\tVariable with name %s refers to an local variable in line : %d\n" , id , refer_variable->line);
                            is_refers = true;
                            break;
                        }
                        else if (refer_variable->scope < scope && (functions_stack.top())->scope >= refer_variable->scope)
                        {
                            ERROR("cannot access variable with name " + new_variable_name, yylineno);
                            exit(0);
                            is_refers = true;
                            break;
                        }
                    }
                    else if (refer_variable->scope > 0 && refer_variable->type == LOCAL_VAR)
                    {
                        fprintf(syntax_file, "\tVariable with name %s refers to a local variable in line : %d\n" , id , refer_variable->line);
                        is_refers = true;
                        break;
                    }
                }
            }
            if (!is_refers)
            {
                type = ((scope == 0) ? GLOBAL_VAR : LOCAL_VAR);

                new_variable = new SymbolTableEntry(new_variable_name, type, var_s,scope, yylineno, 1);
                setSpaceAndOffset(new_variable);
                increaseCurrentScopeOffset();

                symbolTable->insert(new_variable);
                *lvalue = new_variable;
            }
            else
            {
                //akoma kai an den eixe access to passarw panw gia elegxw gia peretero suntaktika la8h
                *lvalue = refer_variable;
            }
        }
    }
    check_for_name = true;
}

void handleLocalVariable(SymbolTableEntry **lvalue, char *id)
{
    
    SymbolTableEntry *new_local;
    SymbolTableEntry *refers;
    AlphaType type = LOCAL_VAR;
    
    bool is_lib = false;

    std::string name = std::string(id);

    fprintf(syntax_file, " lvalue: [ Local ID -> Local %s ] \n" , id);

    if ((refers = symbolTable->lookup_lib_function(name)) != nullptr)
    {
        ERROR("local variable refers to a library function ", yylineno);
        exit(0);
        *lvalue = refers;
        is_lib = true;
    }
    
    if (!is_lib)
    {
        if ((refers = symbolTable->lookup_scope(name, scope)) != nullptr)
        {
            if (refers->type == LOCAL_VAR)
                fprintf(syntax_file,  "Local : %s refers to exist local variable\n" , id);
            else if (refers->type == FORMAL)
                fprintf(syntax_file, "Local : %s refers to an argument variable\n" , id);
            else if (refers->type == GLOBAL_VAR)
                fprintf(syntax_file, "Local : %s refers to exist global variable\n" , id);
            *lvalue = refers;
        }
        else
        {
            
            if (scope == 0){
                type = GLOBAL_VAR;
            }
            new_local = new SymbolTableEntry(name, type, var_s,scope, yylineno, 1);
            
            
            setSpaceAndOffset(new_local);

            increaseCurrentScopeOffset();
           
            symbolTable->insert(new_local);

            *lvalue = new_local;
          
        }
    }
}

void handleGlobalVariable(SymbolTableEntry **lvalue, char *id)
{
    std::string var_name = std::string(id);

    fprintf(syntax_file, " [ lvalue: ::ID -> ::%s ] \n" , id);

    if ((*lvalue = symbolTable->lookup_scope(var_name, 0)) == nullptr){
        ERROR(var_name + " variable/function is not defined in the global scope ", yylineno);
        exit(0);
        }
    else
    {
        if ((*lvalue)->type == USER_FUNCTION || (*lvalue)->type == LIB_FUNCTION)
            fprintf(syntax_file, "\t%s refers to a function in line %d" ,const_cast<char*>(((*lvalue)->name).c_str()) , (*lvalue)->line);
        else
        {
            fprintf(syntax_file, "\t%s refers to a global var in line %d" ,const_cast<char*>(((*lvalue)->name).c_str()) , (*lvalue)->line);
        }
    }
}

char *newTemporaryFunctionName()
{
    is_anonymous_function = true;

    return strdup(const_cast<char *>(std::string("$function_" + std::to_string(function_name_counter++)).c_str()));
}

quad::quad(iopcode_t op,
           expr* result,
           expr* arg1,
           expr* arg2,
           unsigned int label,
           unsigned int line)
{
    this->op        = op;
    this->result    = result;
    this->arg1      = arg1;
    this->arg2      = arg2;
    this->label     = label;    //target label to jump to
    this->line      = line;     //yyline
}

expr::expr(SymbolTableEntry* sym)
{
    assert(sym != nullptr);

    this->symbol = sym;

    switch(symbol->sym_scope)
    {
        case var_s          :   this->type = var_e; break;
        case programfunc_s  :   this->type = programfunc_e; break;
        case libraryfunc_s  :   this->type = libraryfunc_e; break;
        default             :   assert(0);
    }
}

expr::expr(expr_t type)
{
    this->type = type;
}

expr::expr(int intnumberConst)
{
    this->type           =  constint_e;
    this->intnumberConst =  intnumberConst;

    this->symbol         = new SymbolTableEntry(std::to_string(intnumberConst) , LOCAL_VAR , scope , yylineno , 1);
}

expr::expr(expr_t type , std::string stringConst)
{
    this->type        = type;
    this->stringConst = stringConst;

    this->symbol      = new SymbolTableEntry(stringConst , LOCAL_VAR , scope , yylineno , 1);
}

expr::expr(expr_t type , double numberConst)
{
    this->type        =  type;
    this->numberConst =  numberConst;

    this->symbol      = new SymbolTableEntry(std::to_string(numberConst) , LOCAL_VAR , scope , yylineno , 1);
}

expr::expr(expr_t type , bool boolConst)
{
    this->type      = type;
    this->boolConst = boolConst;

    this->symbol    = new SymbolTableEntry("" , LOCAL_VAR , scope , yylineno , 1);

    (boolConst)     ? this->symbol->name = "\"true\"" : this->symbol->name = "\"false\"";
}

void emitArithmeticOperation(iopcode_t op ,
                             expr** result ,
                             expr* arg1   ,
                             expr* arg2)
{
    *result           = new expr(arithmetic_e);
    if(isTemporaryExpression(arg1))
      (*result)->symbol = arg1->symbol;
    else if(isTemporaryExpression(arg2))
      (*result)->symbol = arg2->symbol;
    else
      (*result)->symbol = newTemporaryVariable();

    emit(op , *result , arg1 , arg2 , 0 , yylineno);
}

void emitReOperation(iopcode_t op ,
                       expr** result ,
                       expr* arg1   ,
                       expr* arg2)
{
    *result              = new expr(bool_e);

    (*result)->truelist  = std::list<unsigned int>();
    (*result)->falselist = std::list<unsigned int>();

    (*result)->truelist.push_back(nextQuadLabel());
    (*result)->falselist.push_back(nextQuadLabel()+1);

    //tobepatched
    emit(op , nullptr , arg1 , arg2 , 0 , yylineno);

    //tobepatched
    emit(jump , nullptr , nullptr , nullptr , 0, yylineno);
}

void emitBoolOperation(iopcode_t op  ,
                       expr** result ,
                       expr*  arg1   ,
                       expr* arg2)
{
    *result           = new expr(bool_e);
    (*result)->symbol = newTemporaryVariable();

    emit(op , *result , arg1 , arg2 , 0 , yylineno);
}

void emit(iopcode op ,
          expr* result ,
          expr* arg1 ,
          expr* arg2 ,
          unsigned int label ,
          unsigned int line)
{

    quad new_quad(op,result,arg1,arg2,label,line);


    quads.push_back(new_quad);

    currentQuad = quads.size();
}

expr* emitIfTableItem(expr* expression)
{
    if(expression->type != tableitem_e)
        return expression;
    else
    {
        expr* result = new expr(var_e);
        result->symbol = newTemporaryVariable();
        emit(tablegetelem , result , expression , expression->index , nextQuadLabel() , yylineno);

        return result;
    }
}

scopespace_t currentScopeSpace()
{
    if(scopeSpaceCounter == 1)
        return programVar;
    else if(scopeSpaceCounter % 2 == 0)
        return formalArg;
    else
        return functionLocal;
}

unsigned int currentOffset()
{
    switch(currentScopeSpace())
    {
        case programVar     :    return programVarOffset;
        case functionLocal  :    return functionLocalOffset;
        case formalArg      :    return formalArgOffset;
        default             :    assert(0);
    }
}

void increaseCurrentScopeOffset()
{
    switch(currentScopeSpace())
    {
        case programVar     :    ++programVarOffset; break;
        case functionLocal  :    ++functionLocalOffset; break;
        case formalArg      :    ++formalArgOffset; break;
        default             :    assert(0);
    }
}

void enterScopeSpace()
{
    ++scopeSpaceCounter;
}

void exitScopeSpace()
{
    assert(scopeSpaceCounter > 1);
    --scopeSpaceCounter;
}

void resetFormalArgOffset()
{
    formalArgOffset = 0;
}

void resetFunctionLocalOffset()
{
    functionLocalOffset = 0;
}

void restoreCurrentScopeOffset(unsigned int offset)
{
    switch (currentScopeSpace())
    {
        case programVar     :   programVarOffset = offset; break;
        case functionLocal  :   functionLocalOffset = offset; break;
        case formalArg      :   formalArgOffset = offset; break;
        default             :   assert(0);
    }
}

void setSpaceAndOffset(SymbolTableEntry* sym)
{
    sym->space  = currentScopeSpace();
    sym->offset = currentOffset();
}

void setSymScope(SymbolTableEntry* sym)
{
    assert(sym != nullptr);

    switch(sym->type)
    {
        case LIB_FUNCTION   : sym->sym_scope = libraryfunc_s; break;
        case USER_FUNCTION  : sym->sym_scope = programfunc_s; break;
        default             : sym->sym_scope = var_s;         break;
    }
}

expr* makeCall(expr* lvalue , expr* elist)
{
    std::vector<expr*> reversed = elist->exprlist;

    expr* func   = emitIfTableItem(lvalue);
    expr* result = nullptr;

    for(auto x = reversed.crbegin() ; x != reversed.crend() ; x++)
        emit(param , *x , nullptr , nullptr , 0 , yylineno);

    emit(call , func , nullptr , nullptr , 0 , yylineno);

    result = new expr(var_e);
    result->symbol = newTemporaryVariable();

    emit(getretval , result , nullptr , nullptr , 0 , yylineno);

    return result;
}

expr* makeNewTable(expr* list , bool indexed)
{
    expr* table   = new expr(newtable_e);
    table->symbol = newTemporaryVariable();

    emit(tablecreate , table , nullptr , nullptr , 0 , yylineno);

    if(indexed)
    {
        int index                       = 0;
        expr* index_elem                = nullptr;
        SymbolTableEntry* index_entry   = nullptr;

        for(auto x : list->exprlist)
        {
			index_elem = new expr(index++);
            emit(tablesetelem , table , index_elem , x , 0 , yylineno);
        }
    }else
        for(auto x : list->exprlist)
            emit(tablesetelem , table , x , x->index , 0 , yylineno);

    return table;
}

unsigned int nextQuadLabel()
{
    return currentQuad;
}

expr* memberItem(expr** lvalue , char* id)
{
    *lvalue         = emitIfTableItem(*lvalue);

    expr* item      = new expr(tableitem_e);
    item->symbol    = (*lvalue)->symbol;
    item->index     = new expr(conststring_e , std::string(id));

    return item;
}

void patchLabel(unsigned int quadNumber , unsigned int label)
{
    //shoulnt have to patch the current quad ever
    assert(quadNumber < currentQuad);
    quads[quadNumber].label = label;
}

char* newTemporaryVariableName()
{
    return strdup(const_cast<char *>(std::string("_t" + std::to_string(variable_temp_counter++)).c_str()));
}

void resetTemporaryVariable()
{
    variable_temp_counter = 0;
}

SymbolTableEntry* newTemporaryVariable()
{
    SymbolTableEntry* sym  = nullptr;
    std::string       name = std::string(newTemporaryVariableName());

    if((sym = symbolTable->lookup_scope(name , scope)) != nullptr)
        return sym;
    else
    {
        sym = new SymbolTableEntry(name , LOCAL_VAR , scope , yylineno , 1);
        setSpaceAndOffset(sym);
        setSymScope(sym);
        increaseCurrentScopeOffset();

		    symbolTable->insert(sym);

        return sym;
    }
}

bool isTemporaryVariable(std::string variable_name)
{
    return (variable_name[0] == '_');
}

bool isTemporaryExpression(expr* expression)
{
    return ( expression->symbol            != nullptr &&
             expression->symbol->sym_scope == var_s   &&
             isTemporaryVariable(expression->symbol->name) );
}

bool uminusMissuse(expr* expression)
{
    if(expression->type == constbool_e   ||
       expression->type == conststring_e ||
       expression->type == nil_e         ||
       expression->type == newtable_e    ||
       expression->type == programfunc_e ||
       expression->type == libraryfunc_e ||
       expression->type == bool_e         )
    {
        ERROR("Illegal expr to unary -" , yylineno);
        exit(0);
        return true;
    }
    return false;
}

bool arithmeticMissuse(std::string op , expr* expr1 , expr* expr2)
{
    /*
    if((expr1->type != constnum_e) || (expr2->type != constnum_e))
    {
        ERROR("Illegal expr to " +  op + " operator" , yylineno);
        return true;
    }
    */
    return false;
}

void printQuads()
{
  int quad_line = 1;
  for(auto quad : quads)
  {
    quads_file << std::to_string(quad_line) << ":\t";
    switch (quad.op)
    {
        case assign_op:
            quads_file  << "ASSIGN" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case tablegetelem:
            quads_file  << "TABLEGETELEM" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case tablesetelem:
            quads_file  << "TABLESETELEM" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case tablecreate:
            quads_file  << "TABLECREATE" << " "
                        << (quad.result)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case add_op:
            quads_file  << "ADD" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case sub_op:
            quads_file  << "SUB" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case uminus_op:
            quads_file  << "UMINUS" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case mul_op:
            quads_file  << "MUL" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name <<  "  "
                        << (quad.arg2)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case div_op:
            quads_file  << "DIV" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name <<  "  "
                        << (quad.arg2)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case mod_op:
            quads_file  << "MOD" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name <<  "  "
                        << (quad.arg2)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case if_greater:
            quads_file  << "IF_GREATER" << " "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name <<  "  "
                        <<  std::to_string(quad.label + 1) << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case if_greatereq:
            quads_file  << "IF_GREATEREQ" << " "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name <<  "  "
                        <<  std::to_string(quad.label + 1) << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case if_less:
            quads_file  << "IF_LESS" << " "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name <<  "  "
                        <<  std::to_string(quad.label + 1) << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case if_lesseq:
            quads_file  << "IF_LESSEQ" << " "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name <<  "  "
                        <<  std::to_string(quad.label + 1) << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case if_eq:
            quads_file  << "IF_EQUAL" << " "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name <<  "  "
                        <<  std::to_string(quad.label + 1) << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case if_noteq:
            quads_file  << "IF_NOTEQUAL" << " "
                        << (quad.arg1)->symbol->name << "  "
                        << (quad.arg2)->symbol->name <<  "  "
                        <<  std::to_string(quad.label + 1) << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case and_op:
            quads_file  << "AND" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name <<  "  "
                        << (quad.arg2)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case or_op:
            quads_file  << "OR" << " "
                        << (quad.result)->symbol->name << "  "
                        << (quad.arg1)->symbol->name <<  "  "
                        << (quad.arg2)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case jump:
            quads_file  << "JUMP" <<  " "
                        << std::to_string(quad.label + 1) << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case funcstart:
            quads_file  << "FUNCSTART" << " "
                        << (quad.result)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case funcend:
            quads_file  << "FUNCEND" << " "
                        << (quad.result)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case call:
            quads_file  << "CALL" << " "
                        << (quad.result)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case param:
            quads_file  << "PARAM" << " "
                        << (quad.result)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case getretval:
            quads_file  << "GETRETVAL" << " "
                        << (quad.result)->symbol->name << "  "
                        << "[ line: " << quad.line << " ]" << std::endl;
            break;
        case ret:
            quads_file  << "RETURN";
            if(quad.result != nullptr)
            {
                quads_file << " " << (quad.result)->symbol->name;
            }
            quads_file  << "  [ line: " << quad.line << " ]" << std::endl;
            break;
    }
    quad_line++;
  }
  quads_file.close();
}

void handleTriada(expr **exprtmp)
{
    //to logiko einai na mhn einai empty pote
    if(!((*exprtmp)->truelist.empty()))
    {
        for(auto x:(*exprtmp)->truelist)
        {
            patchLabel(x , nextQuadLabel());
        }
    }

    //to logiko einai na mhn einai empty pote
    if(!((*exprtmp)->falselist.empty()))
    {
        for(auto y:(*exprtmp)->falselist)
        {
            patchLabel(y , nextQuadLabel() + 2);
        }

    }//telos backpatch

    *exprtmp           = new expr(bool_e);
    (*exprtmp)->symbol = newTemporaryVariable();

    emit(assign_op , *exprtmp , new expr(constbool_e,true) , nullptr , nextQuadLabel() , yylineno);
    emit(jump , nullptr , nullptr , nullptr , nextQuadLabel() + 2 , yylineno);
    emit(assign_op , *exprtmp , new expr(constbool_e,false), nullptr , nextQuadLabel() , yylineno);
}
