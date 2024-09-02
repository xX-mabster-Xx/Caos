#include "drivers/vga.h"

enum { TABLE_SIZE = 16 };

static void print_char(char c) {
  static unsigned offset = 0;
  vga_set_char(offset, c);
  ++offset;
}

void complete_with_spaces(int n) {
  for (int col = 0; col < n; ++col) {
    print_char(' ');
  }
}

char int_to_x_char(int value) {
  if (value < 10) {
    return (char)('0' + value);
  }
  return (char)('a' + value - 10);
}

void show_vga_symbol_table(void) {
  print_char(' ');
  for (int row = 0; row < TABLE_SIZE; ++row) {
    print_char(' ');
    print_char(int_to_x_char(row));
  }
  complete_with_spaces(COLS - 2 * TABLE_SIZE - 1);

  for (int row = 0; row < TABLE_SIZE; ++row) {
    print_char(int_to_x_char(row));
    for (int col = 0; col < TABLE_SIZE; ++col) {
      print_char(' ');
      print_char((char)(row * TABLE_SIZE + col));
    }
    complete_with_spaces(COLS - 2 * TABLE_SIZE - 1);
  }
  for (int row = TABLE_SIZE; row < ROWS; ++row) {
    complete_with_spaces(COLS);
  }
}
