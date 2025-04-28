    .intel_syntax noprefix

    # Multiboot v1 header
    .section .multiboot
    .align 4
    .long 0x1BADB002       # magic
    .long 0                # flags
    .long -(0x1BADB002)    # checksum

    # 32-bit code entry point
    .text
    .code32
    .global _start

_start:
    # set up a stack at 0x90000 (we might change this later)
    mov esp, 0x00090000

    # call the kernel
    call kernelMain

1:  hlt
    jmp 1b
