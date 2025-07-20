.set IRQ_BASE, 0x20

.section .text

.extern handle_interrupt_wrapper

# Macro to generate exception handlers - using extern "C" names
.macro HANDLE_EXCEPTION num
.global handle_exception_\num
handle_exception_\num:
    movb $\num, (interrupt_number)
    jmp interrupt_bottom
.endm

# Macro to generate interrupt request handlers - using extern "C" names
.macro HANDLE_INTERRUPT_REQUEST num
.global handle_interrupt_request_\num
handle_interrupt_request_\num:
    movb $\num + IRQ_BASE, (interrupt_number)
    pushl $0
    jmp interrupt_bottom
.endm

# CPU Exception handlers (0x00-0x13)
HANDLE_EXCEPTION 0x00  # Divide by zero
HANDLE_EXCEPTION 0x01  # Debug
HANDLE_EXCEPTION 0x02  # Non-maskable interrupt
HANDLE_EXCEPTION 0x03  # Breakpoint
HANDLE_EXCEPTION 0x04  # Overflow
HANDLE_EXCEPTION 0x05  # Bound range exceeded
HANDLE_EXCEPTION 0x06  # Invalid opcode
HANDLE_EXCEPTION 0x07  # Device not available
HANDLE_EXCEPTION 0x08  # Double fault
HANDLE_EXCEPTION 0x09  # Coprocessor segment overrun
HANDLE_EXCEPTION 0x0a  # Invalid TSS
HANDLE_EXCEPTION 0x0b  # Segment not present
HANDLE_EXCEPTION 0x0c  # Stack-segment fault
HANDLE_EXCEPTION 0x0d  # General protection fault
HANDLE_EXCEPTION 0x0e  # Page fault
HANDLE_EXCEPTION 0x0f  # Reserved
HANDLE_EXCEPTION 0x10  # x87 floating-point exception
HANDLE_EXCEPTION 0x11  # Alignment check
HANDLE_EXCEPTION 0x12  # Machine check
HANDLE_EXCEPTION 0x13  # SIMD floating-point exception

# Hardware Interrupt Request handlers (IRQ 0-15)
HANDLE_INTERRUPT_REQUEST 0x00  # System timer
HANDLE_INTERRUPT_REQUEST 0x01  # Keyboard
HANDLE_INTERRUPT_REQUEST 0x02  # Cascade for slave PIC
HANDLE_INTERRUPT_REQUEST 0x03  # Serial port 2
HANDLE_INTERRUPT_REQUEST 0x04  # Serial port 1
HANDLE_INTERRUPT_REQUEST 0x05  # Parallel port 2
HANDLE_INTERRUPT_REQUEST 0x06  # Floppy disk
HANDLE_INTERRUPT_REQUEST 0x07  # Parallel port 1
HANDLE_INTERRUPT_REQUEST 0x08  # Real-time clock`
HANDLE_INTERRUPT_REQUEST 0x09  # Available
HANDLE_INTERRUPT_REQUEST 0x0a  # Available
HANDLE_INTERRUPT_REQUEST 0x0b  # Available
HANDLE_INTERRUPT_REQUEST 0x0c  # Mouse
HANDLE_INTERRUPT_REQUEST 0x0d  # Math coprocessor
HANDLE_INTERRUPT_REQUEST 0x0e  # Primary ATA hard disk
HANDLE_INTERRUPT_REQUEST 0x0f  # Secondary ATA hard disk
HANDLE_INTERRUPT_REQUEST 0x31  # System call (software interrupt)

# Common interrupt handler bottom half
interrupt_bottom:
    pushl %ebp
    pushl %edi
    pushl %esi

    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax

    # Call C++ interrupt handler wrapper
    pushl %esp
    push (interrupt_number)
    call handle_interrupt_wrapper
    movl %eax, %esp        # Switch stack if handler returned new ESP

    popl %eax
    popl %ebx
    popl %ecx
    popl %edx

    popl %esi
    popl %edi
    popl %ebp

    add $4, %esp

# Default ignore handler for unimplemented interrupts
.global interrupt_ignore
interrupt_ignore:
    iret

.data
    interrupt_number: .byte 0
