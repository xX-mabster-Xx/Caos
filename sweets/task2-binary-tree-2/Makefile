CC = gcc
CFLAGS = -g -m32 -O2 -Wall -Werror -std=gnu11

export EJ_BAN_BY_REGEX_REQ_MMAP=\bmmap\(;;Используйте mmap
export EJ_BAN_BY_REGEX_BAN_READ=\b[pf]?read\b;;Не используйте read и аналогичные функции
export EJ_BAN_BY_REGEX_BAN_READ=\b(f?getc|getchar)\b;;Не используйте функции для чтения из файлов

solution: solution.c

test: solution
	python3 test.py

style: solution
	python3 test.py --fix-style
