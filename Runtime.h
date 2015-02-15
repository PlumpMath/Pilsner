#ifndef RUNTIME_H
#define RUNTIME_H

#include "GC.h"
#include "Obj.h"

#define MAX_ACTIVATION_FRAMES 256

typedef enum {
  MODE_NORMAL,
  MODE_DEF,
  MODE_FUNC_CALL,
  MODE_LAMBDA_RETURN,
  MODE_IMMEDIATE_RETURN,
  MODE_DO_BLOCK_RETURN,
  MODE_IF_BRANCH,
  MODE_IF_RETURN,
} FrameMode;

typedef struct {
  int depth;
  Obj *p; // the program counter
  FrameMode mode;
  int arg_count; // this is used when entering MODE_FUNC_CALL
  int form_count; // this is used when evaling do-forms
  char name[128];
  Obj *env;
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
  Frame frames[MAX_ACTIVATION_FRAMES];
  int top_frame;
  RuntimeMode mode;
  //bool print_top_level_result;
} Runtime;

Runtime *runtime_new();
void runtime_delete(Runtime *r);

void runtime_eval(Runtime *r, const char *source);
void runtime_inspect_env(Runtime *r);
void runtime_print_frames(Runtime *r);

void runtime_env_assoc(Runtime *r, Obj *env, Obj *key, Obj *value);
Obj *runtime_env_lookup(Obj *env, Obj *key);
Obj *runtime_env_make_local(Runtime *r, Obj *parent_env);

#endif
