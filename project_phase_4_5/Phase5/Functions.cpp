#include "Functions.h"

extern avm_memcell _stack[AVM_STACKSIZE];

extern avm_memcell _ax;
extern avm_memcell _retval;

extern int _top;
extern unsigned int _topsp;

extern unsigned int _pc;
extern unsigned int _code_size;

extern std::vector<instruction> _instructions;

extern std::vector<userfunc>    _userfuncs_array;

extern std::string _types_to_string[];

extern unsigned int _number_of_programvars;

extern unsigned int _total_actuals;

bool userfunc_tobool(avm_memcell* memcell_to_bool)
{
  return true;
}

bool libfunc_tobool(avm_memcell* memcell_to_bool)
{
  return true;
}

std::string userfunc_tostring(avm_memcell* memcell_to_string)
{
  return std::string("User Function , Address[" +  std::to_string(_userfuncs_array[memcell_to_string->data.userFuncVal].address) + "]");
}

std::string libfunc_tostring(avm_memcell* memcell_to_string)
{
  return std::string("Library Function [ " + std::string(memcell_to_string->data.libFuncVal) + " ]");
}

void memcell_clear_libfunc(avm_memcell *lib_func)
{
  assert( lib_func->data.libFuncVal );

  free(lib_func->data.libFuncVal);
}

void copy_content(avm_memell_t type , avm_memcell* src , avm_memcell* dest)
{
  switch(type)
  {
    case number_avm:
      dest->type         = number_avm;
      dest->data.realVal = src->data.realVal;

      break; 
    case string_avm:
      dest->type           = string_avm;
      dest->data.stringVal = strdup(src->data.stringVal);

      break;
    case bool_avm:
      dest->type         = bool_avm;
      dest->data.boolVal = src->data.boolVal;

      break;
    case table_avm:
      dest->type           = table_avm;
      dest->data.tableVal  = src->data.tableVal;

      avm_increase_reference_counter(src->data.tableVal);

      break;
    case userfunc_avm:
      dest->type             = userfunc_avm;
      dest->data.userFuncVal = src->data.userFuncVal;

      break;
    case libfunc_avm:
      dest->type            = libfunc_avm;
      dest->data.libFuncVal = strdup(src->data.libFuncVal);

      break;
    case nil_avm:
      dest->type            = nil_avm;

      break;
    default:
      assert(0);
  }
}

void library_function_print()
{
  unsigned int number_of_arguments = avm_get_total_actuals();

  for(int argument = 0; argument < number_of_arguments; argument++)
  {
    std::string output = avm_tostring(avm_get_actual(argument));

    std::cout << output;
  }
}

void library_function_input()
{
  std::string input_string;

  bool is_number  = true;
  bool is_real    = false;
  bool quotes     = false;

  getline(std::cin , input_string);

  is_real = (input_string.find('.') != std::string::npos) ? true : false;
  quotes  = (input_string.find('"') != std::string::npos) ? true : false;

  for(auto character : input_string)
  {
    if(!std::isdigit((int)character) )
    {
      if(character!='.'){
        is_number = false;
        break;
      }
    }
  }

  avm_memcellclear(&_retval);

  if(is_real && is_number && !quotes)
  {
    _retval.type          = number_avm;
    _retval.data.realVal  = atof(input_string.c_str());
  }
  else if(is_number && !quotes)
  {
    _retval.type          = number_avm;
    _retval.data.realVal  = atoi(input_string.c_str());
  }
  else if(input_string == "true" || input_string == "false")
  {
    _retval.type         = bool_avm;
    _retval.data.boolVal = (input_string == "true") ? true : false;
  }
  else if(input_string == "nil")
  {
    _retval.type         = nil_avm;
  }else
  {
    _retval.type            = string_avm;
    _retval.data.stringVal  = strdup(const_cast<char*>(input_string.c_str()));
  }
}

void library_function_typeof()
{
  unsigned int argument = avm_get_total_actuals();

  if(argument != 1)
    avm_error("one argument (not " + std::to_string(argument) + ") expected in library function \"typeof\" \n");
  else
  {
    avm_memcellclear(&_retval);

    _retval.type           = string_avm;
    _retval.data.stringVal = strdup(const_cast<char*>( _types_to_string[avm_get_actual(0)->type].c_str() ) );
  }
}

