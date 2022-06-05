#include "Table.h"

extern avm_memcell _stack[AVM_STACKSIZE];

extern std::string _types_to_string[];

extern avm_memcell _ax;
extern avm_memcell _bx;
extern avm_memcell _retval;

extern  int _top;

bool compare_tables(avm_table* left_table , avm_table* right_table)
{
  if(!left_table->integerIndexed.empty() && !right_table->integerIndexed.empty())
  {
    for(auto items : left_table->integerIndexed)
    {
      auto find = right_table->integerIndexed.find(items.first);

      if(find != right_table->integerIndexed.end())
      {
        switch(items.second->type)
        {
          case number_avm   :
            if(items.second->data.realVal != find->second->data.realVal)
               return false;
            break;
          case string_avm   :
            if(strcmp(items.second->data.stringVal , find->second->data.stringVal) != 0)
              return false;
            break;
          case bool_avm     :
            if(items.second->data.boolVal != find->second->data.boolVal)
              return false;
            break;
          case libfunc_avm  :
            if(strcmp(items.second->data.libFuncVal , find->second->data.libFuncVal) != 0)
              return false;
            break;
          case userfunc_avm :
            if(items.second->data.userFuncVal != find->second->data.userFuncVal)
              return false;
            break;
          case table_avm    :
            if(!compare_tables(items.second->data.tableVal,find->second->data.tableVal))
              return false;
            break;
          default           :
              return false;
        }
      }
      else
      {
        return false;
      }
    }
  }

  if(!left_table->stringIndexed.empty() && !right_table->stringIndexed.empty())
  {
    for(auto items : left_table->stringIndexed)
    {
      auto find = right_table->stringIndexed.find(items.first);

      if(find != right_table->stringIndexed.end())
      {
        switch(items.second->type)
        {
          case number_avm   :
            if(items.second->data.realVal != find->second->data.realVal)
               return false;
            break;
          case string_avm   :
            if(strcmp(items.second->data.stringVal , find->second->data.stringVal) != 0)
              return false;
            break;
          case bool_avm     :
            if(items.second->data.boolVal != find->second->data.boolVal)
              return false;
            break;
          case libfunc_avm  :
            if(strcmp(items.second->data.libFuncVal , find->second->data.libFuncVal) != 0)
              return false;
            break;
          case userfunc_avm :
            if(items.second->data.userFuncVal != find->second->data.userFuncVal)
              return false;
            break;
          case table_avm    :
            if(!compare_tables(items.second->data.tableVal,find->second->data.tableVal))
              return false;
            break;
          default:
            return false;
        }
      }
      else
      {
        return false;
      }
    }
  }

  return true;
}

bool table_tobool(avm_memcell* memcell_to_bool)
{
  return true;
}

std::string table_tostring(avm_memcell* memcell_to_string)
{
  std::string table_content("[ ");

  std::map<double, avm_memcell*>::iterator      rit;
  std::map<std::string, avm_memcell*>::iterator srit;

  if(!memcell_to_string->data.tableVal->integerIndexed.empty())
  {
    for (rit = memcell_to_string->data.tableVal->integerIndexed.begin();
        rit != memcell_to_string->data.tableVal->integerIndexed.end();
        ++rit)
    {
      if(rit->second->type == table_avm && memcell_to_string->type == table_avm) 
        if(rit->second->data.tableVal == memcell_to_string->data.tableVal)
          break;

      if((std::next(rit) == memcell_to_string->data.tableVal->integerIndexed.end()) && memcell_to_string->data.tableVal->stringIndexed.empty())
        table_content = table_content + "{" + std::to_string(rit->first) + " : " + avm_tostring(rit->second) + "} ";
      else
        table_content = table_content + "{" + std::to_string(rit->first) + " : " + avm_tostring(rit->second) + "}, ";
    }
  }

  if(!memcell_to_string->data.tableVal->stringIndexed.empty())
  {
    for (srit = memcell_to_string->data.tableVal->stringIndexed.begin();
        srit != memcell_to_string->data.tableVal->stringIndexed.end();
        ++srit)
    {
      if(srit->second->type == table_avm && memcell_to_string->type == table_avm) 
        if(srit->second->data.tableVal == memcell_to_string->data.tableVal)
          break;

      if( std::next(srit) == memcell_to_string->data.tableVal->stringIndexed.end())
        table_content = table_content + "{" + srit->first + " : " + avm_tostring(srit->second) + "} ";
      else
        table_content = table_content + "{" + srit->first + " : " + avm_tostring(srit->second) + "}, ";
    }
  }

  table_content += " ]";

  return std::string(table_content);
}

void memcell_clear_table(avm_memcell* table)
{
  assert(table->data.tableVal);

  avm_decrease_reference_counter(table->data.tableVal);
}

void execute_newtable(instruction* instru)
{
  avm_memcell* left_value = avm_translate_operand(instru->result , nullptr);

  assert( left_value && ( (&_stack[AVM_STACKSIZE - 1] >= left_value && &_stack[_top] < left_value ) || left_value == &_retval ) );

  avm_memcellclear(left_value);

  left_value->type            = table_avm;
  left_value->data.tableVal   = new avm_table();

  avm_increase_reference_counter(left_value->data.tableVal);
}

void execute_tablegetelem(instruction* instru)
{
  avm_memcell* left_value = avm_translate_operand(instru->result , nullptr);
  avm_memcell* table      = avm_translate_operand(instru->arg1 , nullptr);
  avm_memcell* index      = avm_translate_operand(instru->arg2 , &_ax);

  assert( left_value && ( (&_stack[AVM_STACKSIZE - 1] >= left_value && &_stack[_top] < left_value )  || left_value == &_retval ) );
  assert( table && ( &_stack[AVM_STACKSIZE - 1] >= left_value && &_stack[_top] < left_value ) );
  assert( index );

  avm_memcellclear(left_value);

  left_value->type = nil_avm;

  if(table->type != table_avm)
    avm_error("illegal use of type " +  _types_to_string[table->type]  + " as table.");
  else
  {
    avm_memcell* value = avm_table_getelement(table->data.tableVal , index);

    avm_assign(left_value , value);
  }
}

void execute_tablesetelem(instruction* instru)
{
  avm_memcell* table = avm_translate_operand(instru->result , nullptr);
  avm_memcell* index = avm_translate_operand(instru->arg1 , &_ax);
  avm_memcell* value = avm_translate_operand(instru->arg2 , &_bx);

  assert( table && ( &_stack[AVM_STACKSIZE-1] >= table && &_stack[_top] < table ) );
  assert( index );
  assert( value );

  if(table->type != table_avm)
    avm_error("illegal use of type " +  _types_to_string[table->type]  + " as table.");
  else
    avm_table_setelement(table->data.tableVal , index , value );

}
