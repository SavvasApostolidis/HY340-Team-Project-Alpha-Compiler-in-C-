#include "Relational.h"

extern avm_memcell _ax;
extern avm_memcell _bx;

extern unsigned int _pc;

extern bool _execution_is_finished;

extern std::string _types_to_string[];

std::string relational_operators[] =
{
  "<=",
  ">=",
  "<",
  ">"
};

relational_func_t relational_funcs[] =
{
  jle_implementation,
  jge_implementation,
  jlt_implementation,
  jgt_implementation
};

void execute_relational(instruction* instru)
{
  assert(instru->opcode >= jle_v && instru->opcode <= jgt_v);

  assert(instru->result->type == label_a);

  avm_memcell *right_value_1 = avm_translate_operand(instru->arg1 , &_ax);
  avm_memcell *right_value_2 = avm_translate_operand(instru->arg2 , &_bx);

  bool result = false;

  if(right_value_1->type != number_avm || right_value_2->type != number_avm)
    avm_error("relational operator " + relational_operators[instru->opcode - jle_v] + " must contain only numbers not [ " + _types_to_string[right_value_1->type] + " ] and [ " + _types_to_string[right_value_2->type] + " ].");
  else
  {
    relational_func_t function_to_call = relational_funcs[instru->opcode - jle_v];

    result = (*function_to_call)(right_value_1->data.realVal , right_value_2->data.realVal);

    if(!_execution_is_finished && result)
      _pc = instru->result->val;
  }
}

void execute_jeq(instruction* instru)
{
  assert(instru->result->type == label_a);

  avm_memcell *right_value_1 = avm_translate_operand(instru->arg1 , &_ax);
  avm_memcell *right_value_2 = avm_translate_operand(instru->arg2 , &_bx);

  bool result = false;

  if(right_value_1->type == undef_avm || right_value_2->type == undef_avm)
    avm_error("\"undefined\" involved in the realtional operator ( == ).");
  else if(right_value_1->type == nil_avm || right_value_2->type == nil_avm)     
    result = (right_value_1->type == nil_avm && right_value_2->type == nil_avm);
  else if(right_value_1->type == bool_avm || right_value_2->type == bool_avm)
    result = (avm_tobool(right_value_1) == avm_tobool(right_value_2));
  else if(right_value_1->type != right_value_2->type)
    avm_error(_types_to_string[right_value_1->type] + " == " + _types_to_string[right_value_2->type] + " is illegal.");
  else
  {
    switch (right_value_1->type)
    {
      case number_avm   : result = (right_value_1->data.realVal == right_value_2->data.realVal);                 break;
      case string_avm   : result = (!strcmp(right_value_1->data.stringVal , right_value_2->data.stringVal));     break;
      case libfunc_avm  : result = (!strcmp(right_value_1->data.libFuncVal , right_value_2->data.libFuncVal));   break;
      case userfunc_avm : result = (right_value_1->data.userFuncVal == right_value_2->data.userFuncVal);         break;
      case table_avm    : result = compare_tables(right_value_1->data.tableVal , right_value_2->data.tableVal);  break;
      default           : assert(0);
    }
  }

  if(!_execution_is_finished && result)
    _pc = instru->result->val;
}

void execute_jne(instruction* instru)
{
  assert(instru->result->type == label_a);

  avm_memcell *right_value_1 = avm_translate_operand(instru->arg1 , &_ax);
  avm_memcell *right_value_2 = avm_translate_operand(instru->arg2 , &_bx);

  bool result = false;

  if(right_value_1->type == undef_avm || right_value_2->type == undef_avm)
    avm_error("\"undefined\" in the realtional operator ( != ).");
  else if(right_value_1->type == nil_avm || right_value_2->type == nil_avm)
    result = (right_value_1->type == nil_avm && right_value_2->type == nil_avm);
  else if(right_value_1->type == bool_avm || right_value_2->type == bool_avm)
    result = (avm_tobool(right_value_1) == avm_tobool(right_value_2));
  else if(right_value_1->type != right_value_2->type)
    avm_error(_types_to_string[right_value_1->type] + " != " + _types_to_string[right_value_2->type] + " is illegal.");
  else
  {
    switch (right_value_1->type)
    {
      case number_avm   : result = (right_value_1->data.realVal == right_value_2->data.realVal);                 break;
      case string_avm   : result = (!strcmp(right_value_1->data.stringVal , right_value_2->data.stringVal));    break;
      case libfunc_avm  : result = (!strcmp(right_value_1->data.libFuncVal , right_value_2->data.libFuncVal));  break;
      case userfunc_avm : result = (right_value_1->data.userFuncVal == right_value_2->data.userFuncVal);         break;
      case table_avm    : result = compare_tables(right_value_1->data.tableVal , right_value_2->data.tableVal); break;
      default           : assert(0);
    }
  }

  if(!_execution_is_finished && !result)
    _pc = instru->result->val;
}

void execute_jump(instruction* instru)
{
  assert(instru->result->type == label_a);

  _pc = instru->result->val;
}

inline bool jle_implementation (double x , double y)
{
  return x <= y;
}

inline bool jge_implementation (double x , double y)
{
  return x >= y;
}

inline bool jlt_implementation (double x , double y)
{
  return x < y;
}

inline bool jgt_implementation (double x , double y)
{
  return x > y;
}
