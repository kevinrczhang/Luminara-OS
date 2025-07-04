#ifndef INTERRUPT_MANAGER_H
#define INTERRUPT_MANAGER_H

#include "gdt.h"
#include "types.h"
#include "port.h"

class InterruptManager;

class InterruptHandler
{
protected:
    uint8_t interrupt_number;
    InterruptManager* interrupt_manager;
    
    InterruptHandler(InterruptManager* interrupt_manager, uint8_t interrupt_number);
    ~InterruptHandler();

public:
    virtual uint32_t handle_interrupt(uint32_t esp);
};

class InterruptManager
{
    friend class InterruptHandler;

protected:
    static InterruptManager* active_interrupt_manager;
    InterruptHandler* handlers[256];

    struct GateDescriptor
    {
        uint16_t handler_address_low_bits;
        uint16_t gdt_code_segment_selector;
        uint8_t reserved;
        uint8_t access;
        uint16_t handler_address_high_bits;
    } __attribute__((packed));

    static GateDescriptor interrupt_descriptor_table[256];

    struct InterruptDescriptorTablePointer
    {
        uint16_t size;
        uint32_t base;
    } __attribute__((packed));

    uint16_t hardware_interrupt_offset_value;
    
    static void set_interrupt_descriptor_table_entry(
        uint8_t interrupt,
        uint16_t code_segment_selector_offset, 
        void (*handler)(),
        uint8_t descriptor_privilege_level, 
        uint8_t descriptor_type
    );

    uint32_t do_handle_interrupt(uint8_t interrupt, uint32_t esp);

    Port8BitSlow pic_master_command_port;
    Port8BitSlow pic_master_data_port;
    Port8BitSlow pic_slave_command_port;
    Port8BitSlow pic_slave_data_port;

public:
    InterruptManager(uint16_t hardware_interrupt_offset, GlobalDescriptorTable* global_descriptor_table);
    ~InterruptManager();
    
    uint16_t get_hardware_interrupt_offset();
    void activate();
    void deactivate();
    
    // Main interrupt handling function (made it public so the wrapper can access it).
    static uint32_t handle_interrupt(uint8_t interrupt, uint32_t esp);
};

// Assembly interface functions declared with C linkage to avoid name mangling :)
extern "C" {
    // Wrapper function called from assembly code.
    uint32_t handle_interrupt_wrapper(uint8_t interrupt, uint32_t esp);
    
    void interrupt_ignore();
    
    void handle_exception_0x00();
    void handle_exception_0x01();
    void handle_exception_0x02();
    void handle_exception_0x03();
    void handle_exception_0x04();
    void handle_exception_0x05();
    void handle_exception_0x06();
    void handle_exception_0x07();
    void handle_exception_0x08();
    void handle_exception_0x09();
    void handle_exception_0x0a();
    void handle_exception_0x0b();
    void handle_exception_0x0c();
    void handle_exception_0x0d();
    void handle_exception_0x0e();
    void handle_exception_0x0f();
    void handle_exception_0x10();
    void handle_exception_0x11();
    void handle_exception_0x12();
    void handle_exception_0x13();
    
    void handle_interrupt_request_0x00();
    void handle_interrupt_request_0x01();
    void handle_interrupt_request_0x02();
    void handle_interrupt_request_0x03();
    void handle_interrupt_request_0x04();
    void handle_interrupt_request_0x05();
    void handle_interrupt_request_0x06();
    void handle_interrupt_request_0x07();
    void handle_interrupt_request_0x08();
    void handle_interrupt_request_0x09();
    void handle_interrupt_request_0x0a();
    void handle_interrupt_request_0x0b();
    void handle_interrupt_request_0x0c();
    void handle_interrupt_request_0x0d();
    void handle_interrupt_request_0x0e();
    void handle_interrupt_request_0x0f();
    void handle_interrupt_request_0x31();
}

#endif
