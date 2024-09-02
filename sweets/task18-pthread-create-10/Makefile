CC = gcc
CFLAGS = -Wall -Werror -O2 -std=gnu11 -pthread

export EJ_BAN_BY_REGEX_BAN_FFLUSH=\bfflush\b;;Не используйте fflush. Правильное решение будет корректно работать и без сброса буфера (подумайте, почему).
export EJ_BAN_BY_REGEX_BAN_SYNCHRONIZATION_PRIMITIVES=\bpthread_(mutex|cond);;В этой задаче не нужны примитивы синхронизации. Используйте pthread_join.
export EJ_BAN_BY_REGEX_BAN_ATOMICS=\b_Atomic\b|\batomic_\w+;;В этой задаче не нужны атомарные переменные.
export EJ_BAN_BY_REGEX_BAN_VOLATILE=\bvolatile\b;;volatile не нужно использовать в многопоточных программах, это либо излишне, либо ошибочно.
export EJ_BAN_BY_REGEX_BAN_SEMAPHORE=\bsem_t\b;;В этой задаче не нужен семафор. Используйте pthread_create и pthread_join.
export EJ_RETESTS_AMOUNT=10

nejudge-update:
	git submodule update --init
	cd nejudge && ./update.bash

solution : solution.c wrappers.c wrappers.cfg
	$(CC) $(CFLAGS) @wrappers.cfg solution.c wrappers.c -o $@

test: solution nejudge-update
	python3 nejudge/test.py

clean:
	rm -rf solution
