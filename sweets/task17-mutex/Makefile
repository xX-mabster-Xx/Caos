CC = gcc
CFLAGS = -Wall -Werror -std=gnu11 -m32 -O2

export EJ_BAN_BY_REGEX_BAN_SYSCALL=\bsyscall\(;;Используйте определённые в системе функции futex_wait и futex_wake вместо сырых системных вызовов.

nejudge-update:
	git submodule update --init
	cd nejudge && ./update.bash

solution: main.c solution.c
	$(CC) $(CFLAGS) main.c -o $@

test: solution nejudge-update
	python3 nejudge/test.py

clean:
	rm -rf solution
