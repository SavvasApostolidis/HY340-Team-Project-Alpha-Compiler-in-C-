#include "Assign.h"

extern avm_memcell _stack[AVM_STACKSIZE];

extern avm_memcell _ax;
extern avm_memcell _retval;

extern int _top;

bool number_tobool(avm_memcell* memcell_to_bool)
{
  return memcell_to_bool->data.realVal != 0;
}

bool string_tobool(avm_memcell* memcell_to_bool)
{
  return strlen(memcell_to_bool->data.stringVal) != 0;
}

bool bool_tobool(avm_memcell* memcell_to_bool)
{
  return memcell_to_bool->data.boolVal;
}

bool nil_tobool(avm_memcell* memcell_to_bool)
{
  return false;
}

bool undef_tobool(avm_memcell* memcell_to_bool)
{
  assert(0);
  return false;
}

std::string number_tostring(avm_memcell* memcell_to_string)
{
  std::string number = std::to_string(memcell_to_string->data.realVal);

  char* formated = (char*)malloc(sizeof(char) * (number.size()) + 1);

  sprintf(formated , "%.3f" , memcell_to_string->data.realVal);

  return std::string((formated));
}

std::string string_tostring(avm_memcell* memcell_to_string)
{
  return std::string(memcell_to_string->data.stringVal);
}

std::string bool_tostring(avm_memcell* memcell_to_string)
{
  return (memcell_to_string->data.boolVal)  ? (std::string("true"))
                                            : (std::string("false"));
}

std::string nil_tostring(avm_memcell* memcell_to_string)
{
  return std::string("nil");
}

std::string undef_tostring(avm_memcell* memcell_to_string)
{
  return std::string("undefined");
}

void memcell_clear_string(avm_memcell* memcell_to_clear)
{
  assert(memcell_to_clear->data.stringVal);
  free(memcell_to_clear->data.stringVal);
}

void avm_assign(avm_memcell* left_value , avm_memcell* right_value)
{
  if(left_value == right_value)
    return;

  if(left_value->type   == table_avm &&
     right_value->type  == table_avm &&
     left_value->data.tableVal == right_value->data.tableVal)
    return;

  if(right_value->type == undef_avm && right_value != &_retval)
    avm_warning("assigning form undefined value [lv = undef]");

  avm_memcellclear(left_value);

  memcpy(left_value , right_value , sizeof(avm_memcell));

  if(left_value->type == string_avm)
    left_value->data.stringVal = strdup(right_value->data.stringVal);
  else if(left_value->type == libfunc_avm)
    left_value->data.libFuncVal = strdup(right_value->data.libFuncVal);
  else if(left_value->type == table_avm){
    avm_increase_reference_counter(left_value->data.tableVal);
  }
}

void execute_assign (instruction* instru)
{
  avm_memcell *left_value   = avm_translate_operand(instru->result , nullptr);
  avm_memcell *right_value  = avm_translate_operand(instru->arg1 , &_ax);

  assert( left_value && ( ( &_stack[AVM_STACKSIZE - 1] >= left_value && left_value > &_stack[_top] ) || left_value == &_retval ) );

  avm_assign(left_value , right_value);
}

void execute_nop(instruction* instru)
{
  avm_warning("nop instruction.");
}
