CC = gcc
CFLAGS = -Wall -Werror -g -std=gnu11 -m32

solution : solution.c

test: solution
	python3 test.py --run-cmd="python3 checker.py input.txt ./solution"

style: solution
	python3 test.py --run-cmd="python3 checker.py input.txt ./solution" --fix-style
