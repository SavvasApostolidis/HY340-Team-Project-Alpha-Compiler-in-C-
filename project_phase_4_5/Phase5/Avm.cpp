#include "Assign.h"
#include "Arithmetic.h"
#include "Functions.h"
#include "Relational.h"
#include "Table.h"

#include "help.h"

avm_memcell _stack[AVM_STACKSIZE];

//registers
avm_memcell _ax;
avm_memcell _bx;
avm_memcell _cx;

//return val register
avm_memcell _retval;

//stack top and stack frame top "pointers"
int _top;
unsigned int _topsp;

unsigned int _pc;
unsigned int _current_line;
unsigned int _code_size;

unsigned int _number_of_programvars;

unsigned int _total_actuals = 0;

bool _execution_is_finished;

FILE* _target_code_file;

std::vector<std::string> _string_values_array;
std::vector<int>         _integer_values_array;
std::vector<double>      _real_values_array;
std::vector<userfunc>    _userfuncs_array;
std::vector<std::string> _libfuncs_array;

std::vector<instruction> _instructions;

std::map<std::string , library_func_t> _library_functions;

std::string _types_to_string[]
{
  "number"            ,
  "string"            ,
  "boolean"           ,
  "table"             ,
  "userfunction"      ,
  "libraryfunction"   ,
  "nil"               ,
  "undefined"
};

memclear_func_t _memcell_clear_funcs[] =
{
  0,
  memcell_clear_string,
  0,
  memcell_clear_table,
  0,
  memcell_clear_libfunc,
  0,
  0
};

tobool_func_t _memcell_tobool_funcs[] =
{
  number_tobool    ,
  string_tobool    ,
  bool_tobool      ,
  table_tobool     ,
  userfunc_tobool  ,
  libfunc_tobool   ,
  nil_tobool       ,
  undef_tobool
};

tostring_func_t _memcell_tostring_funcs[] =
{
  number_tostring     ,
  string_tostring     ,
  bool_tostring       ,
  table_tostring      ,
  userfunc_tostring   ,
  libfunc_tostring    ,
  nil_tostring        ,
  undef_tostring
};

execute_func_t _execute_funcs[] =
{
  execute_assign        ,
  execute_add           ,
  execute_sub           ,
  execute_mul           ,
  execute_div           ,
  execute_mod           ,
  execute_jeq           ,
  execute_jne           ,
  execute_jle           ,
  execute_jge           ,
  execute_jlt           ,
  execute_jgt           ,
  execute_call          ,
  execute_pusharg       ,
  execute_funcenter     ,
  execute_funcexit      ,
  execute_newtable      ,
  execute_tablegetelem  ,
  execute_tablesetelem  ,
  execute_jump          ,
  execute_nop
};

avm_memcell* avm_translate_operand(vmarg* arg , avm_memcell* reg)
{
  switch (arg->type)
  {
    case global_a   :

      return &_stack[AVM_STACKSIZE - 1 - arg->val];
    case formal_a   :

      return &_stack[_topsp + AVM_STACKENV_SIZE + 1 + arg->val];
    case local_a    :

      return &_stack[_topsp - arg->val];
    case integer_a  :
      reg->type             = number_avm;
      reg->data.realVal     = ((double)_integer_values_array[arg->val]);

      return reg;
    case real_a     :
      reg->type             = number_avm;
      reg->data.realVal     = _real_values_array[arg->val];

      return reg;
    case string_a   :
      reg->type             = string_avm;
      reg->data.stringVal   = strdup(const_cast<char*>(std::string(_string_values_array[arg->val]).c_str()));

      return reg;
    case bool_a     :
      reg->type             = bool_avm;
      reg->data.boolVal     = ((arg->val) ? (true) : (false));

      return reg;
    case nil_a      :
      reg->type             = nil_avm;

      return reg;
    case userfunc_a :
      reg->type             = userfunc_avm;
      reg->data.userFuncVal = arg->val;

      return reg;
    case libfunc_a  :
      reg->type             = libfunc_avm;
      reg->data.libFuncVal  = strdup(const_cast<char*>(std::string(_libfuncs_array[arg->val]).c_str()));

      return reg;
    case retval_a   :

      return &_retval;
    default         :
      assert(0);
  }
}

 void avm_initstack()
{
  for(int cell = 0; cell < AVM_STACKSIZE; cell++)
  {
    AVM_WIPEOUT(_stack[cell]);
    _stack[cell].type = undef_avm;
  }
}

