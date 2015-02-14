#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#include "GC.h"
#include "Obj.h"
#include "Parser.h"
#include "Repl.h"
#include "Runtime.h"
#include "Tests.h"

void tests() {
  //test_gc();
  //test_printing();
  //test_parsing();
  //test_runtime();
  //test_local_environments();
  //test_str_allocs();
}

int main()
{
  //tests();
  repl();
}

