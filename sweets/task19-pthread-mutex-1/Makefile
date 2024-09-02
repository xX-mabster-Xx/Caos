CC = gcc
CFLAGS = -Wall -O2 -Werror -std=gnu11 -pthread

nejudge-update:
	git submodule update --init
	cd nejudge && ./update.bash

solution : solution.c wrappers.c wrappers.cfg
	$(CC) $(CFLAGS) @wrappers.cfg solution.c wrappers.c -o $@

export EJ_BAN_BY_REGEX_BAN_ATOMICS=\b_Atomic\b|\batomic_\\w+;;В этой задаче не нужны атомарные переменные.
export EJ_BAN_BY_REGEX_BAN_VOLATILE=\bvolatile\b;;volatile не нужно использовать в многопоточных программах, это либо излишне, либо ошибочно.

test: solution nejudge-update
	python3 nejudge/test.py

clean :
	rm -rf solution
