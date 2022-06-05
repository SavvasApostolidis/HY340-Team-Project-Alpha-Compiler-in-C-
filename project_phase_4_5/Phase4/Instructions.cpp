#include "Instructions.h"

generator_func_t generators[] =
{
    generate_ASSIGN,        generate_ADD,               generate_SUB,
    generate_MUL,           generate_DIV,               generate_MOD,
    generate_IF_EQ,         generate_IF_NOTEQ,          generate_IF_LESSEQ,
    generate_IF_GREATEREQ,  generate_IF_LESS,           generate_IF_GREATER,
    generate_CALL,          generate_PARAM,             generate_RETURN,
    generate_GETRETVAL,     generate_FUNCSTART,         generate_FUNCEND,
    generate_TABLECREATE,   generate_TABLEGETELEM,      generate_TABLESETELEM,
    generate_JUMP,          generate_UMINUS
};

std::ofstream instructions_text_file("instructions.txt");
FILE*         instructions_binary_file;

std::vector<instruction*>       instructions;

std::vector<int>                integer_value_array;
std::vector<double>             real_value_array;
std::vector<std::string>        string_value_array;
std::vector<userfunc*>          user_functions_array;
std::vector<std::string>        library_functions_array;

std::list<incompletejump*>      incompletejumps_list;

std::stack<SymbolTableEntry*>   target_code_functions_stack;

unsigned int currentInstruction     = 0;
unsigned int currentProcessedQuad   = 0;
unsigned int function_counter       = 0;

extern unsigned int programVarOffset;


unsigned int nextInstructionLabel()
{
    return currentInstruction;
}

void emitInstruction(instruction *instruction)
{
    instructions.push_back(instruction);

    currentInstruction = instructions.size();
}

void insertIntegerArray(vmarg* arg , int number)
{
    int index   = 0;
    bool found  = false;

    arg->type = integer_a;

    if(!integer_value_array.empty())
        for(int integer = 0; integer < integer_value_array.size(); integer++)
            if(integer_value_array[integer] == number)
            {
                index = integer;
                found = true;
            }

    if(!found)
    {
        integer_value_array.push_back(number);
        arg->val  = integer_value_array.size() - 1;
    }else
        arg->val  = index;

}

void insertRealArray(vmarg* arg , double number)
{
    int index   = 0;
    bool found  = false;

    arg->type = real_a;

    if(!real_value_array.empty())
        for(int real = 0; real < real_value_array.size(); real++)
            if(real_value_array[real] == number)
            {
                index = real;
                found = true;
            }

    if(!found)
    {
        real_value_array.push_back(number);
        arg->val  = real_value_array.size() - 1;
    }else
        arg->val  = index;
}

void insertStringArray(vmarg* arg , std::string string_val)
{
    int index   = 0;
    bool found  = false;

    arg->type = string_a;

    if(!string_value_array.empty())
        for(int string = 0; string < string_value_array.size(); string++)
            if(string_value_array[string] == string_val)
            {
                index = string;
                found = true;
            }

    if(!found)
    {
        string_value_array.push_back(string_val);
        arg->val  = string_value_array.size() - 1;
    }else
        arg->val  = index;
}

void insertLibFunctionArray(vmarg* arg , std::string lib_function)
{
    int index   = 0;
    bool found  = false;

    arg->type = libfunc_a;

    if(!library_functions_array.empty())
        for(int lib_func = 0; lib_func < library_functions_array.size(); lib_func++)
            if(library_functions_array[lib_func] == lib_function)
            {
                index = lib_func;
                found = true;
            }

    if(!found)
    {
        library_functions_array.push_back(lib_function);
        arg->val  = library_functions_array.size() - 1;
    }else
        arg->val  = index;
}

int findUserFunction(SymbolTableEntry* user_func)
{
    int index = -1;

    if(!user_functions_array.empty())
        for(int func = 0; func < user_functions_array.size(); func++)
            if(user_functions_array[func]->address == user_func->taddress)
                index = func;

    return index;
}