void avm_initialize()
{
  avm_initstack();

  _code_size             = _instructions.size();
  _current_line          = 0;

  _top                   = AVM_STACKSIZE - _number_of_programvars - 1;
  _topsp                 = AVM_STACKSIZE - _number_of_programvars - 1;

  _execution_is_finished = false;

  _library_functions.insert(std::pair<std::string , library_func_t> ("print"              , library_function_print));
  _library_functions.insert(std::pair<std::string , library_func_t> ("input"              , library_function_input));
  _library_functions.insert(std::pair<std::string , library_func_t> ("typeof"             , library_function_typeof));
  _library_functions.insert(std::pair<std::string , library_func_t> ("totalarguments"     , library_function_totalarguments));
  _library_functions.insert(std::pair<std::string , library_func_t> ("argument"           , library_function_agrument));
  _library_functions.insert(std::pair<std::string , library_func_t> ("objecttotalmembers" , library_function_objecttotalmembers));
  _library_functions.insert(std::pair<std::string , library_func_t> ("objectmemberkeys"   , library_function_objectmemberkeys));
  _library_functions.insert(std::pair<std::string , library_func_t> ("objectcopy"         , library_function_objectcopy));
  _library_functions.insert(std::pair<std::string , library_func_t> ("strtonum"           , library_function_strtonum));
  _library_functions.insert(std::pair<std::string , library_func_t> ("sqrt"               , library_function_sqrt));
  _library_functions.insert(std::pair<std::string , library_func_t> ("sin"                , library_function_sin));
  _library_functions.insert(std::pair<std::string , library_func_t> ("cos"                , library_function_cos));
}

void avm_error(std::string message)
{
  std::cout << "[ Runtime Error ] [ Line : " <<  _current_line  <<  " ] : " << message << "\n";
  std::cout << "Terminating Programm\n";

  _execution_is_finished = true;

  exit(0);
}

void avm_warning(std::string message)
{
  std::cout << "[ Runtime Warning ] [ Line : " <<  _current_line  <<  " ] : " << message << "\n";
}

bool avm_tobool(avm_memcell* memcell_to_bool)
{
  assert( memcell_to_bool->type >= number_avm && memcell_to_bool->type < undef_avm );

  return (*_memcell_tobool_funcs[memcell_to_bool->type])(memcell_to_bool);
}

std::string avm_tostring(avm_memcell* memcell_to_string)
{
  assert( memcell_to_string->type >= 0 && memcell_to_string->type <= undef_avm );

  return (*_memcell_tostring_funcs[memcell_to_string->type])(memcell_to_string);
}

void avm_memcellclear(avm_memcell* memcell_to_clear)
{
  if(memcell_to_clear->type != undef_avm)
  {
    memclear_func_t function = _memcell_clear_funcs[memcell_to_clear->type];

    if(function)
      (*function)(memcell_to_clear);

    memcell_to_clear->type = undef_avm;
  }
}

void avm_dec_top()
{
  if(_top == 0)
      avm_error("Stack Overflow");
  else
    --_top;

}

void avm_push_environment_value(unsigned int value)
{
  _stack[_top].type             = number_avm;
  _stack[_top].data.integerVal  = value;
  
  avm_dec_top();
}

unsigned int avm_get_environment_value(unsigned int value)
{
  assert( _stack[value].type == number_avm );

  unsigned int val = (unsigned) _stack[value].data.integerVal;

  assert( ((int)val) == _stack[value].data.integerVal );

  return val;
}

void avm_save_enviroment()
{
  avm_push_environment_value(_total_actuals);
  avm_push_environment_value(_pc + 1);
  avm_push_environment_value(_top + _total_actuals + 2);
  avm_push_environment_value(_topsp);
}

unsigned int avm_get_total_actuals(void)
{
  return avm_get_environment_value(_topsp + AVM_NUMACTUALS_OFFSET);
}

