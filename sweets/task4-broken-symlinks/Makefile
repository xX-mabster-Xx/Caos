CC = gcc
CFLAGS = -g -O2 -Wall -Werror -std=gnu11 -Wl,--wrap=stat -Wl,--wrap=lstat -Wl,--wrap=fstat -Wl,--wrap=fstatat -Wl,--wrap=__xstat -Wl,--wrap=__lxstat -Wl,--wrap=__fxstat -Wl,--wrap=__fxstatat

solution: solution.c stat_wrappers.c

static:
	rm -rf static
	mkdir static
	ln -s . static/good
	ln -s /non/existing static/broken

test: solution static
	python3 test.py

style: solution static
	python3 test.py --fix-style

clean:
	rm -rf static solution
