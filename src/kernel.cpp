#include <stddef.h>
extern "C" void kernelMain();

// VGA text-mode buffer at 0xB8000
static volatile unsigned short* const VGA = (unsigned short*)0xB8000;
static unsigned int cursor = 0;

// write one character with white-on-black attribute
extern "C" void putchar(char c) {
    VGA[cursor++] = (unsigned char)c | (0x07 << 8);
}

// only handles C-strings at the moment
extern "C" void printf(const char* str) {
    for (size_t i = 0; str[i] != '\0'; ++i) {
        putchar(str[i]);
    }
}

extern "C" void kernelMain() {
    printf("Hello, world!");
    while (1) {
        asm volatile("hlt");
    }
}
