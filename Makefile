CFLAGS=-std=c11 -g -static -Wall -O0

mcc: mcc.c

test: mcc
	./test.sh

clean:
	rm -f mcc *.o *~ tmp*

.PHONY: test clean
