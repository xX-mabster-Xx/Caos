CC = gcc
LD = gcc
CFLAGS = -O2 -Wall -Werror -Wformat-security -Wignored-qualifiers -Winit-self -Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith -Wtype-limits -Wempty-body -Wlogical-op -Wstrict-prototypes -Wold-style-declaration -Wold-style-definition -Wmissing-parameter-type -Wmissing-field-initializers -Wnested-externs -Wno-pointer-sign -std=gnu99
LDFLAGS = -O2 -std=gnu99 -lm

export TZ=Europe/Moscow

solution : solution.c

test: solution
	python3 test.py

style: solution
	python3 test.py --fix-style
