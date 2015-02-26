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

static const char *specials = "+-*/=%&_!@?§<>λ§°'$";

bool isokinsymbol(char c) {
  for (int i = 0; i < strlen(specials); i++) {
    if(specials[i] == c) return true;
  }
  return false;
}

Obj *parse_form(GC *gc, Parser *p, const char *source) {
  //printf("Parsing form\n");
  if (source[p->pos] == '(') {
    //printf("Found (\n");
    return parse_list(gc, p, source);
  }
  else if(source[p->pos] == ';') {
    while(source[p->pos] != '\n' && source[p->pos] != '\0') {
      p->pos++;
    }
  }
  else if(source[p->pos] == '\'') {
    p->pos++;
    Obj *nil = gc_make_cons(gc, NULL, NULL);
    Obj *quote = gc_make_symbol(gc, "quote");
    Obj *quoted_form = parse_form(gc, p, source);
    Obj *rest = gc_make_cons(gc, quoted_form, nil);
    Obj *cons = gc_make_cons(gc, quote, rest);
    return cons;
  }
  else if(isalpha(source[p->pos]) || isokinsymbol(source[p->pos])) {
    char *name = malloc(sizeof(char) * 256);
    int i = 0;
    while(!iswhitespace(source[p->pos]) &&
	  source[p->pos] != ')' &&
	  source[p->pos] != '(' &&
	  source[p->pos] != '\0') {
      name[i++] = source[p->pos];
      p->pos++;
      if(i >= 256) {
	error("Can't have symbols longer than 256 chars");
      }
    }
    name[i] = '\0';
    return gc_make_symbol_from_malloced_string(gc, name);
  }
  else if(source[p->pos] == '"') {
    char *text = malloc(sizeof(char) * 256);
    int i = 0;
    p->pos++;
    while(source[p->pos] != '"' && source[p->pos] != '\0') {
      text[i++] = source[p->pos];
      p->pos++;
    }
    text[i] = '\0';
    return gc_make_string(gc, text); // TODO: use other function here?
  }
  else if(isdigit(source[p->pos])) {
    char s[256];
    int i = 0;
    while(isdigit(source[p->pos]) && source[p->pos] != '\0') {
      s[i++] = source[p->pos];
      p->pos++;
    }
    s[i] = '\0';
    double num = atof(s);
    return gc_make_number(gc, num);
  }
  
  return NULL;
}

Obj *parse_list(GC *gc, Parser *p, const char *source) {
  Obj *list = gc_make_cons(gc, NULL, NULL);
  Obj *lastCons = list;

  p->pos++; // move beyond the first paren

  if(source[p->pos] == ')') {
    p->pos++;
    return list;
  }
  
  while(1) {
        
    Obj *item = parse_form(gc, p, source);
    if(item) {
      lastCons->car = item;
      Obj *next = gc_make_cons(gc, NULL, NULL);
      lastCons->cdr = next;
      lastCons = next;
    }

    if(source[p->pos] == '\0') {
      printf("Parser error: Missing ending parenthesis.\n");
      return NULL;
    }

    if(source[p->pos] == ')') {
      p->pos++;
      break;
    }
    
    p->pos++;
  }

  return list;
}

Obj *parse(GC *gc, const char *source) {
  Parser* p = parser_make();
  int source_len = strlen(source);
  int parens = 0;

  Obj *forms = gc_make_cons(gc, NULL, NULL);
  Obj *last_form = forms;
  
  while(p->pos < source_len) {
    Obj *form = parse_form(gc, p, source);
    if(form) {
      last_form->car = form;
      last_form->cdr = gc_make_cons(gc, NULL, NULL);
      last_form = last_form->cdr;
    }

    p->pos++;
  }

  return forms;
}
