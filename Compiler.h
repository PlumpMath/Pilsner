#ifndef COMPILER_H
#define COMPILER_H

#include "Obj.h"
#include "Bytecode.h"
#include "GC.h"
#include "Runtime.h"

Code *compile(Runtime *r, Obj *env, Obj *form, int *OUT_code_length);
void compile_and_print(const char *source);

#endif