void makeOperand(expr* expression , vmarg* arg)
{
    int index = 0;

    switch(expression->type)
    {
        case var_e:
        case tableitem_e:
        case arithmetic_e:
        case bool_e:
        case newtable_e:
        case assign_e:
            assert(expression->symbol);

            switch (expression->symbol->space)
            {
                case programVar    :    arg->type = global_a;   break;
                case functionLocal :    arg->type = local_a;    break;
                case formalArg     :    arg->type = formal_a;   break;
                default            :    assert(0);
            }
            arg->val  = expression->symbol->offset;

            break;
        case constbool_e:
            arg->type = bool_a;
            arg->val  = ((expression->boolConst) ? (1) : (0) );

            break;
        case conststring_e:
            insertStringArray(arg , expression->stringConst);

            break;
        case constint_e:
            insertIntegerArray(arg , expression->intnumberConst);

            break;
        case constreal_e:
            insertRealArray(arg , expression->numberConst);

            break;
        case nil_e:
            arg->type = nil_a;

            break;
        case programfunc_e:
            arg->type = userfunc_a;
            arg->val  = findUserFunction(expression->symbol);

            break;
        case libraryfunc_e:
            insertLibFunctionArray(arg , expression->symbol->name);

            break;
        default:
            assert(0);
    }
}

void generate(vmopcode_t vmop, quad* current_proc_quad)
{
    instruction *new_instruction = new instruction(vmop , new vmarg() , new vmarg() , new vmarg() , current_proc_quad->line);

    if(current_proc_quad->result != nullptr)
        makeOperand(current_proc_quad->result , new_instruction->result);
    if(current_proc_quad->arg1 != nullptr)
        makeOperand(current_proc_quad->arg1 , new_instruction->arg1);
    if(current_proc_quad->arg2 != nullptr)
        makeOperand(current_proc_quad->arg2 , new_instruction->arg2);

    current_proc_quad->taddress = nextInstructionLabel();

    emitInstruction(new_instruction);
}

void generateArithetic (vmopcode_t vmop, quad* current_proc_quad)
{
    generate(vmop , current_proc_quad);
}

void generateRelational(vmopcode_t vmop, quad* current_proc_quad)
{
    instruction *new_instruction = new instruction(vmop , new vmarg() , new vmarg() , new vmarg() , current_proc_quad->line);

    if(current_proc_quad->arg1 != nullptr)
        makeOperand(current_proc_quad->arg1 , new_instruction->arg1);
    if(current_proc_quad->arg2 != nullptr)
        makeOperand(current_proc_quad->arg2 , new_instruction->arg2);

    new_instruction->result->type = label_a;

    if(current_proc_quad->label <= currentProcessedQuad )
        new_instruction->result->val = quads[current_proc_quad->label].taddress;
    else
        incompletejumps_list.push_back(new incompletejump(nextInstructionLabel() , current_proc_quad->label));

    current_proc_quad->taddress = nextInstructionLabel();

    emitInstruction(new_instruction);
}

void patchIncompeteJumps()
{
  for(auto incom_instruction : incompletejumps_list)
  {
    if(incom_instruction->instruction_address == nextQuadLabel())
        instructions[incom_instruction->instruction_number]->result->val = nextInstructionLabel();
    else
        instructions[incom_instruction->instruction_number]->result->val = quads[incom_instruction->instruction_address].taddress;
  }
}

void generate_ASSIGN (quad* current_proc_quad)
{
    generate(assign_v , current_proc_quad);
}

void generate_ADD (quad* current_proc_quad)
{
    generateArithetic(add_v , current_proc_quad);
}

void generate_SUB (quad* current_proc_quad)
{
    generateArithetic(sub_v , current_proc_quad);
}

void generate_MUL (quad* current_proc_quad)
{
    generateArithetic(mul_v , current_proc_quad);
}

void generate_DIV (quad* current_proc_quad)
{
    generateArithetic(div_v , current_proc_quad);
}

void generate_MOD (quad* current_proc_quad)
{
    generateArithetic(mod_v , current_proc_quad);
}

void generate_UMINUS (quad* current_proc_quad)
{
    instruction* new_instruction = new instruction(mul_v , new vmarg() , new vmarg() , new vmarg() , current_proc_quad->line);

    //dimos
    //edw prepei na baloume sto taddress to nextInstructionLabel()
    current_proc_quad->taddress = nextInstructionLabel();

    if(current_proc_quad->result != nullptr)
        makeOperand(current_proc_quad->result , new_instruction->result);
    if(current_proc_quad->arg1   != nullptr)
        makeOperand(current_proc_quad->arg1   , new_instruction->arg1);

    insertIntegerArray(new_instruction->arg2 , -1);

    emitInstruction(new_instruction);
}

