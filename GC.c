#include "GC.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define LOG 0
#define LOG_GC_COLLECT_RESULT 1
#define LOG_PUSH_AND_POP 0

void gc_stack_push(GC *gc, Obj *o) {
  if(gc->stackSize >= STACK_MAX) error("Stack overflow.");
  gc->stack[gc->stackSize++] = o;
  #if LOG_PUSH_AND_POP
  printf("Pushed: ");
  print_obj(o);
  printf("\n");
  #endif
}

Obj *gc_stack_pop(GC *gc) {
  if(gc->stackSize < 0) error("Stack underflow.");
  Obj *o = gc->stack[--gc->stackSize];
  #if LOG_PUSH_AND_POP
  printf("Popped: ");
  print_obj(o);
  printf("\n");
  #endif
  return o;
}

void gc_stack_print(GC *gc) {
  printf("Value stack:\n");
  for(int i = gc->stackSize - 1; i >= 0; i--) {
    printf("%d:\t", i);
    print_obj(gc->stack[i]);
    printf("\n");
  }
  printf("------------\n");
}

Obj *gc_make_obj(GC *gc, Type type) {
  Obj *o = malloc(sizeof(Obj));
  o->reachable = false;
  o->type = type;
  o->name = NULL;

  #if LOG
  printf("Created obj %p of type %s.\n", o, type_to_str(o->type));
  #endif
  
  // Put new object first in the linked list containing all objects:
  o->next = gc->firstObj; 
  gc->firstObj = o;
  
  return o;
}

Obj *gc_make_cons(GC *gc, Obj *car, Obj *cdr) {
  Obj *o = gc_make_obj(gc, CONS);
  o->car = car;
  o->cdr = cdr;
  return o;
}

void set_name(Obj *o, const char *name) {
  char *name_copy = malloc(sizeof(char) * strlen(name));
  strcpy(name_copy, name);
  o->name = name_copy;
}

Obj *gc_make_symbol_from_malloced_string(GC *gc, char *name) {
  Obj *o = gc_make_obj(gc, SYMBOL);
  o->name = name;
  return o;
}

Obj *gc_make_symbol(GC *gc, const char *name) {
  Obj *o = gc_make_obj(gc, SYMBOL);
  set_name(o, name);
  return o;
}

Obj *gc_make_func(GC *gc, const char *name, void *f) {
  Obj *o = gc_make_obj(gc, FUNC);
  o->func = f;
  o->name = (char*)name; // names of funcs are static strings and will not need to be freed when Obj is GC:d
  return o;
}

Obj *gc_make_number(GC *gc, double x) {
  Obj *o = gc_make_obj(gc, NUMBER);
  o->number = x;
  return o;
}

Obj *gc_make_string(GC *gc, char *text) {
  Obj *o = gc_make_obj(gc, STRING);
  o->name = text;
  return o;
}

Obj *gc_make_lambda(GC *gc, Obj *env, Obj *args, Obj *body) {
  Obj *o = gc_make_obj(gc, LAMBDA);
  Obj *envAndArgs = gc_make_cons(gc, env, args);
  o->car = envAndArgs;
  o->cdr = body;
  return o;
}

void gc_delete(Obj *o) {
  if(o->type == SYMBOL || o->type == STRING) {
    free(o->name);
  }
  free(o);
}

void mark(Obj *o) {
  #if LOG
  printf("Marking %p, %s as reachable.\n", o, obj_to_str(o));
  #endif
  
  if(o->reachable) {
    // This one has already been visited by mark(), return to avoid infinite loops
    return;
  }
  
  o->reachable = true;
  
  if (o->type == CONS || o->type == LAMBDA) {
    if(o->car) {
      mark(o->car);
    }
    if(o->cdr) {
      mark(o->cdr);
    }
  }
}

void gc_init(GC *gc) {
  gc->stackSize = 0;
  gc->firstObj = NULL;
}

GCResult gc_collect(GC *gc) {
  // Objects on the stack are all 'roots', i.e. they get automatically marked
  for (int i = 0; i < gc->stackSize; i++) {
    mark(gc->stack[i]);
  }

  GCResult result;
  result.alive = 0;
  result.freed = 0;
  
  // Sweep!
  Obj** obj = &gc->firstObj;
  while (*obj) {
    #if LOG
    printf("Sweep visiting %p, %s. ", *obj, obj_to_str(*obj));
    #endif
    
    if ((*obj)->reachable) {
      #if LOG
      printf("\n");
      #endif
      Obj* reached = *obj;
      reached->reachable = false; // reached this time, unmark it for future sweeps
      obj = &(reached->next); // pointer to the next object
      result.alive++;
    } else {
      #if LOG
      printf("Will free object.\n");
      #endif      
      Obj* unreached = *obj;
      *obj = unreached->next; // change the pointer in place, *THE MAGIC*
      gc_delete(unreached);
      result.freed++;
    }
  }

  #if LOG_GC_COLLECT_RESULT
  printf("Sweep done, %d objects freed and %d object still alive.\n", result.freed, result.alive);
  #endif

  return result;
}