void library_function_totalarguments()
{
  unsigned int p_topsp          = avm_get_environment_value(_topsp + AVM_SAVEDTOPSP_OFFSET);
  unsigned int total_arguments  = avm_get_environment_value(_topsp + AVM_NUMACTUALS_OFFSET);

  if(total_arguments != 0 )
    avm_error("zero arguments (not " + std::to_string(total_arguments) + ") expected in library function \"totalarguments\"\n");

  avm_memcellclear(&_retval);

  // to kanoume auto dioti apo tnn execute_call ua exoume eidh swsei to enviroment opote
  // an eimaste eksw apo synarthsh to topsp ua einai eidh tesseres ueseis poio katw
  // sth stoiba opote prepei na tsekaroume to apouhkeumeno topsp
  if(p_topsp ==  AVM_STACKSIZE - _number_of_programvars - 1)
  {
    avm_warning("library function with name \"totalarguments\" called outside a function.");
    
    _retval.type = nil_avm;
  }else
  {
    _retval.type         = number_avm;
    _retval.data.realVal = avm_get_environment_value(p_topsp + AVM_NUMACTUALS_OFFSET);
  }
}

void library_function_agrument()
{
  unsigned int p_topsp                  = avm_get_environment_value(_topsp + AVM_SAVEDTOPSP_OFFSET);

  avm_memcellclear(&_retval);

  if(p_topsp == AVM_STACKSIZE - _number_of_programvars - 1)
  {
    avm_warning("library function with name \"argument\" called outside a function.");

    _retval.type = nil_avm;
    return;
  }

  unsigned int number_of_args_prev_call = avm_get_environment_value(p_topsp + AVM_NUMACTUALS_OFFSET);
  unsigned int number_of_args           = avm_get_total_actuals();
  avm_memcell* argument                 = nullptr;

  if(number_of_args != 1)
    avm_error("one argument (not " + std::to_string(number_of_args) + ") expected in library function \"argument\" \n");
  else
  {
    argument = avm_get_actual(0);

    if(argument->type != number_avm)
      avm_error("parameter in library function with name \"argument\" must be number not " + _types_to_string[argument->type] + " type.\n");
    else if(((int)argument->data.realVal) > number_of_args_prev_call - 1)
      avm_error("parameter in library function with name \"argument\" is out of range.\n");
    else if(((int)argument->data.realVal) < 0 )
      avm_error("parameter in library function with name \"argument\" must be a positive number.");
    else
      avm_assign(&_retval , &_stack[p_topsp + AVM_STACKENV_SIZE + 1 + ((unsigned int)argument->data.realVal)]);
  }
}

void library_function_objectmemberkeys()
{
  avm_memcell* table    = nullptr;
  unsigned int argument = avm_get_total_actuals();

  if(argument != 1)
    avm_error("one argument (not " + std::to_string(argument) + ") expected in library function \"objectmemberkeys\" \n");

  table = avm_get_actual(0);

  if(table->type != table_avm)
    avm_error("library function with name \"objectmemberkeys\" take as argument only table.\n");

  avm_memcellclear(&_retval);

  _retval.type = table_avm;

  avm_table* keys_table = new avm_table();
  double key            = 0.0;

  //kanoume to reference_counter autou tou table mias kai twra o _retval kataxwrhths
  //deixnei se ayto , opote se periptwsh pou pame meta na kalesoume thn avm_memcellclear
  //gia to _retval (se mia allh isws synarthsh) o reference_counter ua htan 0
  //kai ua eixame problhma me to na kauarhsoume to table
  avm_increase_reference_counter(keys_table);

  for(auto integer_keys : table->data.tableVal->integerIndexed)
  {
    avm_memcell* index = new avm_memcell();

    index->type         = number_avm;
    index->data.realVal = integer_keys.first;

    keys_table->integerIndexed.insert(std::pair<double , avm_memcell*>(key , index));
    key++;
  }

  for (auto string_keys : table->data.tableVal->stringIndexed)
  {
    avm_memcell* index = new avm_memcell();

    index->type           = string_avm;
    index->data.stringVal = strdup(const_cast<char*>(string_keys.first.c_str()));

    keys_table->integerIndexed.insert(std::pair<double , avm_memcell*>(key , index));
    key++;
  }

  _retval.data.tableVal = keys_table;
}

