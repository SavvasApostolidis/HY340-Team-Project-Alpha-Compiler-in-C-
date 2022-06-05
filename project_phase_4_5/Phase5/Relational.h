#ifndef RELATIONAL_H
#define RELATIONAL_H

#include "Avm.h"
#include "Table.h"

#define execute_jle execute_relational
#define execute_jge execute_relational
#define execute_jlt execute_relational
#define execute_jgt execute_relational

inline bool jle_implementation (double x , double y);
inline bool jge_implementation (double x , double y);
inline bool jlt_implementation (double x , double y);
inline bool jgt_implementation (double x , double y);

void execute_relational(instruction* instru);

void execute_jeq(instruction* instru);
void execute_jne(instruction* instru);

void execute_jle(instruction* instru);
void execute_jge(instruction* instru);
void execute_jlt(instruction* instru);
void execute_jgt(instruction* instru);

void execute_jump(instruction* instru);

typedef bool (*relational_func_t)(double x , double y);

#endif
