#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <string>

//auta eginan include apla gia na gnwrizei to vm ta structures

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
            this->id              = id;
            this->address         = address;
            this->total_local     = total_local;
            this->total_arguments = total_arguments;
        };
};

#endif
