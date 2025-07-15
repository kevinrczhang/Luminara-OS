#ifndef DRIVER_H
#define DRIVER_H

#include "types.h"
#include "interrupts.h"

// Abstract base class for all hardware drivers
class Driver : public InterruptHandler
{
protected:
    Driver(InterruptManager* manager, uint8_t interrupt_number);

public:
    virtual ~Driver();
    
    // Pure virtual methods that derived drivers must implement
    virtual void initialize() = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void reset() = 0;
    
    // Virtual method from InterruptHandler that drivers can override
    virtual uint32_t handle_interrupt(uint32_t esp) override = 0;
    
    // Optional virtual methods for common driver operations
    virtual bool is_available() { return true; }
    virtual const char* get_driver_name() = 0;
    virtual uint32_t get_driver_version() { return 0x0100; } // Default version 1.0
};

#endif