avm_memcell* avm_get_actual(unsigned int index)
{
  assert(index < avm_get_total_actuals());

  return &_stack[_topsp + AVM_STACKENV_SIZE + 1 + index];
}

library_func_t avm_get_library_function(char* function)
{
  std::map<std::string , library_func_t>::iterator find;

  find = _library_functions.find(std::string(function));

  if(find != _library_functions.end())
    return find->second;
  else
    return nullptr;
}

void avm_call_library_function(char* library_func_to_call)
{
  library_func_t library_function = avm_get_library_function(library_func_to_call);

  if(library_function == nullptr)
      avm_error("Unsupported library function [" + std::string(library_func_to_call) + "] called.");
  else
  {
      _topsp          = _top;
      _total_actuals  = 0;

      (*library_function)();

      if(!_execution_is_finished)
        execute_funcexit(nullptr);
  }
}

avm_memcell* avm_table_getelement(avm_table* table , avm_memcell* index)
{
  
  if(index->type != string_avm && index->type != number_avm)
  {
    avm_error("trying to access a table item with index type " + _types_to_string[index->type] + " (which is not supported).");
  }
  else
  {
    if(index->type == number_avm)
    {
      auto find = table->integerIndexed.find(index->data.realVal);

      if(find != table->integerIndexed.end())
        return find->second;
      else
        return new avm_memcell(nil_avm);
    }
    else if(index->type == string_avm)
    {
      std::string index_name(index->data.stringVal);

      auto find = table->stringIndexed.find(index_name);
      
      if(find != table->stringIndexed.end())
        return find->second;
      else
        return new avm_memcell(nil_avm);;
    }
  }
}

void avm_table_setelement(avm_table* table , avm_memcell* index , avm_memcell* value)
{
  assert(value->type <= undef_avm && value->type >= number_avm);

  if(index->type != string_avm && index->type != number_avm)
    avm_error("trying to set a table item with index type " + _types_to_string[index->type] + " (which is not supported).");
  else
  {
    if(index->type == number_avm)
    {
      auto find = table->integerIndexed.find(index->data.realVal);

      if(find != table->integerIndexed.end())
      {
        avm_memcellclear(find->second);

        if(value->type == nil_avm)
        {
          table->integerIndexed.erase(index->data.realVal);

          avm_memcellclear(index);
        }else
          avm_assign(find->second , value);
      }else
      {
        if(value->type != nil_avm)
        {
          avm_memcell* new_value = new avm_memcell();

          avm_assign(new_value , value);

          table->integerIndexed.insert(std::pair<double , avm_memcell* >(  index->data.realVal , new_value));
        }
      }
    }
    else if(index->type == string_avm)
    {
      auto find = table->stringIndexed.find(std::string(index->data.stringVal));

      if(find != table->stringIndexed.end())
      {
        avm_memcellclear(find->second);

        if(value->type == nil_avm)
        {
          table->stringIndexed.erase(std::string(index->data.stringVal));

          avm_memcellclear(index);
        }else
          avm_assign(find->second , value);
      }else
      {
        if(value->type != nil_avm)
        {
          avm_memcell* new_value = new avm_memcell();

          avm_assign(new_value , value);

          table->stringIndexed.insert(std::pair<std::string , avm_memcell* >( std::string(index->data.stringVal) , new_value));
        }
      }
    }
  }
}

void avm_destroy_table(avm_table* table_to_destroy)
{
    
    for(auto int_indexed : table_to_destroy->integerIndexed)
    {
      avm_memcellclear(int_indexed.second);
      delete int_indexed.second;
    }

    for(auto string_indexed : table_to_destroy->stringIndexed)
    {
      avm_memcellclear(string_indexed.second);
      delete string_indexed.second;
    }

    delete table_to_destroy;
}

void avm_increase_reference_counter(avm_table* table)
{
    table->reference_counter++;
}

void avm_decrease_reference_counter(avm_table* table)
{
  assert(table->reference_counter > 0);

  if(!--table->reference_counter)
    avm_destroy_table(table);
}

