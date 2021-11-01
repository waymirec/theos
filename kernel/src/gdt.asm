; gdt.asm

[bits 64]
global load_gdt
load_gdt:
    lgdt [rdi]
    mov ax, 0x10      ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    pop rdi
    mov rax, 0x08     ; far jump
    push rax
    push rdi
    o64 retf