void library_function_objecttotalmembers()
{
  avm_memcell* table    = nullptr;
  unsigned int argument = avm_get_total_actuals();

  if(argument != 1)
    avm_error("one argument (not " + std::to_string(argument) + ") expected in library function \"objecttotalmembers\" \n");

  table = avm_get_actual(0);

  if(table->type != table_avm)
    avm_error("library function with name \"objecttotalmembers\" take as argument only table.\n");

  avm_memcellclear(&_retval);

  _retval.type         = number_avm;
  _retval.data.realVal = table->data.tableVal->integerIndexed.size() + table->data.tableVal->stringIndexed.size();
}

void library_function_objectcopy()
{
  avm_memcell* table         = nullptr;
  avm_table* shallow_copy    = new avm_table(); 

  unsigned int argument      = avm_get_total_actuals();

  if(argument != 1)
    avm_error("one argument (not " + std::to_string(argument) + ") expected in library function \"objectcopy\" \n");

  table = avm_get_actual(0);

  if(table->type != table_avm)
    avm_error("library function with name \"objectcopy\" take as argument only table.\n");

  if(!table->data.tableVal->integerIndexed.empty())
  {
    for(auto index : table->data.tableVal->integerIndexed)
    {
      avm_memcell* value = new avm_memcell();

      copy_content(index.second->type , index.second , value );
      
      shallow_copy->integerIndexed.insert(std::pair<double , avm_memcell*>(index.first , value));
    }
  }

  if(!table->data.tableVal->stringIndexed.empty())
  {
    for(auto index : table->data.tableVal->stringIndexed)
    {
      avm_memcell* value = new avm_memcell();

      copy_content(index.second->type , index.second , value );

      shallow_copy->stringIndexed.insert(std::pair<std::string , avm_memcell*>(std::string(index.first) , value));
    }
  }

  avm_memcellclear(&_retval);

  _retval.type          = table_avm;
  _retval.data.tableVal = shallow_copy;   
}

void library_function_strtonum()
{
  avm_memcell* string    = nullptr;
  unsigned int argument  = avm_get_total_actuals();

  if(argument != 1)
    avm_error("one argument (not " + std::to_string(argument) + ") expected in library function \"strtonum\" \n");

  string = avm_get_actual(0);

  if(string->type != string_avm)
    avm_error("library function with name \"strtonum\" take as argument only string.\n");

  std::string input(string->data.stringVal);

  bool has_digits = (std::find_if(input.begin(), input.end(), ::isdigit) != input.end());
  bool has_alpha  = (std::find_if(input.begin(), input.end(), ::isalpha) != input.end());

  if(has_digits && has_alpha)
  {
    _retval.type = nil_avm;
     return;
  }

  if(has_alpha)
  {
    _retval.type = nil_avm;
     return;
  }

  avm_memcellclear(&_retval);

  _retval.type          = number_avm;
  _retval.data.realVal  = ((double)atof(string->data.stringVal));
}

void library_function_sqrt()
{
  avm_memcell* number    = nullptr;
  unsigned int argument  = avm_get_total_actuals();

  if(argument != 1)
    avm_error("one argument (not " + std::to_string(argument) + ") expected in library function \"sqrt\" \n");

  number = avm_get_actual(0);

  if(number->type != number_avm)
    avm_error("library function with name \"sqrt\" take as argument only number.\n");

  avm_memcellclear(&_retval);

  if(number->data.realVal < 0)
    _retval.type          = nil_avm;
  else
  {
    _retval.type          = number_avm;
    _retval.data.realVal  = sqrt(number->data.realVal);
  }
}

void library_function_sin()
{
  avm_memcell* number    = nullptr;
  unsigned int argument  = avm_get_total_actuals();

  if(argument != 1)
    avm_error("one argument (not " + std::to_string(argument) + ") expected in library function \"sin\" \n");

  number = avm_get_actual(0);

  if(number->type != number_avm)
    avm_error("library function with name \"sin\" take as argument only number.\n");

  number->data.realVal= (number->data.realVal * M_PI)/180;

  avm_memcellclear(&_retval);

  _retval.type          = number_avm;
  _retval.data.realVal  = sin(number->data.realVal);
}

