#include <stdint.h>

extern "C" void loadGdt(void*);

class Gdt {
private:
    struct __attribute__((packed)) GdtEntry {
        uint16_t limitLow;
        uint16_t baseLow;
        uint8_t  baseMiddle;
        uint8_t  access;
        uint8_t  granularity;
        uint8_t  baseHigh;
    };

    struct __attribute__((packed)) GdtDescriptor {
        uint16_t size;
        uint32_t offset;
    };

    GdtEntry entries[3];
    GdtDescriptor descriptor;

    void setEntry(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
        entries[index].limitLow     = limit & 0xFFFF;
        entries[index].baseLow      = base & 0xFFFF;
        entries[index].baseMiddle   = (base >> 16) & 0xFF;
        entries[index].access       = access;
        entries[index].granularity  = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
        entries[index].baseHigh     = (base >> 24) & 0xFF;
    }

public:
    Gdt() {
        setEntry(0, 0, 0, 0, 0);
        setEntry(1, 0, 0xFFFFF, 0x9A, 0xCF);
        setEntry(2, 0, 0xFFFFF, 0x92, 0xCF);

        descriptor.size = sizeof(entries) - 1;
        descriptor.offset = reinterpret_cast<uint32_t>(&entries);
    }

    void load() {
        loadGdt(&descriptor);
    }
};

Gdt gdt;

extern "C" void initGdt() {
    gdt.load();
}
