#ifndef QUAD_H
#define QUAD_H

#include <stack>
#include <assert.h>
#include <utility>
#include <string.h>

#include "SymbolTable.h"

#define ERROR(message , line) std::cout << "Syntax Error : " << message << " in line : " << line << "\n"

extern FILE* yyin;
extern FILE* yyout;

extern int yylineno;
extern char* yytext;

extern int yylex(void);

typedef enum iopcode
{
    assign_op,          add_op,           sub_op,
    mul_op,             div_op,           mod_op,
    if_eq,              if_noteq,         if_lesseq,
    if_greatereq,       if_less,          if_greater,
    call,               param,            ret,
    getretval,          funcstart,        funcend,
    tablecreate,        tablegetelem,     tablesetelem,
    jump,               uminus_op,        and_op,
    or_op,              not_op
}iopcode_t;

typedef enum expr_type
{
    var_e,
    tableitem_e,

    elist_e,
    indexed_e,

    programfunc_e,
    libraryfunc_e,

    methcall_e,
    normcall_e,

    arithmetic_e,
    bool_e,
    assign_e,
    newtable_e,

    constint_e,
    constreal_e,
    constbool_e,
    conststring_e,

    nil_e,
	  undefined_e
}expr_t;

//ta call 8a exoun plhroforia sto address kai libanem an xreaistei
class expr
{
    public:
        expr_t                  type;
        SymbolTableEntry*       symbol;
        unsigned int            address;
        expr*                   index;
        double                  numberConst;
        int                     intnumberConst;
        std::string             stringConst;
        bool                    boolConst;
        expr*                   next;
        std::vector<expr*>	    exprlist;
        std::list<unsigned int> truelist;
        std::list<unsigned int> falselist;

    expr() {};

    expr(SymbolTableEntry* symbol);

    expr(expr_t type);

    expr(int intnumberConst);

    expr(expr_t type , std::string stringConst);

    expr(expr_t type , double numberConst);

    expr(expr_t type , bool boolConst);
};

class quad
{
    public:
        iopcode_t       op;
        expr*           result;
        expr*           arg1;
        expr*           arg2;
        unsigned int    label;
        unsigned int    line;
        unsigned int    taddress;

    quad(iopcode_t op,
         expr* result,
         expr* arg1,
         expr* arg2,
         unsigned int label,
         unsigned int line);
};

class forloop
{
    public:
        unsigned int test;
        unsigned int enter;

    forloop(unsigned int test , unsigned int enter) : test(test) , enter(enter) {};
};

//Array with the emit quads
extern std::vector<quad> quads;

int yyerror(char const* yaccProvidedMessage);

void functionMissuse(SymbolTableEntry* lvalue ,  std::string message);

bool searchForSameArguments(std::string new_argument);

void addFunctionArgument(char * name);

SymbolTableEntry* handleFunctionPrefix(char* id);

void handleFunctionId(char* id);

void handleFunctionInsert();

void handleIdVariable(SymbolTableEntry** lvalue , char* id);

void handleLocalVariable(SymbolTableEntry** lvalue , char* id);

void handleGlobalVariable(SymbolTableEntry** lvalue , char* id);

char* newTemporaryFunctionName();

void emitArithmeticOperation(iopcode_t  op ,
                             expr** result ,
                             expr*  arg1   ,
                             expr* arg2);

void emitReOperation(iopcode_t  op ,
                       expr** result ,
                       expr*  arg1   ,
                       expr* arg2);

void emitBoolOperation(iopcode_t  op ,
                       expr** result ,
                       expr*  arg1   ,
                       expr* arg2);

//Emit a new quad
void emit(iopcode_t op ,
          expr* result ,
          expr* arg1   ,
          expr* arg2   ,
          unsigned int label ,
          unsigned int line);

expr* emitIfTableItem(expr* expression);

//Returns the current scope space
scopespace_t currentScopeSpace();

//Returns the current offeset
unsigned int currentOffset();

//Increase the current offeset by 1
void increaseCurrentScopeOffset();

//Increase the scope space by 1
void enterScopeSpace();

//Reduce the scope space by 1
void exitScopeSpace();

//Resets the formal arguments offeset to 0
void resetFormalArgOffset();

//Resets the function locals offeset to 0
void resetFunctionLocalOffset();

//Restores the current scope spase to offset
void restoreCurrentScopeOffset(unsigned int offset);

//set the katallhlo scope type metablhths
void setSpaceAndOffset(SymbolTableEntry* sym);

//
void setSymScope(SymbolTableEntry* sym);

//
expr* makeCall(expr* lvalue , expr* elist);

//
expr* makeNewTable(expr* list , bool indexed);

//Returns the current quad
unsigned int nextQuadLabel();

//
expr* memberItem(expr** lvalue , char* id);

//
void patchLabel(unsigned int quadNumber , unsigned int label);

//Returns a new temporary variable name
char* newTemporaryVariableName();

//Reset the global counter for the temporary variables
void resetTemporaryVariable();

//Returns a new temporary variable
SymbolTableEntry* newTemporaryVariable();

//Returns true if the variable name begins with '_'
bool isTemporaryVariable(std::string variable_name);

bool isTemporaryExpression(expr* expression);

//Checks for wrong use of uminus e.g -true;
bool uminusMissuse(expr* expression);

//Checks for wrong use of the arithetic operators e.g "haha" + 3;
bool arithmeticMissuse(std::string op , expr* expr1 , expr* expr2);

//Prints the quads
void printQuads();

//ftiaxnei th triada gia apotimhsh expr
void handleTriada(expr **exprtmp);

#endif
