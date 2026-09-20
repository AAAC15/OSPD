; boot de ospd
; uso multiboot

multiboot_page_align  equ 1 << 0   
multiboot_memory_info equ 1 << 1   
multiboot_flags       equ multiboot_page_align | multiboot_memory_info
multiboot_magic       equ 0x1badb002
multiboot_checksum    equ -(multiboot_magic + multiboot_flags)

section .multiboot
align 4
    dd multiboot_magic
    dd multiboot_flags
    dd multiboot_checksum
    
section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .text
global _start

_start:
    cli
    mov esp, stack_top

    ; apagar cursor de hardware
    mov dx, 0x3d4
    mov al, 0x0a
    out dx, al
    inc dx
    mov al, 0x20
    out dx, al

    ; limpiar toda la pantalla vga
    mov edi, 0xb8000
    mov ecx, 2000
    mov ax, 0x0720

.clear_loop:
    mov [edi], ax
    add edi, 2
    loop .clear_loop

    mov edi, 0xb8000

    ; imprimir bootok
    mov word [edi],      0x075b ; [
    mov word [edi + 2],  0x0242 ; b
    mov word [edi + 4],  0x024f ; o
    mov word [edi + 6],  0x024f ; o
    mov word [edi + 8],  0x0254 ; t
    mov word [edi + 10], 0x024f ; o
    mov word [edi + 12], 0x024b ; k
    mov word [edi + 14], 0x075d ; ]

    ; pasar parametros a c
    push ebx             
    push eax             
    
    extern kernelMain
    call kernelMain      
    
    cli
.loop:
    hlt
    jmp .loop
