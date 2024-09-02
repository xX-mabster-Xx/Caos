CC = gcc
CFLAGS = -Wall -O2 -Werror -std=gnu11 -pthread

export EJUDGE_REAL_TIME_LIMIT_MS = 2000
export EJ_BAN_BY_REGEX_REQ_ = (eventfd|pipe);;Для синхронизации нитей используйте eventfd или pipe

nejudge-update:
	git submodule update --init
	cd nejudge && ./update.bash

solution : solution.c wrappers.c wrappers.cfg
	$(CC) $(CFLAGS) @wrappers.cfg solution.c wrappers.c -o $@

test: solution nejudge-update
	python3 nejudge/test.py

clean :
	rm -f solution