void generate_IF_EQ (quad* current_proc_quad)
{
    generateRelational(jeq_v , current_proc_quad);
}

void generate_IF_NOTEQ (quad* current_proc_quad)
{
    generateRelational(jne_v , current_proc_quad);
}

void generate_IF_LESSEQ (quad* current_proc_quad)
{
    generateRelational(jle_v , current_proc_quad);
}

void generate_IF_GREATEREQ (quad* current_proc_quad)
{
    generateRelational(jge_v , current_proc_quad);
}

void generate_IF_LESS (quad* current_proc_quad)
{
    generateRelational(jlt_v , current_proc_quad);
}

void generate_IF_GREATER (quad* current_proc_quad)
{
    generateRelational(jgt_v , current_proc_quad);
}

void generate_CALL (quad* current_proc_quad)
{
    instruction *new_instruction = nullptr;

    current_proc_quad->taddress  = nextInstructionLabel();
    new_instruction              = new instruction(call_v , new vmarg() , nullptr , nullptr , current_proc_quad->line);

    if(current_proc_quad->result != nullptr)
        makeOperand(current_proc_quad->result , new_instruction->result);

    emitInstruction(new_instruction);
}

void generate_PARAM (quad* current_proc_quad)
{
    instruction* new_instruction = nullptr;

    current_proc_quad->taddress  = nextInstructionLabel();
    new_instruction              = new instruction(pusharg_v , new vmarg() , nullptr , nullptr , current_proc_quad->line);

    if(current_proc_quad->result != nullptr)
        makeOperand(current_proc_quad->result , new_instruction->result);

    emitInstruction(new_instruction);
}

void generate_RETURN (quad* current_proc_quad)
{
    instruction* assign_instruction = new instruction(assign_v , new vmarg() , new vmarg() , nullptr , current_proc_quad->line);
    SymbolTableEntry* function      = nullptr;

    current_proc_quad->taddress     = nextInstructionLabel();

    //dimos
    //Se periptwsh pou to return den gurnaei kati (return;) tote den prepei na
    //den prepei na kaloume thn make operand dioti to result einai nulll.
    //Epishs epeidh kanoume generate ena asssing ,to opoio einai aparaithto na
    //ginei eite epistrefoume kati eite oxi dioti an den to kaname otan kanoume
    //generate getretval o return kataxwrhths(epeidh den exoume kanei assign kati
    //sto keno return) ua exei thn prohgoumenh timh opote kanoume assign sto return
    //kataxwrhth thn timh nil.
    if(current_proc_quad->result != nullptr)
    {
        makeOperand(current_proc_quad->result , assign_instruction->arg1);

        assign_instruction->result->type = retval_a;
    }else
    {
        assign_instruction->arg1->type = nil_a;

        assign_instruction->result->type = retval_a;
    }

    emitInstruction(assign_instruction);


    function = target_code_functions_stack.top();
    (function->target_code_return_list).push_back(nextInstructionLabel());
}

void generate_GETRETVAL (quad* current_proc_quad)
{
    instruction *new_instruction = nullptr;

    current_proc_quad->taddress  = nextInstructionLabel();
    new_instruction              = new instruction(assign_v , new vmarg() , new vmarg() , nullptr , current_proc_quad->line);

    if(current_proc_quad->result != nullptr)
        makeOperand(current_proc_quad->result , new_instruction->result);

    new_instruction->arg1->type = retval_a;

    emitInstruction(new_instruction);
}

void generate_FUNCSTART (quad* current_proc_quad)
{
    instruction*  new_instruction     = new instruction(funcenter_v , new vmarg() , nullptr , nullptr , current_proc_quad->line);
    SymbolTableEntry* function        = current_proc_quad->result->symbol;

    function->taddress                = nextInstructionLabel();
    current_proc_quad->taddress       = nextInstructionLabel();

    user_functions_array.push_back(new userfunc(function->name , function->taddress  , function->totalLocal , function->arguments.size()));
    function_counter++;

    function->target_code_return_list = std::list<unsigned int>();
    target_code_functions_stack.push(function);

    new_instruction->result->val      = user_functions_array.size() - 1;

    if(current_proc_quad->result != nullptr)
        makeOperand(current_proc_quad->result , new_instruction->result);

    emitInstruction(new_instruction);
}

