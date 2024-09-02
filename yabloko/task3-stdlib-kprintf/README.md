Напишите функцию `kprintf` со следующей сигнатурой:
```c
void kprintf(const char* format, ...);
```

Функция должна работать аналогично `printf` из стандартной библиотеки
языка Си, но с крайне ограниченным набором спецификаторов формата:

1) `%u` — вывести беззнаковый `int` в десятичной записи;
2) `%x` — вывести `int` в шестнадцатеричной записи
(цифры `a-f` в нижнем регистре, без префикса `0x`).

Вывод должен производиться на последовательный порт функцией `uartputc`
(можно и на экран, функцией `vga_putc` из прошлой задачи, но
тестирующая система этого не проверяет).

Функция `kprintf` принимает произвольное количество аргументов
(запись `...` в сигнатуре). Чтобы достать их из стека,
почитайте `man stdarg` и воспользуйтесь описанными там макросами.

Не используйте динамическую память. (Поскольку стандартная библиотека языка Си
вам недоступна, и функции `malloc`, `free` и прочие
взять негде, это требование можно было бы и не писать.)

```c
kprintf("year: %u", 1984);
// year: 1984

kprintf("%u, %u, %u, %u, and %u", 3, 14, 15, 92, 6);
// 3, 14, 15, 92, and 6

int arr[7]; kprintf("sizeof(arr) == 0x%x", sizeof(arr));
// sizeof(arr) == 0x1c
```