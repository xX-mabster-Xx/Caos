CC = gcc
CFLAGS = -g -Wall -Werror -std=gnu11 -m32
ICFLAGS = -Wall -Werror -std=gnu11 -m32 -O2

EJ_BAN_BY_REGEX_REQ_SETITIMER=\\bsetitimer\\b

solution : solution.c

interactor : interactor.c checker2.h
	$(CC) $(ICFLAGS) -I$(EJUDGE_INCLUDE) -L$(EJUDGE_LIB) $^ -o $@

clean :
	-rm -f solution interactor solution_nocheck

test: solution interactor
	python3 test.py --interactor './interactor'

style: solution interactor
	python3 test.py --interactor './interactor' --fix-style
