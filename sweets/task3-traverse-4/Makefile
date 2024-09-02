CC = gcc
CFLAGS = -g -Wall -std=gnu11 -D_FILE_OFFSET_BITS=64 -Werror

solution: solution.c

static: static.tgz
	rm -rf static || true
	tar -xzvf static.tgz

test: static solution
	python3 test.py --checker=sorted-lines --may-fail-local tests/003.dat tests/006.dat --user ejexec

style: static solution
	python3 test.py --checker=sorted-lines --may-fail-local tests/003.dat tests/006.dat --user ejexec --fix-style

clean:
	rm -rf solution static
