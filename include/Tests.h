#ifndef TESTS_H
#define TESTS_H

#include <assert.h>
#include "GC.h"
#include "Obj.h"
#include "Tests.h"
#include "Parser.h"
#include "Runtime.h"
#include "Bytecode.h"
#include "Compiler.h"

void test_gc() {
  GC *gc = gc_new();

  Obj *sym1 = gc_make_symbol(gc, "sym1");
  Obj *sym2 = gc_make_symbol(gc, "sym2");
  Obj *sym3 = gc_make_symbol(gc, "sym3");
  Obj *sym4 = gc_make_symbol(gc, "sym4");
  
  Obj *cell1 = gc_make_cons(gc, NULL, NULL);
  Obj *cell2 = gc_make_cons(gc, NULL, NULL);
  Obj *cell3 = gc_make_cons(gc, NULL, NULL);
  Obj *cell4 = gc_make_cons(gc, NULL, NULL);

  cell1->car = sym1;
  cell1->cdr = cell2;
  cell2->car = sym2;

  // lone loop
  cell3->car = cell4;
  cell4->cdr = cell3;

  gc_stack_push(gc, cell1);

  GCResult r1 = gc_collect(gc);
  assert(r1.alive == 4);
  assert(r1.freed == 4);
	 
  GCResult r2 = gc_collect(gc);
  assert(r2.alive == 4);
  assert(r2.freed == 0);

  gc_stack_pop_safely(gc);
  
  GCResult r3 = gc_collect(gc);
  assert(r3.alive == 0);
  assert(r3.freed == 4);

  gc_delete(gc);
}

void test_printing() {
  GC *gc = gc_new();
  
  Obj *cell1 = gc_make_cons(gc, NULL, NULL);
  Obj *sym1 = gc_make_symbol(gc, "sym1");
  Obj *cell2 = gc_make_cons(gc, sym1, cell1);
  Obj *sym2 = gc_make_symbol(gc, "sym2");
  Obj *cell3 = gc_make_cons(gc, sym2, cell2);
  Obj *sym3 = gc_make_symbol(gc, "sym3");
  Obj *cell4 = gc_make_cons(gc, sym3, cell3);
  
  print_obj(cell1); printf("\n");
  print_obj(cell2); printf("\n");
  print_obj(cell3); printf("\n");
  print_obj(cell4); printf("\n");

  // Add a list in the middle of the list
  Obj *cell5 = gc_make_cons(gc, NULL, NULL);
  Obj *cell6 = gc_make_cons(gc, gc_make_symbol(gc, "sym10"), cell5);
  Obj *cell7 = gc_make_cons(gc, gc_make_symbol(gc, "sym20"), cell6);
  cell3->car = cell7;
  print_obj(cell4); printf("\n");

  // A weird cell with head but no tail
  cell1->car = sym1;
  cell1->cdr = NULL;
  print_obj(cell1); printf("\n");

  gc_collect(gc);
  gc_delete(gc);
}

void test_parsing() {
  GC *gc = gc_new();  
  Obj *forms = parse(gc, "() a b c (d e) ((f g h () ()) (() i j) (k (() l ()) m))");
  print_obj(forms);
  gc_delete(gc);
}

void test_runtime() {
  Runtime *r = runtime_new(true);
  //runtime_eval(r, "(def a (quote b)) a bleh (bleh)");
  //runtime_eval(r, "(bleh) (print-sym (quote apa)) (print-two-syms (quote erik) (quote svedang))");
  //runtime_eval(r, "(+ 2 3)");
  //runtime_eval(r, "\"erik\"");
  //runtime_eval(r, "(break) 3 4 5 (+ 2 3) (break) 10 20");
  //runtime_inspect_env(r);
  runtime_delete(r);
}

void test_str_allocs() {
  printf("sizeof(char) = %ld\n", sizeof(char));
  
  const char *a = "aha";
  printf("sizeof(a) = %ld\n", sizeof(a));

  const char *b = "booo";
  printf("sizeof(b) = %ld\n", sizeof(b));

  const char *c = "jo men så attehh....";
  printf("sizeof(c) = %ld\n", sizeof(c));

  const char *d = malloc(256);
  printf("sizeof(d) = %ld\n", sizeof(d));  
}

void test_store_pointer_in_int_array() {
  Obj *o = malloc(sizeof(Obj));
  o->name = "foo";
  
  int a[4];
  a[0] = 10;
  a[1] = 20;
  a[2] = 30;
  a[3] = 40;

  printf("o = %p\n", o);

  // The Obj* takes up two slots in the array
  Obj **p = (Obj**)&a[1];
  *p = o;

  for (int i = 0; i < 4; i++) {
    printf("%d: %d\n", i, a[i]);
  }

  // Treat the [1] position as an Obj* instead of an int
  int *ap = &a[1];
  Obj **oo = (Obj**)ap;
  printf("oo = %p\n", *oo);
  
  printf("name = %s\n", (*oo)->name);
}

void test_bytecode() {
  /* printf("sizeof(Code) = %lu\n", sizeof(Code)); */
  /* printf("sizeof(Obj*) = %lu\n", sizeof(Obj*)); */
  /* printf("sizeof(Code*) = %lu\n", sizeof(Code*)); */
  /* printf("sizeof(int) = %lu\n", sizeof(int)); */
  
  Runtime *r = runtime_new(true);

  //runtime_inspect_env(r);
  //runtime_print_frames(r);

  runtime_env_assoc(r, r->global_env,
		    gc_make_symbol(r->gc, "x"),
		    gc_make_number(r->gc, 12345));
  
  CodeWriter writer;
  code_writer_init(&writer, 1024);
  code_write_push_constant(&writer, gc_make_number(r->gc, 42.0));
  code_write_define(&writer, gc_make_symbol(r->gc, "bleh")); // bleh = 42
  code_write_push_constant(&writer, gc_make_number(r->gc, 100.0));
  code_write_push_constant(&writer, gc_make_number(r->gc, 200.0));
  code_write_direct_lookup_var(&writer, runtime_env_find_pair(r->global_env, gc_make_symbol(r->gc, "+")));
  code_write_call(&writer, 2);
  code_write_direct_lookup_var(&writer, runtime_env_find_pair(r->global_env, gc_make_symbol(r->gc, "bleh")));
  code_write_direct_lookup_var(&writer, runtime_env_find_pair(r->global_env, gc_make_symbol(r->gc, "*")));
  code_write_call(&writer, 2);
  code_write_end(&writer);

  //code_print(writer.codes);
  
  runtime_frame_push(r, 0, NULL, writer.codes, "testframe");

  while(r->top_frame >= 0) {
    runtime_step_eval(r);
  }

  gc_stack_print(r->gc, false);
  runtime_delete(r);
}

void test_bytecode_jump() {
  Runtime *r = runtime_new(true);

  CodeWriter writer;
  code_writer_init(&writer, 1024);
  code_write_push_constant(&writer, gc_make_number(r->gc, 10));
  code_write_jump(&writer, 6);
  code_write_push_constant(&writer, gc_make_number(r->gc, 20));
  code_write_push_constant(&writer, gc_make_number(r->gc, 30));
  code_write_push_constant(&writer, gc_make_number(r->gc, 40));
  code_write_push_constant(&writer, gc_make_number(r->gc, 50));
  code_write_end(&writer);

  //code_print(writer.codes);
  
  runtime_frame_push(r, 0, NULL, writer.codes, "testframe");

  while(r->top_frame >= 0) {
    runtime_step_eval(r);
  }

  gc_stack_print(r->gc, false);
  runtime_delete(r);
}



