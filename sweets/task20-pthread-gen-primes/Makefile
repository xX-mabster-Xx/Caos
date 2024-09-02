CC = gcc
CFLAGS = -Wall -O2 -Werror -std=gnu11 -pthread -Wl,-wrap=pthread_cond_wait -Wl,-wrap=main ${EJUDGE_ADD_FLAGS}
LDLIBS = -lm

export EJ_BAN_BY_REGEX_BAN_TIMEDWAIT=\bpthread_cond_timedwait;;Используйте pthread_cond_wait.
export EJ_BAN_BY_REGEX_BAN_ATOMICS=\b_Atomic\b|\batomic_\w+;;В этой задаче не нужны атомарные переменные.
export EJ_BAN_BY_REGEX_BAN_VOLATILE=\bvolatile\b;;volatile не нужно использовать в многопоточных программах, это либо излишне, либо ошибочно.

nejudge-update:
	git submodule update --init
	cd nejudge && ./update.bash

solution : solution.c wrappers.c

test: nejudge-update
	python3 nejudge/test.py --interactor ./interactor.py --run-cmd 'make -s run'

clean:
	rm -rf solution

run: clean solution
	./solution
