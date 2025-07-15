#include "terminal.h"
#include "port.h"

static uint8_t cursor_x { 0 };
static uint8_t cursor_y { 0 };
static uint16_t* video_memory { VGA_VIDEO_MEMORY };
static uint8_t current_color { VGA_COLOR_WHITE_ON_BLACK };

static Port8Bit cursor_command_port(VGA_CURSOR_COMMAND_PORT);
static Port8Bit cursor_data_port(VGA_CURSOR_DATA_PORT);

void initialize_terminal()
{
    cursor_x = 0;
    cursor_y = 0;
    current_color = VGA_COLOR_WHITE_ON_BLACK;
    video_memory = VGA_VIDEO_MEMORY;
    clear_screen();
}

static inline void write_cursor_register(uint8_t reg, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a" (reg), "Nd" ((uint16_t)0x3D4));
    __asm__ volatile("outb %0, %1" : : "a" (value), "Nd" ((uint16_t)0x3D5));
    // Using non-virtual port methods to eliminate function call overhead
    // cursor_command_port.write_direct(reg);
    // cursor_data_port.write_direct(value);

    // Your functions don't work for me kevin :(
}

void update_hardware_cursor()
{
    uint16_t cursor_position { cursor_y * VGA_SCREEN_WIDTH + cursor_x };
    
    write_cursor_register(0x0F, cursor_position & 0xFF);
    write_cursor_register(0x0E, (cursor_position >> 8) & 0xFF);
}

void clear_screen()
{
    for (int y = 0; y < VGA_SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < VGA_SCREEN_WIDTH; ++x) {
            video_memory[VGA_SCREEN_WIDTH * y + x] = (current_color << 8) | ' ';
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    update_hardware_cursor();
}

void scroll_screen()
{
    // Move all lines up by one
    for (int y = 0; y < VGA_SCREEN_HEIGHT - 1; ++y) {
        for (int x = 0; x < VGA_SCREEN_WIDTH; ++x) {
            video_memory[VGA_SCREEN_WIDTH * y + x] = video_memory[VGA_SCREEN_WIDTH * (y + 1) + x];
        }
    }
    
    // Clears the bottom line.
    for (int x = 0; x < VGA_SCREEN_WIDTH; ++x) {
        video_memory[VGA_SCREEN_WIDTH * (VGA_SCREEN_HEIGHT - 1) + x] = (current_color << 8) | ' ';
    }
    
    // Keeps the cursor at the bottom of the screen.
    cursor_y = VGA_SCREEN_HEIGHT - 1;
}

void get_cursor_position(uint8_t* x, uint8_t* y)
{
    if (x) {
        *x = cursor_x;
    }

    if (y) {
        *y = cursor_y;
    }
}

void set_cursor_position(uint8_t x, uint8_t y)
{
    if (x < VGA_SCREEN_WIDTH && y < VGA_SCREEN_HEIGHT) {
        cursor_x = x;
        cursor_y = y;
        update_hardware_cursor();
    }
}

void set_text_color(uint8_t color)
{
    current_color = color;
}

uint8_t get_text_color()
{
    return current_color;
}

void put_char_colored(char character, uint8_t color)
{
    switch(character)
    {
        case '\n':
            cursor_x = 0;
            cursor_y++;
            break;
            
        case '\t':
            cursor_x = (cursor_x + 4) & ~3;
            break;
            
        case '\r':
            cursor_x = 0;
            break;
            
        case '\b':
            if (cursor_x > 0) {
                cursor_x--;
            }
            break;
            
        default:
            if (character >= ' ' && character <= '~')  {
                video_memory[VGA_SCREEN_WIDTH * cursor_y + cursor_x] = (color << 8) | character;
                cursor_x++;
            }
            break;
    }

    if (cursor_x >= VGA_SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    if (cursor_y >= VGA_SCREEN_HEIGHT) {
        scroll_screen();
    }
    
    update_hardware_cursor();
}

void put_char(char character)
{
    put_char_colored(character, current_color);
}

void handle_backspace()
{
    if (cursor_x > 0) {
        cursor_x--;
    } else if (cursor_y > 0) {
        cursor_y--;
        cursor_x = VGA_SCREEN_WIDTH - 1;
        
        // Find the actual end of text on previous line.
        while (cursor_x > 0) {
            uint16_t cell { video_memory[VGA_SCREEN_WIDTH * cursor_y + cursor_x] };
            if ((cell & 0xFF) != ' ') {
                break;
            }
            cursor_x--;
        }
    }
    
    video_memory[VGA_SCREEN_WIDTH * cursor_y + cursor_x] = (current_color << 8) | ' ';
    
    update_hardware_cursor();
}

// We will move this to a std directory.
void printf(const char* str)
{
    for (int i = 0; str[i] != '\0'; ++i) {
        put_char(str[i]);
    }
}

void printf_colored(const char* str, uint8_t color)
{
    for (int i = 0; str[i] != '\0'; ++i) {
        put_char_colored(str[i], color);
    }
}
