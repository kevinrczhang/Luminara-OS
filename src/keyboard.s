    .intel_syntax noprefix

    .code32
    .global read_scancode
    .global scancode_to_ascii
    .global get_key

read_scancode:
    mov dx, 0x64
.wait_status:
    in al, dx
    test al, 1
    jz .wait_status
    mov dx, 0x60
    in al, dx
    ret

scancode_to_ascii:
    push ebx
    movzx ebx, al
    mov al, BYTE PTR scancodes[ebx]
    pop ebx
    ret

get_key:
    call read_scancode
    call scancode_to_ascii
    ret

    .section .data
    .align 1
scancodes:
    # 0x00..0x0f: numbers row
    .byte 0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t'
    # 0x10..0x1f: QWERTY row
    .byte 'q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s'
    # 0x20..0x2f: ASDF row
    .byte 'd','f','g','h','j','k','l',';','\'', '`',0,'\\','z','x','c','v'
    # 0x30..0x3f: ZXCV row
    .byte 'b','n','m',',','.','/',0,'*',0,' ',0,0,0,0,0,0
