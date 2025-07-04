.set MAGIC, 0x1badb002

# Bit 0 (1<<0): Requests page-aligned loading and memory information
# Bit 1 (1<<1): Requests memory map information from the bootloader
.set FLAGS, (1<<0 | 1<<1)

.set CHECKSUM, -(MAGIC + FLAGS)

# This section must be within the first 8192 bytes of the kernel file.
# It is what the bootloader searches for when attemping to load a kernel.
.section multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

.section .text

# Tells the assembler there is going to be a function called kernelMain.
.extern kernelMain

# ".global" allows other files to call it.
.global _start

_start:
    mov $kernel_stack, %esp

    # Contains the Multiboot magic number 0x2badb002 (different from header magic!).
    # This proves the kernel was loaded by a Multiboot-compliant bootloader.
    # Our kernel can use this to ensure that it was loaded correctly. 
    push %eax

    # Contains a pointer to the Multiboot information structure.
    # This structure contains memory maps, boot device info, 
    # command line arguments, module information, etc.
    # Some of the info includes what we asked for in the flags above!
    push %ebx
    
    call kernelMain

# Theoretically, our kernel should be going in an infinite loop.
# However, in the event we end up here, we stay in a loop.
# TODO: See if we actually need this.
_stop:
    cli
    hlt
    jmp _stop

# .bss section is used to store uninitialized global and static variables
.section .bss

# We want to put some empty space in memory between the stack pointer and 
# our code since the stack grows from the bottom.
.space 2*1024*1024; # 2MB

kernel_stack:
