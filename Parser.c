#include "Parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

typedef struct {
  int pos;
} Parser;

Parser *parser_make() {
  Parser *p = malloc(sizeof(Parser));
  p->pos = 0;
  return p;
}

Obj *parse_list(GC *gc, Parser *p, const char *source);

bool iswhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}

Obj *parse_form(GC *gc, Parser *p, const char *source) {
  //printf("Parsing form\n");
  if (source[p->pos] == '(') {
    //printf("Found (\n");
    return parse_list(gc, p, source);
  }
  else if(isalpha(source[p->pos])) {
    //printf("Found symbol: ");
    char *name = malloc(sizeof(char) * 256); // TODO: free this when the Obj is freed
    char i = 0;
    while(!iswhitespace(source[p->pos]) &&
	  source[p->pos] != ')' &&
	  source[p->pos] != '(') {
      name[i++] = source[p->pos];
      p->pos++;
    }
    name[i] = '\0';
    //printf("%s\n", name);
    return gc_make_symbol(gc, name);
  }

  //printf("No form found, will return NULL\n");
  return NULL;
}

Obj *parse_list(GC *gc, Parser *p, const char *source) {
  Obj *list = gc_make_cons(gc, NULL, NULL);
  Obj *lastCons = list;

  //printf("Starting work on list %p\n", list);

  p->pos++; // move beyond the first paren
  
  while(1) {
    Obj *item = parse_form(gc, p, source);
    if(item) {
      //printf("Adding item to list %p: %s\n", list, obj_to_str(item));
      lastCons->car = item;
      Obj *next = gc_make_cons(gc, NULL, NULL);
      lastCons->cdr = next;
      lastCons = next;
    }

    if(source[p->pos] == ')') {
      //printf("Found )\n");
      p->pos++;
      break;
    }
    
    p->pos++;
  }

  //printf("Ending work on list %p\n", list);  
  return list;
}

Obj *parse(GC *gc, const char *source) {
  Parser* p = parser_make();
  int source_len = strlen(source);
  int parens = 0;

  Obj *forms = gc_make_cons(gc, NULL, NULL);
  Obj *last_form = forms;
  
  while(p->pos < source_len) {
    //printf("Looking for new form at pos %d \n", p->pos);
    Obj *form = parse_form(gc, p, source);
    if(form) {
      //printf("Got top-level form: ");
      //print_obj(form);
      //printf("\n");
      last_form->car = form;
      last_form->cdr = gc_make_cons(gc, NULL, NULL);
      last_form = last_form->cdr;
    }

    p->pos++;
  }

  //printf("%d parens.\n", parens);
 
  return forms;
}