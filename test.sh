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

assert 21 'main(){5+20-4;}'
assert 41 'main(){ 12 + 34 - 5 ;}'
assert 47 'main(){5+6*7;}'
assert 15 'main(){5*(9-6);}'
assert 4  'main(){(3+5)/2;}'
assert 10 'main(){-10+20;}'
assert 30 'main(){+10++20;}'
assert 10 'main(){+20+-10;}'
assert 1 'main(){10==10;}'
assert 0 'main(){10==4;}'
assert 1 'main(){10<=10;}'
assert 0 'main(){10<10;}'
assert 1 'main(){10>=10;}'
assert 0 'main(){10>10;}'
assert 3 'main(){a=1;b=2;c=a+b;}'
assert 6 'main(){ag=3;re=2;df=ag*re;}'
assert 28 'main(){ag=4;af=7;ae=ag*af;}'
assert 5 'main(){return 5;}'
assert 14 'main(){a = 3; b = 5 * 6 - 8; return a + b / 2;}'
assert 10 'main(){if ( 1 ) 10 ;}'
assert 12 'main(){if ( 3 == 1 + 2 ) (6 * 2) ;}'
assert 11 'main(){ag=4;af=7;if(ag + 3 == af) return ag + af;}'
assert 20 'main(){if ( 2 == 3 ) return 10 ; return 20; }'
assert 3  'main(){if ( 1 ) if ( 0 ) return 2; else return 3; else return 4; 8;}'
assert 32 'main(){a = 5; b = 2; while( a = a - 1 ) b = b * 2; return b;}'
assert 55 'main(){j = 0; for(i = 1; i <= 10; i = i + 1) j = j + i; return j;}'
assert 55 'main(){i = 0; j = 0; for( ; i <= 10; i = i + 1) j = j + i; return j;}'
assert 11 'main(){i = 0; for( ; i <= 10; ) i = i + 1; return i;}'
assert 11 'main(){for(i = 0 ; i <= 10; i = i + 1) ; return i;}'
assert 10 'main(){{ 10; }}'
assert 20 'main(){{ 10; 20; }}'
assert 7  'main(){for(i = 0; i < 10; i = i + 1) { if (i == 5) return i + 2; }}'
assert 10 'main(){foo();}'
assert 3  'main(){bar(1, 2);}'
assert 3  'main(){a=1;bar(a, 2);}'
assert 20 'baz(){ return 20; } main(){ return baz(); }'
assert 30 'baz(){ a = 10; return 10 + 20; } main() { return baz();}'

echo OK
