#include "am79c973.h"

Am79C973::Am79C973(PeripheralComponentInterconnectDeviceDescriptor *device, InterruptManager* interrupt_manager)
:   Driver(interrupt_manager, device->interrupt_number + interrupt_manager->get_hardware_interrupt_offset()),
    mac_address_0_port(device->port),
    mac_address_2_port(device->port + 0x02),
    mac_address_4_port(device->port + 0x04),
    register_data_port(device->port + 0x10),
    register_address_port(device->port + 0x12),
    reset_port(device->port + 0x14),
    bus_control_register_data_port(device->port + 0x16)
{
    current_send_buffer = 0;
    current_receive_buffer = 0;

    uint64_t mac_0 { mac_address_0_port.read() % 256 };
    uint64_t mac_1 { mac_address_0_port.read() / 256 };
    uint64_t mac_2 { mac_address_2_port.read() % 256 };
    uint64_t mac_3 { mac_address_2_port.read() / 256 };
    uint64_t mac_4 { mac_address_4_port.read() % 256 };
    uint64_t mac_5 { mac_address_4_port.read() / 256 };
    
    uint64_t mac_address {
        mac_5 << 40
            | mac_4 << 32
            | mac_3 << 24
            | mac_2 << 16
            | mac_1 << 8
            | mac_0
    };
    
    // 32 bit mode
    register_address_port.write(20);
    bus_control_register_data_port.write(0x102);
    
    // stop any current operations
    register_address_port.write(0);
    register_data_port.write(0x04);
    
    // initBlock
    initialization_block.mode = 0x0000; // promiscuous mode = false (only receive packets for this MAC)
    initialization_block.reserved1 = 0;
    initialization_block.num_send_buffers = 3; // 2^3 --> 8 buffers
    initialization_block.reserved2 = 0;
    initialization_block.num_receive_buffers = 3; // 2^3 --> 8 buffers
    initialization_block.physical_address = mac_address;
    initialization_block.reserved3 = 0;
    initialization_block.logical_address = 0; // no multicast filtering
    
    // force 16 byte alignment
    send_buffer_descriptor = (BufferDescriptor*)((((uint32_t) &send_buffer_descriptor_memory[0]) + 15) & ~((uint32_t) 0xF));
    initialization_block.send_buffer_descriptor_address = (uint32_t) send_buffer_descriptor;
    receive_buffer_descriptor = (BufferDescriptor*)((((uint32_t) &receive_buffer_descriptor_memory[0]) + 15) & ~((uint32_t) 0xF));
    initialization_block.receive_buffer_descriptor_address = (uint32_t) receive_buffer_descriptor;
    
    for (uint8_t i = 0; i < 8; ++i) {
        send_buffer_descriptor[i].address = (((uint32_t) &send_buffers[i]) + 15) & ~(uint32_t) 0xF;
        send_buffer_descriptor[i].flags = 0x7FF | 0xF000;
        send_buffer_descriptor[i].flags2 = 0;
        send_buffer_descriptor[i].available = 0;
        
        receive_buffer_descriptor[i].address = (((uint32_t) &receive_buffers[i]) + 15) & ~(uint32_t) 0xF;
        receive_buffer_descriptor[i].flags = 0xF7FF | 0x80000000;
        receive_buffer_descriptor[i].flags2 = 0;
        receive_buffer_descriptor[i].available = 0;
    }
    
    register_address_port.write(1);
    register_data_port.write((uint32_t) (&initialization_block) & 0xFFFF);
    register_address_port.write(2);
    register_data_port.write(((uint32_t) (&initialization_block) >> 16) & 0xFFFF);
    
}

Am79C973::~Am79C973()
{
}

void Am79C973::initialize()
{

}

void Am79C973::activate()
{
    // TODO: we should make the magic numbers constants
    register_address_port.write(0);
    register_data_port.write(0x41); // start initialization

    register_address_port.write(4);
    uint32_t temp { register_data_port.read() };
    register_address_port.write(4);
    register_data_port.write(temp | 0xC00); // enables auto-padding and auto-stripping
    
    register_address_port.write(0);
    register_data_port.write(0x42); // start operations and enable interrupts
}

void Am79C973::deactivate()
{
    
}

void Am79C973::reset()
{
    reset_port.read();
    reset_port.write(0);
}

string Am79C973::get_driver_name() {
    return "Am79C973";
}

uint32_t Am79C973::handle_interrupt(uint32_t esp)
{
    printf("INTERRUPT FROM AMD am79c973\n");
    
    register_address_port.write(0);
    uint32_t temp { register_data_port.read() };

    printf_hex16((uint16_t) (temp & 0xFFFF));
    
    if ((temp & 0x8000) == 0x8000) printf("AMD am79c973 ERROR\n");
    if ((temp & 0x2000) == 0x2000) printf("AMD am79c973 COLLISION ERROR\n");
    if ((temp & 0x1000) == 0x1000) printf("AMD am79c973 MISSED FRAME\n");
    if ((temp & 0x0800) == 0x0800) printf("AMD am79c973 MEMORY ERROR\n");
    if ((temp & 0x0400) == 0x0400) receive();
    if ((temp & 0x0200) == 0x0200) printf("AMD am79c973 DATA SENT\n");
                               
    // awknowledge interrupt
    register_address_port.write(0);
    register_data_port.write(temp);
    
    if ((temp & 0x0100) == 0x0100) {
        printf("AMD am79c973 INIT DONE\n");
    }
    
    return esp;
}

void Am79C973::send(uint8_t* buffer, int size)
{
    int send_descriptor { current_send_buffer };
    current_send_buffer = (current_send_buffer + 1) % 8;
    
    // Cap the max size at 1518 bytes
    if (size > 1518) {
        size = 1518;
    }
    
    for (uint8_t *src = buffer + size -1, *dst = (uint8_t*) (send_buffer_descriptor[send_descriptor].address + size -1); src >= buffer; --src, --dst) {
        *dst = *src;
    }
    
    send_buffer_descriptor[send_descriptor].available = 0;
    send_buffer_descriptor[send_descriptor].flags2 = 0;
    send_buffer_descriptor[send_descriptor].flags = 0x8300F000 | ((uint16_t)((-size) & 0xFFF));

    register_address_port.write(0);
    register_data_port.write(0x48);
}

void Am79C973::receive() {
    printf("AMD am79c973 DATA RECEVED\n");

    for (; (receive_buffer_descriptor[current_receive_buffer].flags & 0x80000000) == 0; current_receive_buffer = (current_receive_buffer + 1) % 8) {
        if (!(receive_buffer_descriptor[current_receive_buffer].flags & 0x40000000)
            && (receive_buffer_descriptor[current_receive_buffer].flags & 0x03000000) == 0x03000000) {
            uint32_t size { receive_buffer_descriptor[current_receive_buffer].flags & 0xFFF };

            if (size > 64) { // remove checksum
                size -= 4;
            }
            
            uint8_t* buffer { (uint8_t*) (receive_buffer_descriptor[current_receive_buffer].address) };
            
            for (int i = 0; i < size; ++i) {
                printf((char*) buffer[i]);
                printf(" ");
            }
        }
        
        receive_buffer_descriptor[current_receive_buffer].flags2 = 0;
        receive_buffer_descriptor[current_receive_buffer].flags = 0x8000F7FF;
    }
}
