#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include "Avm.h"

#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

void execute_add (instruction* instru);

void execute_sub (instruction* instru);
void execute_mul (instruction* instru);
void execute_div (instruction* instru);
void execute_mod (instruction* instru);

inline double sub_implementation (double x , double y);
inline double mul_implementation (double x , double y);
inline double div_implementation (double x , double y);
inline double mod_implementation (double x , double y);

typedef double (*arithmetic_func_t)(double x , double y);

#endif
