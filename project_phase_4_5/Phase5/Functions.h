#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <math.h>
#include <algorithm>

#include "Avm.h"
#include "Assign.h"

bool userfunc_tobool(avm_memcell* memcell_to_bool);

bool libfunc_tobool(avm_memcell* memcell_to_bool);


std::string userfunc_tostring(avm_memcell* memcell_to_string);

std::string libfunc_tostring(avm_memcell* memcell_to_string);


void memcell_clear_libfunc(avm_memcell *lib_func);

void copy_content(avm_memell_t type , avm_memcell* src , avm_memcell* dest);


void library_function_print();

void library_function_input();

void library_function_typeof();

void library_function_totalarguments();

void library_function_agrument();

void library_function_objectmemberkeys();

void library_function_objecttotalmembers();

void library_function_objectcopy();

void library_function_strtonum();

void library_function_sqrt();

void library_function_sin();

void library_function_cos();


void make_table_call(avm_memcell* table);

void execute_call(instruction* instru);

void execute_pusharg(instruction* instru);

void execute_funcenter(instruction* instru);

void execute_funcexit(instruction* instru);

#endif
