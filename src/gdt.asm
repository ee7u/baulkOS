global setGdt
global reloadSegments

section .data
gdtr dw 0 ; limit
     dw 0 ; base

section .text

setGdt:
   mov   [gdtr], di
   mov   [gdtr+2], rsi
   lgdt  [gdtr]
   ret

reloadSegments:
    mov ax, 0x10 ; 0x10 is the offset in the GDT to the data segment
    mov ds, ax ; Load all data segment selectors
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    push 0x08 ; 0x08 is the offset to the code segment
    lea rax, [rel .flush]
    push rax
    retfq
.flush:
    ret
