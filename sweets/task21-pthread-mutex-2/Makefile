CC = gcc
CFLAGS = -Wall -O2 -Werror -std=gnu11 -pthread

nejudge-update:
	git submodule update --init
	cd nejudge && ./update.bash

export EJUDGE_REAL_TIME_LIMIT_MS = 4000
export EJ_BAN_BY_REGEX_REQ_STACK_SIZE = pthread_attr_setstacksize;;Задайте размер стека
export EJ_BAN_BY_REGEX_REQ_PAGE_GUARD_SIZE = pthread_attr_setguardsize;;Задайте размер page guard

solution : solution.c
	$(CC) $(CFLAGS) @wrappers.cfg solution.c wrappers.c -o $@

test: solution nejudge-update
	python3 nejudge/test.py

clean :
	rm -f solution
