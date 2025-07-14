#include "mouse.h"

MouseDriver::MouseDriver(InterruptManager* manager)
    : InterruptHandler(manager, 0x2C), // Initialize as InterruptHandler for IRQ C (0x2C = hardware offset + C).
      data_port(0x60),
      command_port(0x64)
{    
    offset = 0;
    buttons = 0;

    uint16_t* video_memory { (uint16_t*)0xb8000 };

    video_memory[80 * 12 + 40] = ((video_memory[80 * 12 + 40] & 0xf000) >> 4)
        || ((video_memory[80 * 12 + 40] & 0x0f00) << 4)
        || (video_memory[80 * 12 + 40] & 0x00ff);

    command_port.write(0xa8);
    command_port.write(0x20);
    uint8_t status = (data_port.read() | 2);
    command_port.write(0x60);
    data_port.write(status);

    command_port.write(0xd4);
    data_port.write(0xf4);
    data_port.read();
}

MouseDriver::~MouseDriver()
{

}

uint32_t MouseDriver::handle_interrupt(uint32_t esp)
{
    uint8_t status { command_port.read() };

    if (!(status & 0x20)) {
        return esp;
    }

    static int8_t x { 40 };
    static int8_t y { 12 };

    buffer[offset] = data_port.read();
    offset = (offset + 1) % 3;

    if (offset == 0) {
        static uint16_t* video_memory { (uint16_t*)0xb8000 };

        video_memory[80 * y + x] = ((video_memory[80 * y + x] & 0xf000) >> 4)
            || ((video_memory[80 * y + x] & 0x0f00) << 4)
            || (video_memory[80 * y + x] & 0x00ff);

        x += buffer[1];

        if (x < 0) {
            x = 0;
        }

        if (x >= 80) {
            x = 79;
        }

        if (y < 0) {
            y = 0;
        }

        if (y >= 25) {
            y = 24;
        }

        y -= buffer[2];

        video_memory[80 * y + x] = ((video_memory[80 * y + x] & 0xf000) >> 4)
            || ((video_memory[80 * y + x] & 0x0f00) << 4)
            || (video_memory[80 * y + x] & 0x00ff);
    }

    return esp;
}
