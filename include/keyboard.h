#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "interrupts.h"
#include "port.h"
#include "driver.h"

// Honestly, I haven't tested that these all work, we will have to test them.
// PS2 keyboard driver
class KeyboardDriver : public Driver
{
private:
    Port8Bit data_port;     // 0x60 - Keyboard data port
    Port8Bit command_port;  // 0x64 - Keyboard command/status port
    
    bool left_shift_pressed;
    bool right_shift_pressed;
    bool ctrl_pressed;
    bool alt_pressed;
    bool caps_lock_active;
    
    // Extended key handling
    bool extended_key_next;  // True if next key is an extended key (after 0xE0)
    
    bool is_shift_pressed();
    bool should_capitalize(char c);
    char apply_shift(char c);
    void handle_special_key(uint8_t scan_code);
    void handle_extended_key(uint8_t scan_code);

public:
    KeyboardDriver(InterruptManager* manager);
    ~KeyboardDriver();
    
    virtual void initialize() override;
    virtual void activate() override;
    virtual void deactivate() override;
    virtual void reset() override;
    virtual const char* get_driver_name() override;
    
    // InterruptHandler interface implementation
    virtual uint32_t handle_interrupt(uint32_t esp) override;
};

namespace Keyboard {
    // Special keys
    const uint8_t KEY_BACKSPACE     = 0x0E;
    const uint8_t KEY_TAB           = 0x0F;
    const uint8_t KEY_ENTER         = 0x1C;
    const uint8_t KEY_SPACE         = 0x39;
    const uint8_t KEY_ESCAPE        = 0x01;
    
    // Function keys
    const uint8_t KEY_F1            = 0x3B;
    const uint8_t KEY_F2            = 0x3C;
    const uint8_t KEY_F3            = 0x3D;
    const uint8_t KEY_F4            = 0x3E;
    const uint8_t KEY_F5            = 0x3F;
    const uint8_t KEY_F6            = 0x40;
    const uint8_t KEY_F7            = 0x41;
    const uint8_t KEY_F8            = 0x42;
    const uint8_t KEY_F9            = 0x43;
    const uint8_t KEY_F10           = 0x44;
    const uint8_t KEY_F11           = 0x57;
    const uint8_t KEY_F12           = 0x58;
    
    // Modifier keys
    const uint8_t KEY_LEFT_SHIFT    = 0x2A;
    const uint8_t KEY_RIGHT_SHIFT   = 0x36;
    const uint8_t KEY_LEFT_CTRL     = 0x1D;
    const uint8_t KEY_LEFT_ALT      = 0x38;
    const uint8_t KEY_CAPS_LOCK     = 0x3A;
    
    // Extended keys (prefixed with 0xE0)
    const uint8_t EXTENDED_KEY_PREFIX = 0xE0;
    const uint8_t KEY_RIGHT_CTRL    = 0x1D;
    const uint8_t KEY_RIGHT_ALT     = 0x38;
    const uint8_t KEY_DELETE        = 0x53;
    const uint8_t KEY_HOME          = 0x47;
    const uint8_t KEY_END           = 0x4F;
    const uint8_t KEY_PAGE_UP       = 0x49;
    const uint8_t KEY_PAGE_DOWN     = 0x51;
    const uint8_t KEY_ARROW_UP      = 0x48;
    const uint8_t KEY_ARROW_DOWN    = 0x50;
    const uint8_t KEY_ARROW_LEFT    = 0x4B;
    const uint8_t KEY_ARROW_RIGHT   = 0x4D;
    
    // Key state flags
    const uint8_t KEY_RELEASED      = 0x80;  // OR'd with scan code for key release
}

#endif
