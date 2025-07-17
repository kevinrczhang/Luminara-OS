#ifndef PCI_H
#define PCI_H

#include "port.h"
#include "terminal.h"

class PeripheralComponentInterconnectDeviceDescriptor
{
    public:
        uint32_t port;
        uint32_t interrupt_number;

        uint16_t bus_number;
        uint16_t device_number;
        uint16_t function_number;

        uint16_t vendor_id;
        uint16_t device_id;

        uint8_t class_code;
        uint8_t subclass;
        uint8_t programming_interface;

        uint8_t revision_id;

        PeripheralComponentInterconnectDeviceDescriptor();
        ~PeripheralComponentInterconnectDeviceDescriptor();
};

class PeripheralComponentInterconnectController
{
    Port32Bit command_port;
    Port32Bit data_port;
    
    public:
        PeripheralComponentInterconnectController();
        ~PeripheralComponentInterconnectController();
        
        uint32_t read(uint16_t bus_number, uint16_t device_number, uint16_t function_number, uint32_t register_offset);
        void write(uint16_t bus_number, uint16_t device_number, uint16_t function_number, uint32_t register_offset, uint32_t value);
        bool device_has_functions(uint16_t bus_number, uint16_t device_number);
        
        void select_drivers(); // We will add the drivers in the param we pass to this function.
        PeripheralComponentInterconnectDeviceDescriptor get_device_descriptor(uint16_t bus_number, uint16_t device_number, uint16_t function_number);
};

#endif
