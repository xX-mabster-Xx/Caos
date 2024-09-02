CC = gcc
CFLAGS = -Wall -Werror -std=gnu11 -m32

export EJ_BAN_BY_REGEX_BAN_XCHG_IN_UNLOCK=\bspin_unlock:.*xchg\b;;Для разблокировки не нужен ни exchange, ни CAS. Возвращаемое значение не используется (void), линейный порядок операций (seq_cst), предоставляемый locked-инструкциями, тоже не нужен.
export EJ_BAN_BY_REGEX_BAN_LOCK_PREFIX_IN_UNLOCK=\bspin_unlock:.*\block\b;;Для разблокировки не нужны инструкции с lock-префиксом. На x86 mov с правильным выравниванием уже предоставляет необходимые гарантии (атомарность записи и release-семантику).
export EJ_BAN_BY_REGEX_DOTALL=1

nejudge-update:
	git submodule update --init
	cd nejudge && ./update.bash

solution: solution.S main.c
	$(CC) $(CFLAGS) $^ -o $@

test: solution nejudge-update
	python3 nejudge/test.py

clean:
	rm -rf solution
