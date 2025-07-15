#ifndef GDT_H
#define GDT_H

#include "types.h"

class GlobalDescriptorTable
{
public:
    class SegmentDescriptor
    {
    private:
        uint16_t limit_bits_0_15;
        uint16_t base_bits_0_15;
        uint8_t base_bits_16_23;
        uint8_t access_flags;
        uint8_t limit_bits_16_19_and_flags;
        uint8_t base_bits_24_31;

    public:
        SegmentDescriptor(uint32_t base_address, uint32_t segment_limit, uint8_t access_byte);
        
        uint32_t get_base_address();
        uint32_t get_limit();
        
    } __attribute__((packed)); // This is to prevent compiler padding, since it must be exactly 8 bytes.

private:
    SegmentDescriptor null_segment_descriptor;
    SegmentDescriptor unused_segment_descriptor;
    SegmentDescriptor code_segment_descriptor;
    SegmentDescriptor data_segment_descriptor;

public:
    GlobalDescriptorTable();
    ~GlobalDescriptorTable();

    uint16_t get_code_segment_selector();
    uint16_t get_data_segment_selector();
};

namespace GDT {
    const uint8_t ACCESS_PRESENT        = 0x80;
    const uint8_t ACCESS_PRIVILEGE_0    = 0x00;
    const uint8_t ACCESS_PRIVILEGE_3    = 0x60;
    const uint8_t ACCESS_DESCRIPTOR     = 0x10;
    const uint8_t ACCESS_EXECUTABLE     = 0x08;
    const uint8_t ACCESS_READABLE       = 0x02;
    const uint8_t ACCESS_WRITABLE       = 0x02;

    const uint8_t KERNEL_CODE_SEGMENT   = ACCESS_PRESENT | ACCESS_PRIVILEGE_0 | 
                                         ACCESS_DESCRIPTOR | ACCESS_EXECUTABLE | ACCESS_READABLE;  // 0x9A
    const uint8_t KERNEL_DATA_SEGMENT   = ACCESS_PRESENT | ACCESS_PRIVILEGE_0 | 
                                         ACCESS_DESCRIPTOR | ACCESS_WRITABLE;                      // 0x92

    const uint8_t FLAG_16BIT            = 0x00;
    const uint8_t FLAG_32BIT            = 0x40;
    const uint8_t FLAG_GRANULARITY_4KB  = 0x80;  // Limit in 4KB pages
    const uint8_t FLAG_GRANULARITY_BYTE = 0x00;  // Limit in bytes
    
    const uint32_t SEGMENT_SIZE_64MB    = 64 * 1024 * 1024;  // 64 MB
    const uint32_t SEGMENT_SIZE_64KB    = 64 * 1024;         // 64 KB
    const uint32_t PAGE_SIZE_4KB        = 4096;              // 4 KB
}

#endif
