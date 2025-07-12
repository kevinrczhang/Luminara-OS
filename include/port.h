#ifndef PORT_H
#define PORT_H

#include "types.h"

class Port
{
protected:
    uint16_t port_number;
    
    Port(uint16_t port_number);
    // TODO: This must be virtual (currently isn't because the kernel has no memory management yet).
    ~Port();
};

class Port8Bit : public Port
{
public:
    Port8Bit(uint16_t port_number);
    ~Port8Bit();

    virtual uint8_t read();
    virtual void write(uint8_t data);
    
    // Non-virtual methods for direct port access (no function call overhead)
    inline uint8_t read_direct()
    {
        return read_8(port_number);
    }
    
    inline void write_direct(uint8_t data)
    {
        write_8(port_number, data);
    }

protected:
    static inline uint8_t read_8(uint16_t port)
    {
        uint8_t result;
        // Store the result from the specified port into the eax register.
        __asm__ volatile("inb %1, %0" : "=a" (result) : "Nd" (port));
        return result;
    }

    static inline void write_8(uint16_t port, uint8_t data)
    {
        // Write the data value to the io port specified.
        __asm__ volatile("outb %0, %1" : : "a" (data), "Nd" (port));
    }
};

class Port8BitSlow : public Port8Bit
{
public:
    Port8BitSlow(uint16_t port_number);
    ~Port8BitSlow();

    virtual void write(uint8_t data);

protected:
    static inline void write_8_slow(uint16_t port, uint8_t data)
    {
        // Create two labels "1" and jump to them to create a small delay (needed for slow hardware).
        __asm__ volatile("outb %0, %1\njmp 1f\n1: jmp 1f\n1:" : : "a" (data), "Nd" (port));
    }
};

class Port16Bit : public Port
{
public:
    Port16Bit(uint16_t port_number);
    ~Port16Bit();

    virtual uint16_t read();
    virtual void write(uint16_t data);

protected:
    static inline uint16_t read_16(uint16_t port)
    {
        uint16_t result;
        __asm__ volatile("inw %1, %0" : "=a" (result) : "Nd" (port));
        return result;
    }

    static inline void write_16(uint16_t port, uint16_t data)
    {
        __asm__ volatile("outw %0, %1" : : "a" (data), "Nd" (port));
    }
};

class Port32Bit : public Port
{
public:
    Port32Bit(uint16_t port_number);
    ~Port32Bit();

    virtual uint32_t read();
    virtual void write(uint32_t data);

protected:
    static inline uint32_t read_32(uint16_t port)
    {
        uint32_t result;
        __asm__ volatile("inl %1, %0" : "=a" (result) : "Nd" (port));
        return result;
    }

    static inline void write_32(uint16_t port, uint32_t data)
    {
        __asm__ volatile("outl %0, %1" : : "a"(data), "Nd" (port));
    }
};

#endif
