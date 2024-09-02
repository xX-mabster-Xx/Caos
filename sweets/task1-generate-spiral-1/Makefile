CC = gcc
CFLAGS = -O2 -Wall -Werror -std=gnu11 -Wl,-wrap=main

export EJ_BAN_BY_REGEX_BAN_ALLOCATIONS=\b(c|m|re)alloc;;В этой задаче не нужны дополнительные аллокации. Используйте mmap.

solution: solution.c wrappers.c

test: solution
	python3 test.py --output-file=output.dat

style: solution
	python3 test.py --output-file=output.dat --fix-style
