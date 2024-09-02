CC = gcc
CFLAGS = -Wall -Werror -std=gnu11 -g

export EJ_BAN_BY_REGEX_REQ_REUSE=SO_REUSEADDR

solution : solution.c

interactor : interactor.c checker2.h testinfo.h

clean :
	rm -f interactor solution

test: solution interactor
	python3 test.py --run-cmd 'timeout 5 ./solution' --interactor './interactor'

style: solution interactor
	python3 test.py --run-cmd 'timeout 5 ./solution' --interactor './interactor' --fix-style
