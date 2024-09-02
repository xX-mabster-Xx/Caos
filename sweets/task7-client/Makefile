CXX = g++
CXXFLAGS = -Wall -Werror -g -std=gnu++14
CC = gcc
CFLAGS = -Wall -Werror -g -std=gnu11

interactor : server.cpp
	${CXX} ${CXXFLAGS} server.cpp -o interactor

solution : solution.c

clean :
	rm -f interactor solution

test: solution interactor
	python3 test.py --run-cmd 'bash -c "timeout 3 ./interactor 23456 & sleep 1 && ./solution params"'

style: solution interactor
	python3 test.py --run-cmd 'bash -c "timeout 3 ./interactor 23456 & sleep 1 && ./solution params"' --fix-style
