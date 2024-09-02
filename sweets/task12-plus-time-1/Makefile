CC = gcc
CFLAGS = -Wall -Werror -std=gnu11 -g -m32 -Wl,--wrap=time

export TZ=UTC

solution : solution.c time.c
	$(CC) $(CFLAGS) $^ -o$@

clean:
	-rm -f solution

test: solution
	python3 test.py

style: solution
	python3 test.py --fix-style
