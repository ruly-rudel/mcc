#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./mcc "$input" > tmp.s
  gcc -o tmp tmp.s stub.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 21 'int main(){return 5+20-4;}'
assert 41 'int main(){return  12 + 34 - 5 ;}'
assert 47 'int main(){return 5+6*7;}'
assert 15 'int main(){return 5*(9-6);}'
assert 4  'int main(){return (3+5)/2;}'
assert 10 'int main(){return -10+20;}'
assert 30 'int main(){return +10++20;}'
assert 10 'int main(){return +20+-10;}'
assert 1 'int main(){return 10==10;}'
assert 0 'int main(){return 10==4;}'
assert 1 'int main(){return 10<=10;}'
assert 0 'int main(){return 10<10;}'
assert 1 'int main(){return 10>=10;}'
assert 0 'int main(){return 10>10;}'
assert 3 'int main(){int a;a=3;return a;}'
assert 3 'int main(){int a;int b;int c;a=1;b=2;c=a+b;return c;}'
assert 6 'int main(){int ag;int re;int df;ag=3;re=2;df=ag*re;return df;}'
assert 28 'int main(){int ag;int af;int ae;ag=4;af=7;ae=ag*af;return ae;}'
assert 5 'int main(){return 5;}'
assert 14 'int main(){int a; int b; a = 3; b = 5 * 6 - 8; return a + b / 2;}'
assert 10 'int main(){if ( 1 ) return 10 ;}'
assert 12 'int main(){if ( 3 == 1 + 2 ) return (6 * 2) ;}'
assert 11 'int main(){int ag;int af;ag=4;af=7;if(ag + 3 == af) return ag + af;}'
assert 20 'int main(){if ( 2 == 3 ) return 10 ; return 20; }'
assert 3  'int main(){if ( 1 ) if ( 0 ) return 2; else return 3; else return 4; 8;}'
assert 32 'int main(){int a; int b; a = 5; b = 2; while( a = a - 1 ) b = b * 2; return b;}'
assert 55 'int main(){int i; int j; j = 0; for(i = 1; i <= 10; i = i + 1) j = j + i; return j;}'
assert 55 'int main(){int i; int j; i = 0; j = 0; for( ; i <= 10; i = i + 1) j = j + i; return j;}'
assert 11 'int main(){int i; int j; i = 0; for( ; i <= 10; ) i = i + 1; return i;}'
assert 11 'int main(){int i; for(i = 0 ; i <= 10; i = i + 1) ; return i;}'
assert 10 'int main(){{ return 10; }}'
assert 20 'int main(){{ 10; return 20; }}'
assert 7  'int main(){int i; for(i = 0; i < 10; i = i + 1) { if (i == 5) return i + 2; }}'
assert 10 'int main(){return foo();}'
assert 3  'int main(){return bar(1, 2);}'
assert 3  'int main(){int a; a=1; return bar(a, 2);}'
assert 20 'int baz(){ return 20; } int main(){ return baz(); }'
assert 30 'int baz(){ int a; int b; a = 10; b = 20; return a + b; } int main() { return baz();}'
assert 10 'int baz(int a, int b){ return b - a; } int main() { return baz(10, 20);}'
assert 30 'int baz(){ int a; int b; a = 10; b = 20; return a + b; } int main() { int a; a = 50; return baz();}'
assert 10 'int baz(int a, int b){ return b - a; } int main() { int a; a = 50; return baz(10, 20);}'
assert 16 'int baz(int n){ if (n == 0) return 1; else return baz(n - 1) * 2; } int main() { return baz(4);}'
assert 34 'int fib(int n){ if (n == 1) return 1; else if (n == 2) return 1; else return fib(n - 1) + fib(n - 2); } int main() { return fib(9);}'
assert 3  'int main(){int x; int *y; x = 3; y = &x; return *y;}'
assert 3  'int main(){int x; int *y; y = &x; *y = 3; return x;}'
assert 4  'int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; return *q;}'
assert 8  'int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 3; return *q;}'
assert 7  'int main(){int *p; alloc4(&p, 1, 2, 4, 8); *(p + 2) = 7; int *q; q = p + 2; return *q;}'
assert 4  'int main() { int **p; int a; int b; int c; int d; a = 1; b = 2; c = 4; d = 8; palloc4(&p, &d, &c, &b, &a); int **q; q = p + 1; return **q; }'
assert 2  'int main() { int **p; int a; int b; int c; int d; a = 1; b = 2; c = 4; d = 8; palloc4(&p, &d, &c, &b, &a); int **q; q = p + 2; return **q; }'
assert 2  'int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; q = q - 1; return *q;}'
assert 4  'int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 3; q = q - 1; return *q;}'
assert 8  'int main() { int **p; int a; int b; int c; int d; a = 1; b = 2; c = 4; d = 8; palloc4(&p, &d, &c, &b, &a); int **q; q = p + 1; q = q - 1; return **q; }'
assert 4  'int main() { int **p; int a; int b; int c; int d; a = 1; b = 2; c = 4; d = 8; palloc4(&p, &d, &c, &b, &a); int **q; q = p + 2; q = q - 1; return **q; }'
assert 2  'int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 2; return q - p;}'
assert 3  'int main(){int *p; alloc4(&p, 1, 2, 4, 8); int *q; q = p + 3; return q - p;}'
assert 1  'int main() { int **p; int a; int b; int c; int d; a = 1; b = 2; c = 4; d = 8; palloc4(&p, &d, &c, &b, &a); int **q; q = p + 1; return q - p; }'
assert 2  'int main() { int **p; int a; int b; int c; int d; a = 1; b = 2; c = 4; d = 8; palloc4(&p, &d, &c, &b, &a); int **q; q = p + 2; return q - p; }'
assert 4  'int main() { int x; int *y; return sizeof(x); }'
assert 8  'int main() { int x; int *y; return sizeof(y); }'
assert 8  'int main() { int x; int *y; return sizeof(&x); }'
assert 4  'int main() { int x; int *y; return sizeof(*y); }'
assert 4  'int main() { int x; int *y; return sizeof(x + 3); }'
assert 8  'int main() { int x; int *y; return sizeof(y + 3); }'
assert 4  'int main() { int x; int *y; return sizeof(3); }'
assert 4  'int main() { int x; int *y; return sizeof(sizeof(y)); }'
assert 4  'int *baz(int* n){ return n + 2; } int main() { int *p; alloc4(&p, 1, 2, 4, 8); return *baz(p);}'
assert 1  'int main() { int a[2]; *a = 1; return *a; }'
assert 2  'int main() { int a[2]; *(a + 1) = 2; *a = 1; return *(a + 1); }'
assert 1  'int main() { int a[2]; *a = 1; *(a + 1) = 2; return *a; }'
assert 3  'int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }'

echo OK
