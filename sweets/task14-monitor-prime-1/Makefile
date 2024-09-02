CC = gcc
CFLAGS = -O2 -Wall -Werror -m32 -std=gnu11
LDLIBS = -lm
ICFLAGS = -Wall -Werror -std=gnu11 -m32 -O2

solution : solution.c

interactor : interactor.c checker2.h
	$(CC) $(ICFLAGS) -I$(EJUDGE_INCLUDE) -L$(EJUDGE_LIB) $^ -o $@ -lm

clean :
	-rm -f solution interactor

test: solution interactor
	python3 test.py --interactor './interactor'

style: solution interactor
	python3 test.py --interactor './interactor' --fix-style
