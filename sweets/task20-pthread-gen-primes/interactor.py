#!/usr/bin/python3

import sys


def is_prime(n: int) -> bool:
    if n < 2:
        return False
    if n == 2:
        return True
    if n == 3:
        return True
    for i in range(2, int(n**0.5)+1):
        if n % i == 0:
            return False
    return True


# args is input output correct pid inf
with open(sys.argv[1]) as f:
    base, count = map(int, f.readline().split())
    print(base, count)

with open(sys.argv[2], 'w') as f:
    pass

numbers = [int(input()) for i in range(count)]

for i in range(len(numbers) - 1):
    if numbers[i] >= numbers[i+1]:
        raise RuntimeError("non-increasing sequence")

for n in numbers:
    if not is_prime(n):
        raise RuntimeError(n, "is not prime")

