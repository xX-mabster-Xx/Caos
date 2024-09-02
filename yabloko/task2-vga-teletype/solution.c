#include "drivers/port.h"
#include "drivers/vga.h"

static unsigned cursor;

enum { END_OF_DISPLAY = ROWS * COLS, BEGIN_OF_LAST_LINE = ROWS * COLS - COLS };

void vga_set_cursor(unsigned offset) {
  port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_HIGH);
  port_byte_out(VGA_DATA_REGISTER, (unsigned short)(offset >> 8));
  port_byte_out(VGA_CTRL_REGISTER, VGA_OFFSET_LOW);
  port_byte_out(VGA_DATA_REGISTER, (unsigned short)(offset % (1 << 8)));
}

void scroll_display() {
  for (int offset = 0; offset < BEGIN_OF_LAST_LINE; ++offset) {
    video_memory[2 * offset] = video_memory[2 * (offset + COLS)];
  }
  for (int offset = BEGIN_OF_LAST_LINE; offset < END_OF_DISPLAY; ++offset) {
    vga_set_char(offset, ' ');
  }
}

void vga_putc(char c) {
  if (c == '\n') {
    cursor = ((cursor / COLS) + 1) * COLS;
  } else {
    vga_set_char(cursor, c);
    cursor++;
  }
  if (cursor >= END_OF_DISPLAY) {
    scroll_display();
    cursor = BEGIN_OF_LAST_LINE;
  }
  vga_set_cursor(cursor);
}
