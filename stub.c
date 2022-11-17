#include <stdio.h>

int foo() { printf("foo called.\n"); return 10; }
int bar(int a, int b) { printf("bar called: %d\n", a + b); return a + b; }
