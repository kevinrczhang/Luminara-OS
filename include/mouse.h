#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"
#include "interrupts.h"
#include "port.h"

class MouseDriver : public InterruptHandler
{
private:
    Port8Bit data_port;
    Port8Bit command_port;
    
    uint8_t buffer[3];
    uint8_t offset;
    uint8_t buttons;

public:
    MouseDriver(InterruptManager* manager);
    ~MouseDriver();
    
    virtual uint32_t handle_interrupt(uint32_t esp);
};

#endif
