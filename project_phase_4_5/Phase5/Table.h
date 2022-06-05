#ifndef TABLE_H
#define TABLE_H

#include "Avm.h"
#include "Assign.h"

bool compare_tables(avm_table* left_table , avm_table* right_table);

bool table_tobool(avm_memcell* memcell_to_bool);

std::string table_tostring(avm_memcell* memcell_to_string);

void memcell_clear_table(avm_memcell* table);

void execute_newtable(instruction* instru);

void execute_tablegetelem(instruction* instru);

void execute_tablesetelem(instruction* instru);

#endif
