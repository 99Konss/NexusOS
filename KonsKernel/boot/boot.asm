; boot/boot.asm
bits 32                     ; 32-bit Modus
global start                ; Export für Linker
extern kernel_main                ; Hauptkernel-Funktion (in C)

section .multiboot
align 4
    dd 0x1BADB002          ; Multiboot Magic
    dd 0x00                ; Flags
    dd -(0x1BADB002 + 0x00) ; Checksum

section .text
start:
    cli                    ; Interrupts ausschalten
    mov esp, stack_top    ; Stack-Pointer setzen
    
    call kernel_main            ; Zu C-Code springen
    
    hlt                   ; Falls kmain zurückkehrt

section .bss
align 16
stack_bottom:
    resb 16384            ; 16KB Stack
stack_top:
