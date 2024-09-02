CC = gcc
CFLAGS = -g -O2 -Wall -Werror -std=gnu11

solution : solution.c wrappers.c wrappers.cfg
	$(CC) $(CFLAGS) @wrappers.cfg solution.c wrappers.c -o $@

interactor : interactor.c checker2.h

clean :
	-rm solution interactor

test: solution interactor
	python3 test.py --interactor './interactor'

style: solution interactor
	python3 test.py --interactor './interactor' --fix-style
