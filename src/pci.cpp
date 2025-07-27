#include "am79c973.h"
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
    uint32_t identifier {
        0x1 << 31
        | ((bus_number & 0xFF) << 16)
        | ((device_number & 0x1F) << 11)
        | ((function_number & 0x07) << 8)
        | (register_offset & 0xFC) // We can only read dwords (32 bits)
    };
    command_port.write(identifier);
    uint32_t result { data_port.read() };
    return result >> (8* (register_offset % 4));
}

void PeripheralComponentInterconnectController::write(uint16_t bus_number, uint16_t device_number, uint16_t function_number, uint32_t register_offset, uint32_t value)
{
    uint32_t identifier {
        0x1 << 31
        | ((bus_number & 0xFF) << 16)
        | ((device_number & 0x1F) << 11)
        | ((function_number & 0x07) << 8)
        | (register_offset & 0xFC) // We can only read dwords (32 bits)
    };
    command_port.write(identifier);
    data_port.write(value);
}

bool PeripheralComponentInterconnectController::device_has_functions(uint16_t bus_number, uint16_t device_number)
{
    return read(bus_number, device_number, 0, 0x0E) & (1<<7); // This bit will tells us whether the device has functions.
}

void PeripheralComponentInterconnectController::select_drivers(DriverManager* driver_manager, InterruptManager* interrupt_manager)
{
    for (int bus_number = 0; bus_number < 8; ++bus_number) {

        for (int device_number = 0; device_number < 32; ++device_number) {

            int num_functions = device_has_functions(bus_number, device_number) ? 8 : 1;

            for (int function_number = 0; function_number < num_functions; ++function_number) {

                PeripheralComponentInterconnectDeviceDescriptor device_descriptor = get_device_descriptor(bus_number, device_number, function_number);
                
                if (device_descriptor.vendor_id == 0x0000 || device_descriptor.vendor_id == 0xFFFF) {
                    continue;
                }

                for (int bar_number = 0; bar_number < 6; ++bar_number) {
                    BaseAddressRegister base_address_register = get_base_address_register(bus_number, device_number, function_number, bar_number);

                    if (base_address_register.address && (base_address_register.type == InputOutput)) {
                        device_descriptor.port = (uint32_t) base_address_register.address;
                    }
                }

                Driver* driver { get_driver(device_descriptor, interrupt_manager) };

                if (driver != 0) {
                    driver_manager->register_driver(driver);
                }
                
                printf_colored("PCI BUS: ", VGA_COLOR_GREEN_ON_BLACK);
                printf_hex16(bus_number & 0xFF);
                
                printf_colored(", DEVICE: ", VGA_COLOR_GREEN_ON_BLACK);
                printf_hex16(device_number & 0xFF);

                printf_colored(", FUNCTION: ", VGA_COLOR_GREEN_ON_BLACK);
                printf_hex16(function_number & 0xFF);
                
                printf_colored(" = VENDOR: ", VGA_COLOR_GREEN_ON_BLACK);
                printf_hex16((device_descriptor.vendor_id & 0xFF00) >> 8);
                printf_hex16(device_descriptor.vendor_id & 0xFF);
                
                printf_colored(", DEVICE: ", VGA_COLOR_GREEN_ON_BLACK);
                printf_hex16((device_descriptor.device_id & 0xFF00) >> 8);
                printf_hex16(device_descriptor.device_id & 0xFF);

                printf("\n");
            }
        }
    }
}

BaseAddressRegister PeripheralComponentInterconnectController::get_base_address_register(uint16_t bus_number, uint16_t device_number, uint16_t function_number, uint16_t bar_number)
{
    BaseAddressRegister result;
    
    uint32_t bar_type { read(bus_number, device_number, function_number, 0x0E) & 0x7F };
    int max_bars { 6 - (4 * bar_type) };

    if (bar_number >= max_bars) {
        return result;
    }
    
    uint32_t bar_value { read(bus_number, device_number, function_number, 0x10 + 4 * bar_number) };

    result.type = (bar_value & 0x1) ? InputOutput : MemoryMapping;
    
    if (result.type == MemoryMapping) {

        // To be implemented in the future.
        
        switch ((bar_value >> 1) & 0x3)
        {
            
            case 0: // 32 Bit Mode
                break;
            case 1: // 20 Bit Mode
                break;
            case 2: // 64 Bit Mode
                break;
        }
        
    } else {
        result.address = (uint8_t*) (bar_value & ~0x3);
        result.prefetchable_bit = false;
    }
    
    return result;
}

Driver* PeripheralComponentInterconnectController::get_driver(PeripheralComponentInterconnectDeviceDescriptor device_descriptor, InterruptManager* interrupt_manager)
{
    Driver* driver = nullptr;

    switch (device_descriptor.vendor_id)
    {
        case 0x1022: // AMD
            switch(device_descriptor.device_id)
            {
                case 0x2000: // AM79C973 (AMD PCnet-PCI II)
                    driver = (Am79C973*) MemoryManager::memory_manager->malloc(sizeof(Am79C973));
                    if (driver != nullptr) {
                        new (driver) Am79C973(&device_descriptor, interrupt_manager);
                    }
                    printf("AMD am79c973 ");
                    return driver;
                    break;
            }
            break;

        case 0x8086: // Intel
            switch(device_descriptor.device_id)
            {
                case 0x100E: // 82540EM Gigabit Ethernet Controller
                    printf("Intel 82540EM ");
                    break;
            }
            break;
    }
    
    
    switch (device_descriptor.class_code)
    {
        case 0x03: // graphics
            switch(device_descriptor.subclass)
            {
                case 0x00: // VGA
                    printf("VGA ");
                    break;
            }
            break;
    }
    
    return 0;
}

PeripheralComponentInterconnectDeviceDescriptor PeripheralComponentInterconnectController::get_device_descriptor(uint16_t bus_number, uint16_t device_number, uint16_t function_number)
{
    PeripheralComponentInterconnectDeviceDescriptor device_descriptor;

    device_descriptor.interrupt_number = read(bus_number, device_number, function_number, 0x3c);
    
    device_descriptor.bus_number = bus_number;
    device_descriptor.device_number = device_number;
    device_descriptor.function_number = function_number;
    
    device_descriptor.vendor_id = read(bus_number, device_number, function_number, 0x00);
    device_descriptor.device_id = read(bus_number, device_number, function_number, 0x02);

    device_descriptor.class_code = read(bus_number, device_number, function_number, 0x0b);
    device_descriptor.subclass = read(bus_number, device_number, function_number, 0x0a);
    device_descriptor.programming_interface = read(bus_number, device_number, function_number, 0x09);

    device_descriptor.revision_id = read(bus_number, device_number, function_number, 0x08);
    
    return device_descriptor;
}
