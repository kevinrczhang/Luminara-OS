#include "gdt.h"

GlobalDescriptorTable::GlobalDescriptorTable()
    : null_segment_descriptor(0, 0, 0),                                             // The first entry in any GDT must be null.
      unused_segment_descriptor(0, 0, 0),
      code_segment_descriptor(0, GDT::SEGMENT_SIZE_64MB, GDT::KERNEL_CODE_SEGMENT), // Reserved for kernel code (64MB, executable).
      data_segment_descriptor(0, GDT::SEGMENT_SIZE_64MB, GDT::KERNEL_DATA_SEGMENT)  // Reserved for kernel data (64MB, writable).
{
    // Creates the GDT pointer structure for LGDT instruction.
    // The processor expects: [16-bit limit][32-bit base address].
    uint32_t gdt_descriptor[2];
    gdt_descriptor[1] = (uint32_t)this;                        // Base address of the GDT.
    gdt_descriptor[0] = sizeof(GlobalDescriptorTable) << 16;   // The size limit of the GDT.
    
    // This loads the GDT descriptor into the GDTR register.
    // The +2 offset skips the limit field since LGDT expects limit first, then base
    asm volatile("lgdt (%0)" : : "p" (((uint8_t*)gdt_descriptor) + 2));
}

GlobalDescriptorTable::~GlobalDescriptorTable()
{
    // This is empty since the GDT remains active until system shutdown :)
}

uint16_t GlobalDescriptorTable::get_data_segment_selector()
{
    // Calculate the offset of the data segment descriptor from the start of GDT.
    return (uint8_t*)&data_segment_descriptor - (uint8_t*)this;
}

uint16_t GlobalDescriptorTable::get_code_segment_selector()
{
    // Calculate the offset of the code segment descriptor from the start of GDT.
    return (uint8_t*)&code_segment_descriptor - (uint8_t*)this;
}

GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(uint32_t base_address, uint32_t segment_limit, uint8_t access_byte)
{
    bool use_page_granularity = (segment_limit > GDT::SEGMENT_SIZE_64KB);
    
    if (use_page_granularity) {
        // Convert the byte limit to page limit (divide by 4096) and ensure it is properly aligned to the 4KB boundary.
        if ((segment_limit & 0xFFF) != 0xFFF) {
            // Round down to nearest page boundary and subtract 1.
            segment_limit = (segment_limit >> 12) - 1;
        } else {
            // This means it is already aligned, so we just convert to pages uwu.
            segment_limit = segment_limit >> 12;
        }
        
        limit_bits_16_19_and_flags = GDT::FLAG_32BIT | GDT::FLAG_GRANULARITY_4KB;
    } else {
        limit_bits_16_19_and_flags = GDT::FLAG_16BIT | GDT::FLAG_GRANULARITY_BYTE;
    }

    limit_bits_0_15 = segment_limit & 0xFFFF;
    limit_bits_16_19_and_flags |= (segment_limit >> 16) & 0x0F;

    base_bits_0_15 = base_address & 0xFFFF;
    base_bits_16_23 = (base_address >> 16) & 0xFF;
    base_bits_24_31 = (base_address >> 24) & 0xFF;

    access_flags = access_byte;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::get_base_address()
{
    uint32_t base_address = base_bits_24_31;
    base_address = (base_address << 8) + base_bits_16_23;
    base_address = (base_address << 16) + base_bits_0_15;

    return base_address;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::get_limit()
{
    uint32_t limit = limit_bits_16_19_and_flags & 0x0F;
    limit = (limit << 16) + limit_bits_0_15;

    bool page_granularity = (limit_bits_16_19_and_flags & GDT::FLAG_GRANULARITY_4KB) != 0;
    
    if (page_granularity) {
        // Convert from pages back to bytes and add the page offset
        limit = (limit << 12) | 0xFFF;
    }

    return limit;
}