void generate_FUNCEND (quad* current_proc_quad)
{
    instruction*      new_instruction   = new instruction(funcexit_v , new vmarg() , nullptr , nullptr , current_proc_quad->line);
    instruction*      assign_retval_nil = new instruction(assign_v , new vmarg() , new vmarg() , nullptr , current_proc_quad->line);
    SymbolTableEntry* function          = target_code_functions_stack.top();

    for(auto instruction : function->target_code_return_list)
        instructions[instruction]->result->val = nextInstructionLabel();

    current_proc_quad->taddress  = nextInstructionLabel();

    //dimos
    if(!function->function_has_return)
    {
        assign_retval_nil->arg1->type = nil_a;

        assign_retval_nil->result->type = retval_a;

        emitInstruction(assign_retval_nil);
    }

    new_instruction->result->val = --function_counter;

    if(current_proc_quad->result != nullptr)
        makeOperand(current_proc_quad->result , new_instruction->result);

    target_code_functions_stack.pop();

    emitInstruction(new_instruction);
}

void generate_TABLECREATE (quad* current_proc_quad)
{
    generate(newtable_v , current_proc_quad);
}

void generate_TABLEGETELEM (quad* current_proc_quad)
{
    generate(tablegetelem_v , current_proc_quad);
}

void generate_TABLESETELEM (quad* current_proc_quad)
{
    generate(tablesetelem_v , current_proc_quad);
}

void generate_JUMP (quad* current_proc_quad)
{
    generateRelational(jump_v , current_proc_quad);
}

void generate_NOP ()
{
    instruction* new_instruction = new instruction(nop_v , nullptr , nullptr , nullptr , 0);

    emitInstruction(new_instruction);
}

