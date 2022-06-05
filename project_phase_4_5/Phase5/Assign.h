#ifndef ASSIGN_H
#define ASSIGN_H

#include "Avm.h"

bool number_tobool(avm_memcell* memcell_to_bool);

bool string_tobool(avm_memcell* memcell_to_bool);

bool bool_tobool(avm_memcell* memcell_to_bool);

bool nil_tobool(avm_memcell* memcell_to_bool);

bool undef_tobool(avm_memcell* memcell_to_bool);

std::string number_tostring(avm_memcell* memcell_to_string);

std::string string_tostring(avm_memcell* memcell_to_string);

std::string bool_tostring(avm_memcell* memcell_to_string);

std::string nil_tostring(avm_memcell* memcell_to_string);

std::string undef_tostring(avm_memcell* memcell_to_string);

void memcell_clear_string(avm_memcell* memcell_to_clear);

void avm_assign(avm_memcell* left_value , avm_memcell* right_value);

void execute_assign(instruction* instru);

void execute_nop(instruction* instru);

#endif
