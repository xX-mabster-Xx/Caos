CC = gcc
CFLAGS = -g -O2 -Wall -Werror -std=gnu11

export EJ_BAN_BY_REGEX_REQ_REUSE=SO_REUSEADDR

solution : solution.c

clean :
	rm -f solution

test: solution interactor.py
	python3 test.py --run-cmd './solution' --interactor './interactor.py' --checker=ignore

style: solution interactor.py
	python3 test.py --run-cmd './solution' --interactor './interactor.py' --checker=ignore --fix-style