void test_bytecode_if() {

  Runtime *r = runtime_new(true);

  int code_length = 0;
  Code *c = compile(r, false, parse(r->gc, "(if 1 1337 404)")->car, &code_length, NULL);
  code_print(c);
  printf("Code length: %d\n", code_length);
  //return;

  printf("\n\n ************************************************ \n\n");

  // The following code works and is what we want to produce above
    
  CodeWriter writer;
  code_writer_init(&writer, 1024);

  //code_write_push_constant(&writer, r->nil); // <-- false
  code_write_push_constant(&writer, gc_make_number(r->gc, 1)); // <-- true

  int length_of_false_block = 5;
  int length_of_true_block = 3;
  
  code_write_if(&writer);
  code_write_jump(&writer, length_of_false_block); // this one leads to the true branch
  // false branch
  code_write_push_constant(&writer, gc_make_number(r->gc, 404));
  code_write_jump(&writer, length_of_true_block);
  // true branch
  code_write_push_constant(&writer, gc_make_number(r->gc, 1337));
  // merge
  code_write_push_constant(&writer, gc_make_string(r->gc, "BRANCHES MERGE HERE"));
  code_write_end(&writer);

  code_print(writer.codes);
  printf("\n");
  
  runtime_frame_push(r, 0, NULL, writer.codes, "top-level");

  while(r->top_frame >= 0) {
    runtime_step_eval(r);
  }

  printf("\n");
  gc_stack_print(r->gc, false);
  runtime_delete(r);
}



void test_bytecode_with_lambda() {
  Runtime *r = runtime_new(true);
  
  CodeWriter writer;

  // Write code for lambda: (fn (dront) (* dront dront))
  code_writer_init(&writer, 1024);
  code_write_push_constant(&writer, gc_make_symbol(r->gc, "dront"));
  code_write_push_constant(&writer, gc_make_symbol(r->gc, "dront"));
  code_write_direct_lookup_var(&writer, runtime_env_find_pair(r->global_env, gc_make_symbol(r->gc, "*")));
  code_write_call(&writer, 2);
  code_write_return(&writer);
  code_write_end(&writer);

  //code_print(writer.codes);
  Code *lambda_code = writer.codes;

  Obj *forms = parse(r->gc, "(fn (dront) (* dront dront))");
  Obj *form = forms->car;
  Obj *args = form->cdr->car;
  Obj *body = form->cdr->cdr->car;
  /* printf("Args: "); print_obj(args); printf("\n"); */
  /* printf("Body: "); print_obj(body); printf("\n"); */

  // Write code for main: ((fn (dront) (* dront dront)) 5)
  code_writer_init(&writer, 1024);
  code_write_push_constant(&writer, gc_make_number(r->gc, 5.0));
  code_write_push_lambda(&writer, args, body);
  code_write_call(&writer, 1); // one arg
  code_write_end(&writer);
  //code_print(writer.codes);
  
  runtime_frame_push(r, 0, NULL, writer.codes, "testframe");

  while(r->top_frame >= 0) {
    runtime_step_eval(r);
  }

  gc_stack_print(r->gc, false);
  runtime_delete(r);
}

void test_compiler() {
  compile_and_print("(def a 10)");
  //compile_and_print("(+ 2 3)");
  //compile_and_print("(fn (x y) (* x x y))");
  return;

  Runtime *r = runtime_new(true);

  Obj *forms = parse(r->gc, "(- 20 3)");
  Obj *form = forms->car;
  int len;
  Code *code = compile(r, false, form, &len, NULL);
  code_print(code);
  runtime_frame_push(r, 0, NULL, code, "top-level");
  
  while(r->top_frame >= 0) {
    runtime_step_eval(r);
  }

  gc_stack_print(r->gc, false);
  runtime_delete(r);
}

void test_sizes() {
  printf("Obj size: %lu\n", sizeof(Obj));
}

void tests() {
  test_gc();
  //test_printing();
  //test_parsing();
  //test_runtime();
  //test_local_environments();
  //test_str_allocs();
  //test_bytecode();
  //test_bytecode_jump();
  //test_bytecode_if();
  //test_bytecode_with_lambda();
  //test_compiler();
}

#endif
