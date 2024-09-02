CXX = g++
CXXFLAGS = -Wall -O2 -Werror -pthread -std=gnu++17

export EJUDGE_REAL_TIME_LIMIT_MS = 3000
export EPS=1e-2
export EJ_BAN_BY_REGEX_REQ_ASYNC=std::async

nejudge-update:
	git submodule update --init
	cd nejudge && ./update.bash

solution : solution.cpp

test: solution nejudge-update
	python3 nejudge/test.py --checker cmp-double

clean :
	rm -f solution
