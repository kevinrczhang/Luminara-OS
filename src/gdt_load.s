.intel_syntax noprefix
.global loadGdt
.section .text
.code32

loadGdt:
    lgdt [eax]

    jmp 0x08:flushSegments

flushSegments:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
