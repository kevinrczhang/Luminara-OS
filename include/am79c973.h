#ifndef AM79C973_H
#define AM79C973_H

#include "driver.h"
#include "interrupts.h"
#include "pci.h"
#include "port.h"
#include "terminal.h"
#include "types.h"

class Am79C973;

class RawDataHandler
{
    protected:
        Am79C973* backend;
    
    public:
        RawDataHandler(Am79C973* backend);
        ~RawDataHandler();

        virtual bool on_raw_data_received(uint8_t* buffer, uint32_t size);

        void send(uint8_t* buffer, uint32_t size);
};

class Am79C973 : public Driver
{
    static const uint32_t STATUS_ERR    = 0x8000;  // Bit 15: Error
    static const uint32_t STATUS_CERR   = 0x2000;  // Bit 13: Collision Error
    static const uint32_t STATUS_MISS   = 0x1000;  // Bit 12: Missed Frame
    static const uint32_t STATUS_MERR   = 0x0800;  // Bit 11: Memory Error
    static const uint32_t STATUS_RINT   = 0x0400;  // Bit 10: Receive Interrupt
    static const uint32_t STATUS_TINT   = 0x0200;  // Bit 9:  Transmit Done
    static const uint32_t STATUS_IDON   = 0x0100;  // Bit 8:  Initialization Done

    struct InitializationBlock
    {
        uint16_t mode;
        uint8_t rlen_reserved;
        uint8_t tlen_reserved; 
        uint8_t physical_address[6];
        uint8_t reserved[2];
        uint8_t logical_address[8];
        uint32_t receive_buffer_descriptor_address;
        uint32_t send_buffer_descriptor_address;
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

    RawDataHandler* raw_data_handler;

    public:
        Am79C973(PeripheralComponentInterconnectDeviceDescriptor* device, InterruptManager* interrupt_manager);
        ~Am79C973();

        void initialize();
        void activate();
        void deactivate();
        void reset();
        string get_driver_name();
        uint32_t handle_interrupt(uint32_t esp);
        void send(uint8_t* buffer, int size);
        void receive();
        void set_handler(RawDataHandler* raw_data_handler);
        uint64_t get_mac_address();
};

#endif