void avm_read_code()
{
  unsigned int magicNumber        = 0;
  unsigned int numberOfIntegers   = 0;
  unsigned int numberOfReals      = 0;
  unsigned int numberOfStrings    = 0;
  unsigned int numberOfUserFunc   = 0;
  unsigned int numberOfLibFunc    = 0;
  unsigned int instructionNumber  = 0;

  fread(&magicNumber , sizeof(unsigned int) , 1 , _target_code_file);

  if(magicNumber != 340200501)
  {
    std::cout << "Wrong magic number\n";
    exit(0);
    return;
  }

  fread(&_number_of_programvars , sizeof(unsigned int) , 1 , _target_code_file);

  fread(&numberOfStrings , sizeof(unsigned int) , 1 , _target_code_file);

  if(numberOfStrings != 0)
  {
    for(int string = 0; string < numberOfStrings; string++)
    {
      char*         string_val;
      unsigned int  lenght;

      fread(&lenght , sizeof(unsigned int) , 1 , _target_code_file);

      string_val          = (char*)malloc(sizeof(char) * (lenght + 1));
      fread(string_val , sizeof(char) * lenght , 1 , _target_code_file);
      string_val[lenght]  = '\0';

      _string_values_array.push_back(std::string(string_val));

      free(string_val);
    }
  }

  fread(&numberOfIntegers , sizeof(unsigned int) , 1 , _target_code_file);

  if(numberOfIntegers != 0)
  {
    for(int integer = 0; integer < numberOfIntegers; integer++)
    {
      int integer_val;

      fread(&integer_val , sizeof(int) , 1 , _target_code_file);

      _integer_values_array.push_back(integer_val);
    }
  }

  fread(&numberOfReals , sizeof(unsigned int) , 1 , _target_code_file);

  if(numberOfReals != 0)
  {
    for(int real = 0; real < numberOfReals; real++)
    {
      double real_val;

      fread(&real_val , sizeof(double) , 1 , _target_code_file);

      _real_values_array.push_back(real_val);
    }
  }

  fread(&numberOfUserFunc , sizeof(unsigned int) , 1 , _target_code_file);

  if(numberOfUserFunc != 0)
  {
    for(int userfuncs = 0; userfuncs < numberOfUserFunc; userfuncs++)
    {
      char*         id;
      unsigned int  lenght;
      unsigned int  address;
      unsigned int  total_local;
      unsigned int  total_arguments;

      fread(&address , sizeof(unsigned int) , 1 , _target_code_file);
      fread(&total_local , sizeof(unsigned int) , 1 , _target_code_file);
      fread(&total_arguments , sizeof(unsigned int) , 1 , _target_code_file);

      fread(&lenght , sizeof(unsigned int) , 1 , _target_code_file);

      id = (char*)malloc(sizeof(char) * (lenght + 1));
      fread(id , sizeof(char) * lenght , 1 , _target_code_file);
      id[lenght] = '\0';

      _userfuncs_array.push_back(userfunc(std::string(id) , address , total_local , total_arguments));
    }
  }

  fread(&numberOfLibFunc , sizeof(unsigned int) , 1 , _target_code_file);

  if(numberOfLibFunc != 0)
  {
    for(int libfunc = 0; libfunc < numberOfLibFunc; libfunc++)
    {
      char*         id;
      unsigned int  lenght;

      fread(&lenght , sizeof(unsigned int) , 1 , _target_code_file);

      id = (char*)malloc(sizeof(char) * (lenght + 1));
      fread(id , sizeof(char) * lenght , 1 , _target_code_file);
      id[lenght] = '\0';

      _libfuncs_array.push_back(std::string(id));
    }
  }

  fread(&instructionNumber , sizeof(unsigned int) , 1 , _target_code_file);

  if(instructionNumber != 0 )
  {
    for(int instru = 0; instru < instructionNumber; instru++)
    {
      int           read_type;
      vmopcode_t    opcode;
      vmarg_t       type;
      unsigned int  val;
      unsigned int  line;

      instruction   instruction;

      fread(&read_type , sizeof(int) , 1 , _target_code_file);

      if(read_type == 1)
      {
        fread(&opcode , sizeof(vmopcode_t) , 1 , _target_code_file);
        instruction.opcode = opcode;

        fread(&type , sizeof(vmarg_t) , 1 ,_target_code_file);
        fread(&val , sizeof(unsigned int) , 1 ,_target_code_file);
        instruction.result = new vmarg(type , val);

        fread(&line , sizeof(unsigned int) , 1 , _target_code_file);
        instruction.line = line;

      }
      else if(read_type == 2)
      {
        fread(&opcode , sizeof(vmopcode_t) , 1 , _target_code_file);
        instruction.opcode = opcode;

        fread(&type , sizeof(vmarg_t) , 1 ,_target_code_file);
        fread(&val , sizeof(unsigned int) , 1 ,_target_code_file);
        instruction.result = new vmarg(type , val);

        fread(&type , sizeof(vmarg_t) , 1 ,_target_code_file);
        fread(&val , sizeof(unsigned int) , 1 ,_target_code_file);
        instruction.arg1 = new vmarg(type , val);

        fread(&line , sizeof(unsigned int) , 1 , _target_code_file);
        instruction.line = line;

      }
      else if(read_type == 3)
      {
        fread(&opcode , sizeof(vmopcode_t) , 1 , _target_code_file);
        instruction.opcode = opcode;

        fread(&type , sizeof(vmarg_t) , 1 ,_target_code_file);
        fread(&val , sizeof(unsigned int) , 1 ,_target_code_file);
        instruction.result = new vmarg(type , val);

        fread(&type , sizeof(vmarg_t) , 1 ,_target_code_file);
        fread(&val , sizeof(unsigned int) , 1 ,_target_code_file);
        instruction.arg1 = new vmarg(type , val);

        fread(&type , sizeof(vmarg_t) , 1 ,_target_code_file);
        fread(&val , sizeof(unsigned int) , 1 ,_target_code_file);
        instruction.arg2 = new vmarg(type , val);

        fread(&line , sizeof(unsigned int) , 1 , _target_code_file);
        instruction.line = line;
      }
      else if(read_type == -3)
      {
        fread(&opcode , sizeof(vmopcode_t) , 1 , _target_code_file);
        instruction.opcode = opcode;

        fread(&type , sizeof(vmarg_t) , 1 ,_target_code_file);
        fread(&val , sizeof(unsigned int) , 1 ,_target_code_file);
        instruction.arg1 = new vmarg(type , val);

        fread(&type , sizeof(vmarg_t) , 1 ,_target_code_file);
        fread(&val , sizeof(unsigned int) , 1 ,_target_code_file);
        instruction.arg2 = new vmarg(type , val);

        fread(&type , sizeof(vmarg_t) , 1 ,_target_code_file);
        fread(&val , sizeof(unsigned int) , 1 ,_target_code_file);
        instruction.result = new vmarg(type , val);

        fread(&line , sizeof(unsigned int) , 1 , _target_code_file);
        instruction.line = line;
      }else
      {
        assert(0);
      }

      _instructions.push_back(instruction);
    }
  }

  printInstructionsToText();

  fclose(_target_code_file);

  return;
}

void execute_cycle()
{
  if(_execution_is_finished)
    return ;
  else
      if(_pc == _code_size)
      {
        _execution_is_finished = true;
        return;
      }else
      {
        assert(_pc < _code_size);

        instruction* instru = &_instructions[_pc];

        assert(instru->opcode >= 0  && instru->opcode <= AVM_MAX_INSTRUCTIONS);

        if(instru->line)
          _current_line = instru->line;

        unsigned int old_pc = _pc;

        assert(_execute_funcs[instru->opcode]);

        _execute_funcs[instru->opcode](instru);

        if(old_pc == _pc)
          ++_pc;
      }
}

int main(int argc , char**argv)
{
  if(argc != 2)
  {
    std::cout << "[Error] : missing argument [file name].\n";
    std::cout << "Exciting...\n";

    return 1;
  }else
  {
    if((_target_code_file = fopen(argv[1] , "rb+")) == NULL)
    {
      std::cout << "[Error] : file with name \"" << argv[1] << "\"" << " not found.\n";
      std::cout << "Exciting...\n";

      return 1;
    }
    avm_read_code();

    avm_initialize();

    while(!_execution_is_finished)
      execute_cycle();
  }
  return 0;
}
