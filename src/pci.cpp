#include "pci.h"

PeripheralComponentInterconnectDeviceDescriptor::PeripheralComponentInterconnectDeviceDescriptor()
{

}

PeripheralComponentInterconnectDeviceDescriptor::~PeripheralComponentInterconnectDeviceDescriptor()
{

}

PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
: command_port(0xCF8),
  data_port(0xCFC)
{

}

PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController()
{

}

uint32_t PeripheralComponentInterconnectController::read(uint16_t bus_number, uint16_t device_number, uint16_t function_number, uint32_t register_offset)
{
    uint32_t id =
        0x1 << 31
        | ((bus_number & 0xFF) << 16)
        | ((device_number & 0x1F) << 11)
        | ((function_number & 0x07) << 8)
        | (register_offset & 0xFC); // We can only read dwords (32 bits)
    command_port.write(id);
    uint32_t result = data_port.read();
    return result >> (8* (register_offset % 4));
}

bool PeripheralComponentInterconnectController::device_has_functions(uint16_t bus_number, uint16_t device_number)
{
    return read(bus_number, device_number, 0, 0x0E) & (1<<7); // This bit will tells us whether the device has functions.
}

void PeripheralComponentInterconnectController::select_drivers()
{
    for(int bus_number = 0; bus_number < 8; bus_number++)
    {
        for(int device_number = 0; device_number < 32; device_number++)
        {
            int num_functions = device_has_functions(bus_number, device_number) ? 8 : 1;
            for(int fucntion_number = 0; fucntion_number < num_functions; fucntion_number++)
            {
                PeripheralComponentInterconnectDeviceDescriptor device_descriptor = get_device_descriptor(bus_number, device_number, fucntion_number);
                
                if (device_descriptor.vendor_id == 0x0000 || device_descriptor.vendor_id == 0xFFFF)
                    break;
                
                printf("PCI BUS ");
                printf_hex16(bus_number & 0xFF);
                
                printf(", DEVICE ");
                printf_hex16(device_number & 0xFF);

                printf(", FUNCTION ");
                printf_hex16(fucntion_number & 0xFF);
                
                printf(" = VENDOR ");
                printf_hex16((device_descriptor.vendor_id & 0xFF00) >> 8);
                printf_hex16(device_descriptor.vendor_id & 0xFF);
                printf(", DEVICE ");
                printf_hex16((device_descriptor.device_id & 0xFF00) >> 8);
                printf_hex16(device_descriptor.device_id & 0xFF);
                printf("\n");
            }
        }
    }
}

PeripheralComponentInterconnectDeviceDescriptor PeripheralComponentInterconnectController::get_device_descriptor(uint16_t bus_number, uint16_t device_number, uint16_t function_number)
{
    PeripheralComponentInterconnectDeviceDescriptor result;
    
    result.bus_number = bus_number;
    result.device_id = device_number;
    result.function_number = function_number;
    
    result.vendor_id = read(bus_number, device_number, function_number, 0x00);
    result.device_id = read(bus_number, device_number, function_number, 0x02);

    result.class_code = read(bus_number, device_number, function_number, 0x0b);
    result.subclass = read(bus_number, device_number, function_number, 0x0a);
    result.programming_interface = read(bus_number, device_number, function_number, 0x09);

    result.revision_id = read(bus_number, device_number, function_number, 0x08);
    result.interrupt_number = read(bus_number, device_number, function_number, 0x3c);
    
    return result;
}
