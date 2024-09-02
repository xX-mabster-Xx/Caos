CC = gcc
CFLAGS = -O2 -Wall -Werror -std=gnu11

solution : solution.c wrappers.c wrappers.cfg
	$(CC) $(CFLAGS) @wrappers.cfg solution.c wrappers.c -o $@

run_interactor: run_interactor.c
interactor: interactor.c

test: interactor run_interactor solution
	python3 test.py --output-file=output.txt --run-cmd="./run_interactor ./interactor input.txt output.txt any ; ./solution"
