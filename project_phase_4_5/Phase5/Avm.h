#ifndef AVM_H
#define AVM_H

#include "unistd.h"
#include "stdio.h"
#include <assert.h>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cstring>

#include "Instructions.h"

#define AVM_STACKSIZE         4096
#define AVM_STACKENV_SIZE     4

#define AVM_NUMACTUALS_OFFSET 4
#define AVM_SAVEDPC_OFFSET    3
#define AVM_SAVEDTOP_OFFSET   2
#define AVM_SAVEDTOPSP_OFFSET 1

#define AVM_MAX_INSTRUCTIONS  (unsigned int) nop_v
#define AVM_WIPEOUT(m)        std::memset(&(m) , 0 , sizeof(m))

typedef class avm_table avm_table_t;

typedef enum avm_memcell_type
{
  number_avm    = 0,
  string_avm    = 1,
  bool_avm      = 2,
  table_avm     = 3,
  userfunc_avm  = 4,
  libfunc_avm   = 5,
  nil_avm       = 6,
  undef_avm     = 7
}avm_memell_t;

class avm_memcell
{
    public:
      avm_memell_t    type;
      union
      {
        double        realVal;
        int           integerVal;
        char*         stringVal;
        bool          boolVal;
        avm_table_t*  tableVal;
        unsigned int  userFuncVal;
        char*         libFuncVal;
      }data;

      avm_memcell()
      {
        this->type = undef_avm;
      };

      avm_memcell(avm_memell_t type)
      {
        this->type = type;
      };
};

class avm_table
{
    public:
      unsigned int reference_counter = 0;

      //std::unordered_map<double , avm_memcell* >      integerIndexed;
      //std::unordered_map<std::string , avm_memcell* > stringIndexed;

      std::map<double , avm_memcell* >      integerIndexed;
      std::map<std::string , avm_memcell* > stringIndexed;

      avm_table(){};
};


typedef void (*execute_func_t)(instruction*);

typedef void (*memclear_func_t)(avm_memcell*);

typedef bool (*tobool_func_t)(avm_memcell*);

typedef std::string (*tostring_func_t)(avm_memcell*);

typedef void (*library_func_t)();


void amv_initstack();

void avm_initialize();


void avm_error(std::string message);

void avm_warning(std::string message);


std::string avm_tostring(avm_memcell* memcell_to_string);

bool avm_tobool(avm_memcell* memcell_to_string);


void avm_memcellclear(avm_memcell* memcell_to_clear);

void avm_dec_top();

void avm_push_environment_value(unsigned int value);

unsigned int avm_get_environment_value(unsigned int value);

void avm_save_enviroment();


unsigned int avm_get_total_actuals(void);

avm_memcell* avm_get_actual(unsigned int index);


library_func_t avm_get_library_function(char* function);

void avm_call_library_function(char* library_func_to_call);


avm_memcell* avm_table_getelement(avm_table* table , avm_memcell* index);

void avm_table_setelement(avm_table* table , avm_memcell* index , avm_memcell* value);

void avm_destroy_table(avm_table* table_to_destroy);

void avm_increase_reference_counter(avm_table* table);

void avm_decrease_reference_counter(avm_table* table);


void avm_read_code();

void execute_cycle();


avm_memcell* avm_translate_operand(vmarg* arg , avm_memcell* reg);

#endif
