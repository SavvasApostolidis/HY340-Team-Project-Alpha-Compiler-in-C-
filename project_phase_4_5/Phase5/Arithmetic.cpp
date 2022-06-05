#include "Arithmetic.h"

extern avm_memcell _stack[AVM_STACKSIZE];

extern avm_memcell _ax;
extern avm_memcell _bx;
extern avm_memcell _retval;

extern int _top;

extern std::string _types_to_string[];

std::string arithmetic_operators[] =
{
  "+",
  "-",
  "*",
  "/",
  "%",
};

arithmetic_func_t arithmetic_funcs[] =
{
  sub_implementation,
  mul_implementation,
  div_implementation,
  mod_implementation
};

void execute_arithmetic (instruction* instru)
{
  avm_memcell *left_value     = avm_translate_operand(instru->result , nullptr);
  avm_memcell *right_value_1  = avm_translate_operand(instru->arg1   , &_ax);
  avm_memcell *right_value_2  = avm_translate_operand(instru->arg2   , &_bx);

  assert( left_value && ( &_stack[AVM_STACKSIZE - 1] >= left_value && left_value > &_stack[_top] ) || left_value == &_retval );
  assert( right_value_1 != nullptr && right_value_2 != nullptr );

  if( right_value_1->type != number_avm || right_value_2->type != number_avm )
    avm_error("arithmetic operator " + arithmetic_operators[instru->opcode - add_v] + " must contain only numbers not [ " + _types_to_string[right_value_1->type] + " ] and [ " + _types_to_string[right_value_2->type] + " ].");
  else
  {
    arithmetic_func_t op = arithmetic_funcs[instru->opcode - sub_v];
    avm_memcellclear(left_value);

    left_value->type          = number_avm;
    left_value->data.realVal  = (*op)(right_value_1->data.realVal , right_value_2->data.realVal);
  }
}

void execute_add (instruction* instru)
{
  avm_memcell *left_value     = avm_translate_operand(instru->result , nullptr);
  avm_memcell *right_value_1  = avm_translate_operand(instru->arg1   , &_ax);
  avm_memcell *right_value_2  = avm_translate_operand(instru->arg2   , &_bx);

  assert( left_value && ( &_stack[AVM_STACKSIZE - 1] >= left_value && left_value > &_stack[_top] ) || left_value == &_retval );
  assert( right_value_1 != nullptr && right_value_2 != nullptr );

  //Sthn monh periptwsh pou kanoume clear to left value einai otan kai ta duo einai ariumoi
  //gia na kauarhsoume se periptwsh pou prin h temp metablhth htan table string h library_function
  //Twra se periptwsh pou htan string kai emeis kauarisoume to left value kai  stis peripwsseis
  //pou ueloume na kanoume concat tote to prohgoumeno string exei xauei eksalou sthn periptwsh prohgoumeno
  //ot left value htan string kai auto pou epete kai auto pou prohgoutan htan string.
  if( right_value_1->type == number_avm && right_value_2->type == number_avm )
  {
    avm_memcellclear(left_value);

    left_value->type          = number_avm;
    left_value->data.realVal  = right_value_1->data.realVal + right_value_2->data.realVal;
  }
  else if( right_value_1->type == string_avm && right_value_2->type == number_avm )
  {
    std::string result(std::string(right_value_1->data.stringVal) + std::to_string(right_value_2->data.realVal));

    left_value->type             = string_avm;
    left_value->data.stringVal   = strdup(const_cast<char*>(result.c_str()));
  }
  else if( right_value_1->type == number_avm && right_value_2->type == string_avm )
  {
    std::string result(std::to_string(right_value_1->data.realVal) + std::string(right_value_2->data.stringVal));

    left_value->type             = string_avm;
    left_value->data.stringVal   = strdup(const_cast<char*>(result.c_str()));
  }
  else if( right_value_1->type == string_avm && right_value_2->type == string_avm )
  {
    std::string result(std::string(right_value_1->data.stringVal) + std::string(right_value_2->data.stringVal));

    left_value->type             = string_avm;
    left_value->data.stringVal   = strdup(const_cast<char*>(result.c_str()));
  }else
    avm_error("arithmetic operator " + arithmetic_operators[instru->opcode - add_v] + " must contain only numbers not [ " + _types_to_string[right_value_1->type] + " ] and [ " + _types_to_string[right_value_2->type] + " ].");
}

inline double sub_implementation (double x , double y)
{
  return x - y;
}

inline double mul_implementation (double x , double y)
{
  return x * y;
}

inline double div_implementation (double x , double y)
{
  if(y == ((double)0)){
    avm_error("illegal action , divide with 0.");
    return 0;
  }
  return  x / y;
}

inline double mod_implementation (double x , double y)
{
  if(y == ((double)0)){
    avm_error("illegal action , divide with 0.");
    return 0;
  }
  return ((unsigned int) x) % ((unsigned int) y);
}
