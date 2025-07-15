#ifndef MOUSE_H
#define MOUSE_H

#include "types.h"
#include "interrupts.h"
#include "port.h"
#include "driver.h"

class MouseDriver : public Driver
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
    
    // Driver interface implementation
    virtual void initialize() override;
    virtual void activate() override;
    virtual void deactivate() override;
    virtual void reset() override;
    virtual const char* get_driver_name() override;
    
    // InterruptHandler interface implementation
    virtual uint32_t handle_interrupt(uint32_t esp) override;
};

#endif