void printArrays()
{

    instructions_text_file << "\nInteger Values \n\t";
    for(auto value : integer_value_array)
        instructions_text_file << "Type  : Integer" << "\n\t"
                               << "Value : " << value << "\n\n\t";

    instructions_text_file << "\nReal Values \n\t";
    for(auto value : real_value_array)
        instructions_text_file << "Type  : Real" << "\n\t"
                               << "Value : " << value << "\n\n\t";

    instructions_text_file << "\nString Values \n\t";
    for(auto value : string_value_array)
        instructions_text_file << "Type  : String" << "\n\t"
                               << "Value : " << value << "\n\n\t";

    instructions_text_file << "\nUser Functions \n\t";

    for(auto function : user_functions_array)
    {
        instructions_text_file << "Name    : " << function->id << "\n\t"
                               << "Address : " << function->address << "\n\t"
                               << "Number of local variables: " << function->total_local << "\n\n\t";
    }

    instructions_text_file << "\nLibrary Functions \n\t";

    for(auto function : library_functions_array)
    {
        instructions_text_file << "Name (Library Function) : " << function << "\n\n\t";
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

    for(auto instru : instructions)
    {
        instructions_text_file << std::to_string(line) << ":\t\t";
        switch(instru->opcode)
        {
            case assign_v:
                instructions_text_file << "ASSIGN  ";
                printTypes(instru->result);
                printTypes(instru->arg1);
                break;
            case add_v:
                instructions_text_file << "ADD  ";
                printTypes(instru->result);
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                break;
            case sub_v:
                instructions_text_file << "SUB  ";
                printTypes(instru->result);
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                break;
            case mul_v:
                instructions_text_file << "MUL  ";
                printTypes(instru->result);
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                break;
            case div_v:
                instructions_text_file << "DIV  ";
                printTypes(instru->result);
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                break;
            case mod_v:
                instructions_text_file << "MOD  ";
                printTypes(instru->result);
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                break;
            case newtable_v:
                instructions_text_file << "CREATETABLE  ";
                printTypes(instru->result);
                break;
            case tablegetelem_v:
                instructions_text_file << "TABLEGETELEM  ";
                printTypes(instru->result);
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                break;
            case tablesetelem_v:
                instructions_text_file << "TABLESETELEM  ";
                printTypes(instru->result);
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                break;
            case nop_v:
                instructions_text_file << "NOP  ";
                break;
            case jump_v:
                instructions_text_file << "JUMP  ";
                printTypes(instru->result);
                break;
            case jeq_v:
                instructions_text_file << "IF_EQUAL  ";
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                printTypes(instru->result);
                break;
            case jne_v:
                instructions_text_file << "IF_NOTEQUAL  ";
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                printTypes(instru->result);
                break;
            case jle_v:
                instructions_text_file << "IF_LESSEQ  ";
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                printTypes(instru->result);
                break;
            case jge_v:
                instructions_text_file << "IF_GREATEREQ  ";
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                printTypes(instru->result);
                break;
            case jlt_v:
                instructions_text_file << "IF_LESS  ";
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                printTypes(instru->result);
                break;
            case jgt_v:
                instructions_text_file << "IF_GREATER  ";
                printTypes(instru->arg1);
                printTypes(instru->arg2);
                printTypes(instru->result);
                break;
            case call_v:
                instructions_text_file << "CALL  ";
                printTypes(instru->result);
                break;
            case pusharg_v:
                instructions_text_file << "PUSHARG  ";
                printTypes(instru->result);
                break;
            case funcenter_v:
                instructions_text_file << "FUNCENTER  ";
                printTypes(instru->result);
                instructions_text_file  << "ADDRESS:[" << user_functions_array[instru->result->val]->address << "]  "
                                        << "TOTAL LOCALS:[" << user_functions_array[instru->result->val]->total_local << "]";
                break;
            case funcexit_v:
                instructions_text_file << "FUNCEXIT  ";
                printTypes(instru->result);
                break;
        }
        instructions_text_file << "\n";
        line++;
    }
    instructions_text_file.close();
}

void printInstructionsToBinary()
{
    unsigned int magicNumber        = 340200501;
    unsigned int numberOfIntegers   = integer_value_array.size();
    unsigned int numberOfReals      = real_value_array.size();
    unsigned int numberOfStrings    = string_value_array.size();
    unsigned int numberOfUserFunc   = user_functions_array.size();
    unsigned int numberOfLibFunc    = library_functions_array.size();
    unsigned int instructionNumber  = instructions.size();
    unsigned int number_of_vars     = programVarOffset;

    if((instructions_binary_file = fopen("../Phase5/instructions.abc","wb+")) == NULL)
        assert(0);

    fwrite(&magicNumber , sizeof(unsigned int) , 1 ,instructions_binary_file);

    fwrite(&number_of_vars , sizeof(unsigned int) , 1 ,instructions_binary_file);

    fwrite(&numberOfStrings , sizeof(unsigned int) , 1 , instructions_binary_file);

    for(auto string : string_value_array)
    {
        char *string_value  = const_cast<char*>(string.c_str());
        unsigned int lenght = strlen(string_value);

        fwrite(&lenght, sizeof(unsigned int) , 1 , instructions_binary_file);
        fwrite(string_value , sizeof(char) * lenght , 1 , instructions_binary_file);
    }

    fwrite(&numberOfIntegers , sizeof(unsigned int) , 1 , instructions_binary_file);

    for(auto integer : integer_value_array)
        fwrite(&integer , sizeof(int) , 1 , instructions_binary_file);

    fwrite(&numberOfReals , sizeof(unsigned int) , 1 , instructions_binary_file);

    for(auto real : real_value_array)
        fwrite(&real , sizeof(double) , 1 , instructions_binary_file);

    fwrite(&numberOfUserFunc , sizeof(unsigned int) , 1 , instructions_binary_file);

    for(auto user_func : user_functions_array)
    {
      char *id            = const_cast<char*>((user_func->id).c_str());
      unsigned int lenght = (user_func->id).size();

      fwrite(&user_func->address , sizeof(unsigned int) , 1 , instructions_binary_file);
      fwrite(&user_func->total_local , sizeof(unsigned int) , 1 , instructions_binary_file);
      fwrite(&user_func->total_arguments , sizeof(unsigned int) , 1 , instructions_binary_file);
      fwrite(&lenght , sizeof(unsigned int) , 1 , instructions_binary_file);
      fwrite(id , sizeof(char) * lenght , 1 , instructions_binary_file);
    }

    fwrite(&numberOfLibFunc , sizeof(unsigned int) , 1 , instructions_binary_file);

    for(auto lib_func : library_functions_array)
    {
      char *id            = const_cast<char*>(lib_func.c_str());
      unsigned int lenght = lib_func.size();

      fwrite(&lenght , sizeof(unsigned int) , 1 , instructions_binary_file);
      fwrite(id , sizeof(char) * lenght , 1 , instructions_binary_file);
    }

    int read_type = 0;

    fflush(instructions_binary_file);
    fwrite(&instructionNumber , sizeof(unsigned int) , 1 , instructions_binary_file);

    for(auto instru : instructions)
    {
      switch(instru->opcode)
      {
          case assign_v:
              //2 type == 2 operands (assign_op)
              read_type = 2;

              fwrite(&read_type , sizeof(int) , 1 , instructions_binary_file);
              fwrite(&instru->opcode , sizeof(vmopcode_t) , 1 , instructions_binary_file);
              fwrite(&instru->result->type , sizeof(vmarg_t) , 1 ,instructions_binary_file);
              fwrite(&instru->result->val , sizeof(unsigned int) , 1 ,instructions_binary_file);
              fwrite(&instru->arg1->type , sizeof(vmarg_t) , 1 ,instructions_binary_file);
              fwrite(&instru->arg1->val , sizeof(unsigned int) , 1 ,instructions_binary_file);
              fwrite(&instru->line , sizeof(unsigned int) , 1 , instructions_binary_file);

              break;
          case add_v:
          case sub_v:
          case mul_v:
          case div_v:
          case mod_v:
          case tablegetelem_v:
          case tablesetelem_v:
              //3 type = 3 operands
              read_type = 3;

              fwrite(&read_type , sizeof(int) , 1 , instructions_binary_file);
              fwrite(&instru->opcode , sizeof(vmopcode_t) , 1 , instructions_binary_file);
              fwrite(&instru->result->type , sizeof(vmarg_t) , 1 ,instructions_binary_file);
              fwrite(&instru->result->val , sizeof(unsigned int) , 1 ,instructions_binary_file);
              fwrite(&instru->arg1->type , sizeof(vmarg_t) , 1 ,instructions_binary_file);
              fwrite(&instru->arg1->val , sizeof(unsigned int) , 1 ,instructions_binary_file);
              fwrite(&instru->arg2->type , sizeof(vmarg_t) , 1 ,instructions_binary_file);
              fwrite(&instru->arg2->val , sizeof(unsigned int) , 1 ,instructions_binary_file);
              fwrite(&instru->line , sizeof(unsigned int) , 1 , instructions_binary_file);

              break;
          case newtable_v:
          case call_v:
          case pusharg_v:
          case funcenter_v:
          case funcexit_v:
          case jump_v:
              //1 type = 1 operands
              read_type = 1;

              fwrite(&read_type , sizeof(int) , 1 , instructions_binary_file);
              fwrite(&instru->opcode , sizeof(vmopcode_t) , 1 , instructions_binary_file);
              fwrite(&instru->result->type , sizeof(vmarg_t) , 1 ,instructions_binary_file);
              fwrite(&instru->result->val , sizeof(unsigned int) , 1 ,instructions_binary_file);
              fwrite(&instru->line , sizeof(unsigned int) , 1 , instructions_binary_file);

              break;
          case jeq_v:
          case jne_v:
          case jle_v:
          case jge_v:
          case jlt_v:
          case jgt_v:
              //-3 type = 3 operands reversed(arg1,arg2,result)
              read_type = -3;

              fwrite(&read_type , sizeof(int) , 1 , instructions_binary_file);
              fwrite(&instru->opcode , sizeof(vmopcode_t) , 1 , instructions_binary_file);
              fwrite(&instru->arg1->type , sizeof(vmarg_t) , 1 ,instructions_binary_file);
              fwrite(&instru->arg1->val , sizeof(unsigned int) , 1 ,instructions_binary_file);
              fwrite(&instru->arg2->type , sizeof(vmarg_t) , 1 ,instructions_binary_file);
              fwrite(&instru->arg2->val , sizeof(unsigned int) , 1 ,instructions_binary_file);
              fwrite(&instru->result->type , sizeof(vmarg_t) , 1 ,instructions_binary_file);
              fwrite(&instru->result->val , sizeof(unsigned int) , 1 ,instructions_binary_file);
              fwrite(&instru->line , sizeof(unsigned int) , 1 , instructions_binary_file);

              break;
          case nop_v:
              break;
      }
    }
    fclose(instructions_binary_file);
}

void generateInstructions  ()
{
    for(currentProcessedQuad = 0; currentProcessedQuad < quads.size() ; currentProcessedQuad++)
        (*generators[quads[currentProcessedQuad].op])(&quads[currentProcessedQuad]);

    patchIncompeteJumps();
}
