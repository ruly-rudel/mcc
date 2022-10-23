#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./mcc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4  '(3+5)/2;'
assert 10 '-10+20;'
assert 30 '+10++20;'
assert 10 '+20+-10;'
assert 1 '10==10;'
assert 0 '10==4;'
assert 1 '10<=10;'
assert 0 '10<10;'
assert 1 '10>=10;'
assert 0 '10>10;'
assert 3 'a=1;b=2;c=a+b;'
assert 6 'ag=3;re=2;df=ag*re;'
assert 28 'ag=4;af=7;ae=ag*af;'

echo OK
