#include "keyboard.h"
#include "terminal.h"

KeyboardDriver::KeyboardDriver(InterruptManager* manager)
    : Driver(manager, 0x21), // Initialize as Driver for IRQ 1 (0x21 = hardware offset + 1).
      data_port(0x60),
      command_port(0x64),
      left_shift_pressed(false),
      right_shift_pressed(false),
      ctrl_pressed(false),
      alt_pressed(false),
      caps_lock_active(false),
      extended_key_next(false)
{
    initialize();
}

KeyboardDriver::~KeyboardDriver()
{
    deactivate();
}

void KeyboardDriver::initialize()
{
    // We check if the keyboard controller has data waiting (bit 0 status).
    while(command_port.read() & 0x1) {
        // If so, we read and discard the data to clear the buffer.
        data_port.read();
    }
    
    command_port.write(0xae);  // This command enables interrupts.
    command_port.write(0x20);  // This command gets the current controller configuration.
    uint8_t status = (data_port.read() | 1) & ~0x10; // Enable interrupts (bit 0), and disable mouse (clear bit 4).
    command_port.write(0x60);  // This command writes the new configuration.
    data_port.write(status);   // We write the modified configuration byte.
    data_port.write(0xf4);     // We send this directly to the keyboard to enable it.
}

void KeyboardDriver::activate()
{
    left_shift_pressed = false;
    right_shift_pressed = false;
    ctrl_pressed = false;
    alt_pressed = false;
    extended_key_next = false;
    
    // Re-enable keyboard
    data_port.write(0xf4);
}

void KeyboardDriver::deactivate()
{
    // Disable keyboard
    data_port.write(0xf5);
}

void KeyboardDriver::reset()
{
    left_shift_pressed = false;
    right_shift_pressed = false;
    ctrl_pressed = false;
    alt_pressed = false;
    caps_lock_active = false;
    extended_key_next = false;
    
    // Send reset command to keyboard
    data_port.write(0xff);
    
    initialize();
}

const char* KeyboardDriver::get_driver_name()
{
    return "PS/2 Keyboard Driver";
}

bool KeyboardDriver::is_shift_pressed()
{
    return left_shift_pressed || right_shift_pressed;
}

bool KeyboardDriver::should_capitalize(char c)
{
    bool shift_effect = is_shift_pressed();
    
    if (caps_lock_active && c >= 'a' && c <= 'z') {
        shift_effect = !shift_effect;
    }
    
    return shift_effect;
}

char KeyboardDriver::apply_shift(char c)
{
    if (c >= 'a' && c <= 'z' && should_capitalize(c)) {
        return c - 'a' + 'A';
    }
    
    if (is_shift_pressed()) {
        switch(c)
        {
            case '1': return '!';
            case '2': return '@';
            case '3': return '#';
            case '4': return '$';
            case '5': return '%';
            case '6': return '^';
            case '7': return '&';
            case '8': return '*';
            case '9': return '(';
            case '0': return ')';
            case '-': return '_';
            case '=': return '+';
            case '[': return '{';
            case ']': return '}';
            case '\\': return '|';
            case ';': return ':';
            case '\'': return '"';
            case '`': return '~';
            case ',': return '<';
            case '.': return '>';
            case '/': return '?';
        }
    }
    
    return c;
}

void KeyboardDriver::handle_special_key(uint8_t scan_code)
{
    switch(scan_code)
    {
        case Keyboard::KEY_F1:
            printf_colored("\n[F1] System Info: Simple OS v1.0\n", VGA_COLOR_YELLOW_ON_BLACK);
            break;
            
        case Keyboard::KEY_F2:
            printf_colored("\n[F2] Memory: 64MB segments active\n", VGA_COLOR_YELLOW_ON_BLACK);
            break;
            
        case Keyboard::KEY_F3:
            printf_colored("\n[F3] Interrupts: Active, PIC configured\n", VGA_COLOR_YELLOW_ON_BLACK);
            break;
            
        case Keyboard::KEY_F4:
            {
                uint8_t x, y;
                get_cursor_position(&x, &y);
                printf_colored("\n[F4] Cursor position: (", VGA_COLOR_YELLOW_ON_BLACK);
                put_char('0' + (x / 10));
                put_char('0' + (x % 10));
                printf_colored(", ", VGA_COLOR_YELLOW_ON_BLACK);
                put_char('0' + (y / 10));
                put_char('0' + (y % 10));
                printf_colored(")\n", VGA_COLOR_YELLOW_ON_BLACK);
            }
            break;
            
        case Keyboard::KEY_F5:
            printf_colored("\n[F5] Clearing screen...\n", VGA_COLOR_YELLOW_ON_BLACK);
            // Wait a moment and then clear the screen.
            for (volatile int i = 0; i < 1000000; ++i);
            clear_screen();
            printf_colored("Screen cleared! Type something...\n> ", VGA_COLOR_GREEN_ON_BLACK);
            break;
    }
}

