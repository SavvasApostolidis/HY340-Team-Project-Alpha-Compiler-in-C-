#include "help.h"

std::ofstream instructions_text_file("help.txt");

extern std::vector<std::string> _string_values_array;
extern std::vector<int>         _integer_values_array;
extern std::vector<double>      _real_values_array;
extern std::vector<userfunc>    _userfuncs_array;
extern std::vector<std::string> _libfuncs_array;

extern std::vector<instruction> _instructions;

void printArrays()
{
    int print_number = 0;

    instructions_text_file << "\nInteger Values \n\t";
    for(auto value : _integer_values_array)
        instructions_text_file << "Number : " << print_number++ <<"\n\t"
                               << "Type   : Integer" << "\n\t"
                               << "Value  : " << value << "\n\n\t";

    print_number = 0;
    instructions_text_file << "\nReal Values \n\t";
    for(auto value : _real_values_array)
        instructions_text_file << "Number : " << print_number++ <<"\n\t"
                               << "Type   : Real" << "\n\t"
                               << "Value  : " << value << "\n\n\t";

    print_number = 0;
    instructions_text_file << "\nString Values \n\t";
    for(auto value : _string_values_array)
        instructions_text_file << "Number : " << print_number++ <<"\n\t"
                               << "Type   : String" << "\n\t"
                               << "Value  : " << value << "\n\n\t";

    print_number = 0;
    instructions_text_file << "\nUser Functions \n\t";
    for(auto function : _userfuncs_array)
    {
        instructions_text_file << "Number                    : " << print_number++ <<"\n\t"
                               << "Name                      : " << function.id << "\n\t"
                               << "Address                   : " << function.address << "\n\t"
                               << "Number of local variables : " << function.total_local << "\n\n\t";
    }

    print_number = 0;
    instructions_text_file << "\nLibrary Functions \n\t";
    for(auto function : _libfuncs_array)
    {
        instructions_text_file << "Number                  : " << print_number++ <<"\n\t" 
                               << "Name (Library Function) : " << function << "\n\n\t";
    }

    instructions_text_file << "\n\n\n";
}

void printTypes(vmarg* argument)
{
    switch(argument->type)
    {
        case label_a:
            instructions_text_file << "LABEL:[" << argument->val << "]  ";
            break;
        case global_a:
            instructions_text_file << "PROGRAMMVAR[OFFSET:" << argument->val << "]  ";
            break;
        case formal_a:
            instructions_text_file << "FORMAL[OFFSET:" << argument->val << "]  ";
            break;
        case local_a:
            instructions_text_file << "LOCAL[OFFSET:" << argument->val << "]  ";
            break;
        case integer_a:
            instructions_text_file << "INTEGER:[" << argument->val << "]  ";
            break;
        case real_a:
            instructions_text_file << "REAL:[" << argument->val << "]  ";
            break;
        case string_a:
            instructions_text_file << "STRING:[" << argument->val << "]  ";
            break;
        case bool_a:
            instructions_text_file << "BOOL:[" << argument->val << "]  ";
            break;
        case nil_a:
            instructions_text_file << "NIL  ";
            break;
        case userfunc_a:
            instructions_text_file << "USERFUNC:[" << argument->val << "]  ";
            break;
        case libfunc_a:
            instructions_text_file << "LIBFUNC:[" << argument->val << "]  ";
            break;
        case retval_a:
            instructions_text_file << "RETVAL  ";
            break;
        default:
            assert(0);
    }
}

void printInstructionsToText()
{
    unsigned int line = 0;

    printArrays();

    instructions_text_file << "Instructions\n\n";

    for(auto instru : _instructions)
    {
        instructions_text_file << std::to_string(line) << ":\t\t";
        switch(instru.opcode)
        {
            case assign_v:
                instructions_text_file << "ASSIGN  ";
                printTypes(instru.result);
                printTypes(instru.arg1);
                break;
            case add_v:
                instructions_text_file << "ADD  ";
                printTypes(instru.result);
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                break;
            case sub_v:
                instructions_text_file << "SUB  ";
                printTypes(instru.result);
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                break;
            case mul_v:
                instructions_text_file << "MUL  ";
                printTypes(instru.result);
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                break;
            case div_v:
                instructions_text_file << "DIV  ";
                printTypes(instru.result);
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                break;
            case mod_v:
                instructions_text_file << "MOD  ";
                printTypes(instru.result);
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                break;
            case newtable_v:
                instructions_text_file << "CREATETABLE  ";
                printTypes(instru.result);
                break;
            case tablegetelem_v:
                instructions_text_file << "TABLEGETELEM  ";
                printTypes(instru.result);
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                break;
            case tablesetelem_v:
                instructions_text_file << "TABLESETELEM  ";
                printTypes(instru.result);
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                break;
            case nop_v:
                instructions_text_file << "NOP  ";
                break;
            case jump_v:
                instructions_text_file << "JUMP  ";
                printTypes(instru.result);
                break;
            case jeq_v:
                instructions_text_file << "IF_EQUAL  ";
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                printTypes(instru.result);
                break;
            case jne_v:
                instructions_text_file << "IF_NOTEQUAL  ";
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                printTypes(instru.result);
                break;
            case jle_v:
                instructions_text_file << "IF_LESSEQ  ";
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                printTypes(instru.result);
                break;
            case jge_v:
                instructions_text_file << "IF_GREATEREQ  ";
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                printTypes(instru.result);
                break;
            case jlt_v:
                instructions_text_file << "IF_LESS  ";
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                printTypes(instru.result);
                break;
            case jgt_v:
                instructions_text_file << "IF_GREATER  ";
                printTypes(instru.arg1);
                printTypes(instru.arg2);
                printTypes(instru.result);
                break;
            case call_v:
                instructions_text_file << "CALL  ";
                printTypes(instru.result);
                break;
            case pusharg_v:
                instructions_text_file << "PUSHARG  ";
                printTypes(instru.result);
                break;
            case funcenter_v:
                instructions_text_file << "FUNCENTER  ";
                printTypes(instru.result);
                instructions_text_file  << "ADDRESS:[" << _userfuncs_array[instru.result->val].address << "]  "
                                        << "TOTAL LOCALS:[" << _userfuncs_array[instru.result->val].total_local << "]";
                break;
            case funcexit_v:
                instructions_text_file << "FUNCEXIT  ";
                printTypes(instru.result);
                break;
            default:
              assert(0);
        }
        instructions_text_file << "\n";
        line++;
    }
    instructions_text_file.close();
}