void library_function_cos()
{
  avm_memcell* number    = nullptr;
  unsigned int argument  = avm_get_total_actuals();

  if(argument != 1)
    avm_error("one argument (not " + std::to_string(argument) + ") expected in library function \"cos\" \n");

  number = avm_get_actual(0);

  if(number->type != number_avm)
    avm_error("library function with name \"cos\" take as argument only table.\n");

  number->data.realVal= (number->data.realVal * M_PI)/180;

  avm_memcellclear(&_retval);

  _retval.type          = number_avm;
  _retval.data.realVal  = cos(number->data.realVal);
}

void make_table_call(avm_memcell* table)
{
  avm_memcell* index  = new avm_memcell();
  avm_memcell* result;

  std::string parenthe("()");

  index->type           = string_avm;
  index->data.stringVal = strdup(const_cast<char*>(parenthe.c_str()));

  result = avm_table_getelement(table->data.tableVal , index);

  if(result->type == nil_avm  ||  result->type != userfunc_avm && result->type != libfunc_avm)
    avm_error("You cannot overload operator \"()\" in table , the type must be userfunc or libfunc type not " + _types_to_string[result->type] + " .");
  else
  { 
    if(result->type == userfunc_avm)
    {
      if(_userfuncs_array[result->data.userFuncVal].total_arguments > _total_actuals)
      {
        avm_error("user function with name \"" + _userfuncs_array[result->data.userFuncVal].id
                  + "\" takes " + std::to_string(_userfuncs_array[result->data.userFuncVal].total_arguments)
                  + " arguments not " + std::to_string(_total_actuals) + "."
                  );
      }

      _pc = _userfuncs_array[result->data.userFuncVal].address;

      assert( _pc < _code_size );
      assert( _instructions[_pc].opcode == funcenter_v );
    }
    else if(result->type == libfunc_avm)
    {
      avm_call_library_function(result->data.stringVal);
    }
  }
}

void execute_call(instruction* instru)
{
  avm_memcell *function = avm_translate_operand(instru->result , &_ax);

  assert( function );

  if(function->type != table_avm)
    avm_save_enviroment();

  switch (function->type)
  {
    case userfunc_avm:
      if(_userfuncs_array[function->data.userFuncVal].total_arguments > _total_actuals)
      {
        avm_error("user function with name \"" + _userfuncs_array[function->data.userFuncVal].id
                  + "\" takes " + std::to_string(_userfuncs_array[function->data.userFuncVal].total_arguments)
                  + " arguments not " + std::to_string(_total_actuals) + "."
                 );
        break;
      }

      _pc = _userfuncs_array[function->data.userFuncVal].address;

      assert( _pc < _code_size );
      assert( _instructions[_pc].opcode == funcenter_v );

      break;
    case libfunc_avm:
      avm_call_library_function(function->data.libFuncVal);

      break;
    case string_avm:
      avm_call_library_function(function->data.stringVal);

      break;
    case table_avm:
      avm_assign(&_stack[_top] , function);
      ++_total_actuals;
      avm_dec_top();

      avm_save_enviroment();
      
      make_table_call(function);

      break;
    default:
      std::string message(avm_tostring(function));

      avm_error("call : cannot bind \"" + message + "\" to function.");

      break;
  }
}

void execute_pusharg(instruction* instru)
{
  avm_memcell * argument = avm_translate_operand(instru->result , &_ax);

  assert(argument);

  avm_assign(&_stack[_top] , argument);
  ++_total_actuals;
  avm_dec_top();
}

void execute_funcenter(instruction* instru)
{
  avm_memcell* function       = avm_translate_operand(instru->result , &_ax);

  assert( function );
  assert( function->data.userFuncVal >= 0 && function->data.userFuncVal < _userfuncs_array.size() );
  assert( _pc == _userfuncs_array[function->data.userFuncVal].address );

  _total_actuals = 0;

  _topsp = _top;
  _top   = _top - _userfuncs_array[function->data.userFuncVal].total_local;
}

void execute_funcexit(instruction* instru)
{
  unsigned int old_top = _top;

  _top   = avm_get_environment_value(_topsp + AVM_SAVEDTOP_OFFSET);
  _pc    = avm_get_environment_value(_topsp + AVM_SAVEDPC_OFFSET);
  _topsp = avm_get_environment_value(_topsp + AVM_SAVEDTOPSP_OFFSET);

  while(++old_top <= _top)
    avm_memcellclear(&_stack[old_top]);
}
