#include <stdio.h>
#include <stdlib.h>

int foo() { printf("foo called.\n"); return 10; }
int bar(int a, int b) { printf("bar called: %d\n", a + b); return a + b; }
int* alloc4(int **p, int a, int b, int c, int d) { *p = calloc(4, sizeof(int)); (*p)[0] = a; (*p)[1] = b; (*p)[2] = c; (*p)[3] = d; return *p; }
int** palloc4(int ***p, int *a, int *b, int *c, int *d) { *p = calloc(4, sizeof(int*)); (*p)[0] = a; (*p)[1] = b; (*p)[2] = c; (*p)[3] = d; return *p; }