void KeyboardDriver::handle_extended_key(uint8_t scan_code)
{
    switch(scan_code)
    {
        case Keyboard::KEY_ARROW_UP:
            printf_colored(" ↑ ", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
            break;
            
        case Keyboard::KEY_ARROW_DOWN:
            printf_colored(" ↓ ", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
            break;
            
        case Keyboard::KEY_ARROW_LEFT:
            printf_colored(" ← ", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
            break;
            
        case Keyboard::KEY_ARROW_RIGHT:
            printf_colored(" → ", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
            break;
            
        case Keyboard::KEY_DELETE:
            // For now, we just treat delete like backspace.
            handle_backspace();
            break;
            
        case Keyboard::KEY_HOME:
            {
                uint8_t x, y;
                get_cursor_position(&x, &y);
                set_cursor_position(0, y);
            }
            break;
            
        case Keyboard::KEY_END:
            {
                uint8_t x, y;
                get_cursor_position(&x, &y);
                set_cursor_position(VGA_SCREEN_WIDTH - 1, y);
            }
            break;
    }
}

uint32_t KeyboardDriver::handle_interrupt(uint32_t esp)
{
    uint8_t scan_code = data_port.read();
    
    if (scan_code == Keyboard::EXTENDED_KEY_PREFIX) {
        extended_key_next = true;
        return esp;
    }
    
    bool key_released = (scan_code & Keyboard::KEY_RELEASED) != 0;
    if (key_released) {
        scan_code &= ~Keyboard::KEY_RELEASED;
    }
    
    if (extended_key_next) {
        extended_key_next = false;
        if (!key_released) {
            handle_extended_key(scan_code);
        }
        return esp;
    }
    
    switch(scan_code)
    {
        case Keyboard::KEY_LEFT_SHIFT:
            left_shift_pressed = !key_released;
            return esp;
            
        case Keyboard::KEY_RIGHT_SHIFT:
            right_shift_pressed = !key_released;
            return esp;
            
        case Keyboard::KEY_LEFT_CTRL:
            ctrl_pressed = !key_released;
            return esp;
            
        case Keyboard::KEY_LEFT_ALT:
            alt_pressed = !key_released;
            return esp;
            
        case Keyboard::KEY_CAPS_LOCK:
            // Only toggle on key press, not release.
            if (!key_released) {
                caps_lock_active = !caps_lock_active;
            }
            return esp;
    }
    
    if (key_released) {
         return esp;
    }
    
    switch(scan_code)
    {
        case Keyboard::KEY_BACKSPACE:
            handle_backspace();
            break;
            
        case Keyboard::KEY_ENTER:
            put_char('\n');
            break;
            
        case Keyboard::KEY_SPACE:
            put_char(' ');
            break;
            
        case Keyboard::KEY_TAB:
            put_char('\t');
            break;
            
        case Keyboard::KEY_ESCAPE:
            printf_colored("\n[ESC pressed]\n", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
            break;
            
        case Keyboard::KEY_F1:
        case Keyboard::KEY_F2:
        case Keyboard::KEY_F3:
        case Keyboard::KEY_F4:
        case Keyboard::KEY_F5:
            handle_special_key(scan_code);
            break;

        default:
        {
            char character = 0;
            
            switch(scan_code)
            {
                // Number row
                case 0x02: character = '1'; break;
                case 0x03: character = '2'; break;
                case 0x04: character = '3'; break;
                case 0x05: character = '4'; break;
                case 0x06: character = '5'; break;
                case 0x07: character = '6'; break;
                case 0x08: character = '7'; break;
                case 0x09: character = '8'; break;
                case 0x0A: character = '9'; break;
                case 0x0B: character = '0'; break;
                case 0x0C: character = '-'; break;
                case 0x0D: character = '='; break;

                // QWERTY row
                case 0x10: character = 'q'; break;
                case 0x11: character = 'w'; break;
                case 0x12: character = 'e'; break;
                case 0x13: character = 'r'; break;
                case 0x14: character = 't'; break;
                case 0x15: character = 'y'; break;
                case 0x16: character = 'u'; break;
                case 0x17: character = 'i'; break;
                case 0x18: character = 'o'; break;
                case 0x19: character = 'p'; break;
                case 0x1A: character = '['; break;
                case 0x1B: character = ']'; break;
                case 0x2B: character = '\\'; break;

                // ASDF row
                case 0x1E: character = 'a'; break;
                case 0x1F: character = 's'; break;
                case 0x20: character = 'd'; break;
                case 0x21: character = 'f'; break;
                case 0x22: character = 'g'; break;
                case 0x23: character = 'h'; break;
                case 0x24: character = 'j'; break;
                case 0x25: character = 'k'; break;
                case 0x26: character = 'l'; break;
                case 0x27: character = ';'; break;
                case 0x28: character = '\''; break;
                case 0x29: character = '`'; break;

                // ZXCV row  
                case 0x2C: character = 'z'; break;
                case 0x2D: character = 'x'; break;
                case 0x2E: character = 'c'; break;
                case 0x2F: character = 'v'; break;
                case 0x30: character = 'b'; break;
                case 0x31: character = 'n'; break;
                case 0x32: character = 'm'; break;
                case 0x33: character = ','; break;
                case 0x34: character = '.'; break;
                case 0x35: character = '/'; break;
            }
            
            if (character != 0) {
                character = apply_shift(character);
                put_char(character);
            } else {
                // Show debug info for unhandled keys.
                printf_colored("[Key:0x", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
                char hex_digits[] = "0123456789ABCDEF";
                put_char(hex_digits[(scan_code >> 4) & 0xF]);
                put_char(hex_digits[scan_code & 0xF]);
                printf_colored("] ", VGA_COLOR_LIGHT_GRAY_ON_BLACK);
            }
            break;
        }
    }
    
    return esp;
}
