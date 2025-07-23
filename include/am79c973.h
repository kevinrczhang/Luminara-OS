#ifndef AM79C973_H
#define AM79C973_H

#include "driver.h"
#include "interrupts.h"
#include "pci.h"
#include "port.h"
#include "terminal.h"
#include "types.h"

class Am79C973 : public Driver
{
    // Holds address of buffer descriptors
    struct InitializationBlock
    {
        uint16_t mode;
        unsigned reserved1 : 4;
        unsigned num_send_buffers : 4;
        unsigned reserved2 : 4;
        unsigned num_receive_buffers : 4;
        uint64_t physical_address : 48;
        uint16_t reserved3;
        uint64_t logical_address;
        uint32_t send_buffer_descriptor_address;
        uint32_t receive_buffer_descriptor_address;
    } __attribute__((packed));

    struct BufferDescriptor
    {
        uint32_t address;
        uint32_t flags;
        uint32_t flags2;
        uint32_t available;
    } __attribute__((packed));

    Port16Bit mac_address_0_port;
    Port16Bit mac_address_2_port;
    Port16Bit mac_address_4_port;
    Port16Bit register_data_port;
    Port16Bit register_address_port;
    Port16Bit reset_port;
    Port16Bit bus_control_register_data_port;

    InitializationBlock initialization_block;

    BufferDescriptor* send_buffer_descriptor;
    uint8_t send_buffer_descriptor_memory[2048 + 15]; // add 15 since we're masking the smallest 4 bits
    uint8_t send_buffers[2 * 1024 + 15][8];
    uint8_t current_send_buffer;

    BufferDescriptor* receive_buffer_descriptor;
    uint8_t receive_buffer_descriptor_memory[2048 + 15];
    uint8_t receive_buffers[2 * 1024 + 15][8];
    uint8_t current_receive_buffer;

    public:
        Am79C973(PeripheralComponentInterconnectDeviceDescriptor* device, InterruptManager* interrupt_manager);
        ~Am79C973();

        void initialize();
        void activate();
        void deactivate();
        void reset();
        string get_driver_name();
        uint32_t handle_interrupt(uint32_t esp);
};

#endif
