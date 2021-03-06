#ifndef RUNTIME_H
#define RUNTIME_H

#include "GC.h"
#include "Obj.h"
#include "Bytecode.h"

#define MAX_FRAMES 1024

typedef struct {
  char name[128];
  Code *p; // current instruction to execute
  Obj *args[64]; // change name to arg_values
  Obj *arg_symbols; // used when looking up args in enclosing scopes
} Frame;

typedef enum {
  RUNTIME_MODE_RUN,
  RUNTIME_MODE_BREAK,
  RUNTIME_MODE_FINISHED,
} RuntimeMode;

typedef struct {
  GC *gc;
  Obj *global_env;
  Obj *nil;
  Obj *true_val;
  Frame frames[MAX_FRAMES];
  int top_frame;
  RuntimeMode mode;
} Runtime;

Runtime *runtime_new(bool builtins);
void runtime_delete(Runtime *r);

void runtime_eval(Runtime *r, const char *source);
void runtime_step_eval(Runtime *r);
bool runtime_load_file(Runtime *r, const char *filename, bool silent);
void runtime_inspect_env(Runtime *r);

Frame *runtime_frame_push(Runtime *r, int arg_count, Obj *arg_symbols, Code *code, const char *name);
void runtime_frame_pop(Runtime *r);
void runtime_print_frames(Runtime *r);

void runtime_env_assoc(Runtime *r, Obj *env, Obj *key, Obj *value);
Obj *runtime_env_find_pair(Obj *env, Obj *key);

#endif
