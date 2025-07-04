#include "port.h"

Port::Port(uint16_t port_number)
{
    this->port_number = port_number;
}

Port::~Port()
{

}

Port8Bit::Port8Bit(uint16_t port_number)
    : Port(port_number)
{
    
}

Port8Bit::~Port8Bit()
{

}

void Port8Bit::write(uint8_t data)
{
    write_8(port_number, data);
}

uint8_t Port8Bit::read()
{
    return read_8(port_number);
}

Port8BitSlow::Port8BitSlow(uint16_t port_number)
    : Port8Bit(port_number)
{

}

Port8BitSlow::~Port8BitSlow()
{
}

void Port8BitSlow::write(uint8_t data)
{
    write_8_slow(port_number, data);
}

Port16Bit::Port16Bit(uint16_t port_number)
    : Port(port_number)
{

}

Port16Bit::~Port16Bit()
{

}

void Port16Bit::write(uint16_t data)
{
    write_16(port_number, data);
}

uint16_t Port16Bit::read()
{
    return read_16(port_number);
}

Port32Bit::Port32Bit(uint16_t port_number)
    : Port(port_number)
{

}

Port32Bit::~Port32Bit()
{

}

void Port32Bit::write(uint32_t data)
{
    write_32(port_number, data);
}

uint32_t Port32Bit::read()
{
    return read_32(port_number);
}
