#ifndef TERMINAL_H
#define TERMINAL_H

#include "types.h"
#include "port.h"

#define VGA_VIDEO_MEMORY ((uint16_t*)0xb8000)
#define VGA_SCREEN_WIDTH 80
#define VGA_SCREEN_HEIGHT 25

#define VGA_CURSOR_COMMAND_PORT 0x3D4
#define VGA_CURSOR_DATA_PORT 0x3D5

#define VGA_CURSOR_LOCATION_HIGH 0x0E
#define VGA_CURSOR_LOCATION_LOW 0x0F

#define VGA_COLOR_BLACK_ON_BLACK 0x00
#define VGA_COLOR_BLUE_ON_BLACK 0x01
#define VGA_COLOR_GREEN_ON_BLACK 0x02
#define VGA_COLOR_CYAN_ON_BLACK 0x03
#define VGA_COLOR_RED_ON_BLACK 0x04
#define VGA_COLOR_MAGENTA_ON_BLACK 0x05
#define VGA_COLOR_BROWN_ON_BLACK 0x06
#define VGA_COLOR_LIGHT_GRAY_ON_BLACK 0x07
#define VGA_COLOR_DARK_GRAY_ON_BLACK 0x08
#define VGA_COLOR_LIGHT_BLUE_ON_BLACK 0x09
#define VGA_COLOR_LIGHT_GREEN_ON_BLACK 0x0A
#define VGA_COLOR_LIGHT_CYAN_ON_BLACK 0x0B
#define VGA_COLOR_LIGHT_RED_ON_BLACK 0x0C
#define VGA_COLOR_LIGHT_MAGENTA_ON_BLACK 0x0D
#define VGA_COLOR_YELLOW_ON_BLACK 0x0E
#define VGA_COLOR_WHITE_ON_BLACK 0x0F

#define VGA_COLOR_BLACK_ON_WHITE 0xF0
#define VGA_COLOR_WHITE_ON_BLUE 0x1F
#define VGA_COLOR_YELLOW_ON_BLUE 0x1E
#define VGA_COLOR_WHITE_ON_RED 0x4F
#define VGA_COLOR_YELLOW_ON_RED 0x4E
#define VGA_COLOR_BLACK_ON_YELLOW 0xE0
#define VGA_COLOR_BLUE_ON_YELLOW 0xE1
#define VGA_COLOR_RED_ON_YELLOW 0xE4
#define VGA_COLOR_BLACK_ON_GREEN 0x20
#define VGA_COLOR_WHITE_ON_GREEN 0x2F
#define VGA_COLOR_BLACK_ON_CYAN 0x30
#define VGA_COLOR_WHITE_ON_CYAN 0x3F
#define VGA_COLOR_BLACK_ON_MAGENTA 0x50
#define VGA_COLOR_WHITE_ON_MAGENTA 0x5F

#define VGA_COLOR_WHITE VGA_COLOR_WHITE_ON_BLACK
#define VGA_COLOR_GREEN VGA_COLOR_GREEN_ON_BLACK
#define VGA_COLOR_RED VGA_COLOR_RED_ON_BLACK
#define VGA_COLOR_YELLOW VGA_COLOR_YELLOW_ON_BLACK
#define VGA_COLOR_CYAN VGA_COLOR_CYAN_ON_BLACK
#define VGA_COLOR_MAGENTA VGA_COLOR_MAGENTA_ON_BLACK

void initialize_terminal();
void clear_screen();
void scroll_screen();

void update_hardware_cursor();
void get_cursor_position(uint8_t* x, uint8_t* y);
void set_cursor_position(uint8_t x, uint8_t y);

void put_char(char character);
void put_char_colored(char character, uint8_t color);
void printf(const char* str);
void printf_colored(const char* str, uint8_t color);

void handle_backspace();

void set_text_color(uint8_t color);
uint8_t get_text_color();

#endif
