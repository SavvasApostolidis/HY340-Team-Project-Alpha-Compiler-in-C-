#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "Quad.h"

typedef enum vmopcode
{
    assign_v,           add_v,          sub_v,
    mul_v,              div_v,          mod_v,
    jeq_v,              jne_v,          jle_v,
    jge_v,              jlt_v,          jgt_v,
    call_v,             pusharg_v,      funcenter_v,
    funcexit_v,         newtable_v,     tablegetelem_v,
    tablesetelem_v,     jump_v,         nop_v
}vmopcode_t;

typedef enum vmargs
{
    label_a    ,
    global_a   ,
    formal_a   ,
    local_a    ,
    integer_a  ,
    real_a     ,
    string_a   ,
    bool_a     ,
    nil_a      ,
    userfunc_a ,
    libfunc_a  ,
    retval_a
}vmarg_t;

class vmarg
{
    public:
        vmarg_t         type;
        unsigned int    val;

        vmarg(){};

        vmarg(vmarg_t type , unsigned val)
        {
            this->type = type;
            this->val  = val;
        };
};

class instruction
{
    public:
        vmopcode_t      opcode;
        vmarg           *result = nullptr;
        vmarg           *arg1   = nullptr;
        vmarg           *arg2   = nullptr;
        unsigned int    line;

        instruction(){};

        instruction(vmopcode_t    opcode,
                    vmarg         *result,
                    vmarg         *arg1,
                    vmarg         *arg2,
                    unsigned int  line)
        {
            this->opcode = opcode;
            this->result = result;
            this->arg1   = arg1;
            this->arg2   = arg2;
            this->line   = line;
        };
};

class userfunc
{
    public:
        std::string     id;
        unsigned int    address;
        unsigned int    total_local;
        unsigned int    total_arguments;

        userfunc(std::string  id,
                 unsigned int address,
                 unsigned int total_local,
                 unsigned int total_arguments)
        {
            this->id                  = id;
            this->address             = address;
            this->total_local         = total_local;
            this->total_arguments     = total_arguments;
        };
};

class incompletejump
{
    public:
        unsigned int instruction_number;
        unsigned int instruction_address;

    incompletejump(unsigned int instruction_number,
                   unsigned int instruction_address)
    {
        this->instruction_number  = instruction_number;
        this->instruction_address = instruction_address;
    };
};

typedef void (*generator_func_t)(quad*);

unsigned int nextInstructionLabel();

void emitInstruction(instruction *instruction);

void insertIntegerArray(vmarg* arg , int number);

void insertRealArray(vmarg* arg , double number);

void insertStringArray(vmarg* arg , std::string string);

void insertLibFunctionArray(vmarg* arg , std::string lib_function);

int findUserFunction(std::string user_func);

void makeOperand (expr* expression , vmarg* arg);

void generate (vmopcode_t vmop, quad* current_proc_quad);

void generateArithetic (vmopcode_t vmop , quad* current_proc_quad);

void generateRelational (vmopcode_t vmop, quad* current_proc_quad);

void patchIncompeteJumps();

void generate_ASSIGN        ( quad* );

void generate_ADD           ( quad* );
void generate_SUB           ( quad* );
void generate_MUL           ( quad* );
void generate_DIV           ( quad* );
void generate_MOD           ( quad* );
void generate_UMINUS        ( quad* );

void generate_IF_EQ         ( quad* );
void generate_IF_NOTEQ      ( quad* );
void generate_IF_LESSEQ     ( quad* );
void generate_IF_GREATEREQ  ( quad* );
void generate_IF_LESS       ( quad* );
void generate_IF_GREATER    ( quad* );

void generate_CALL          ( quad* );
void generate_PARAM         ( quad* );
void generate_RETURN        ( quad* );
void generate_GETRETVAL     ( quad* );
void generate_FUNCSTART     ( quad* );
void generate_FUNCEND       ( quad* );

void generate_TABLECREATE   ( quad* );
void generate_TABLEGETELEM  ( quad* );
void generate_TABLESETELEM  ( quad* );

void generate_JUMP          ( quad* );

void generate_NOP           ();

void printArrays();

void printTypes(vmarg* argument);

void printInstructionsToText ();

void printInstructionsToBinary ();

void generateInstructions ();

#endif
