; kernel/start.asm - Kernel Entry Point with Interrupt Stubs
bits 32

; ========================
; 1. MULTIBOOT HEADER
; ========================
section .multiboot
align 4
    dd 0x1BADB002              ; Magic
    dd 0x00000003              ; Flags
    dd -(0x1BADB002 + 0x00000003) ; Checksum

; ========================
; 2. GLOBAL SYMBOLS
; ========================
global _start
extern kernel_main
extern isr_handler
extern irq_handler

; Export ALL interrupt stubs
global _isr0, _isr1, _isr2, _isr3, _isr4, _isr5, _isr6, _isr7
global _isr8, _isr9, _isr10, _isr11, _isr12, _isr13, _isr14, _isr15
global _isr16, _isr17, _isr18, _isr19, _isr20, _isr21, _isr22, _isr23
global _isr24, _isr25, _isr26, _isr27, _isr28, _isr29, _isr30, _isr31

global _irq0, _irq1, _irq2, _irq3, _irq4, _irq5, _irq6, _irq7
global _irq8, _irq9, _irq10, _irq11, _irq12, _irq13, _irq14, _irq15

; ========================
; 3. KERNEL START
; ========================
section .text

_start:
    ; Stack setzen
    mov esp, stack_top
    
    ; Multiboot Parameter übergeben
    push ebx        ; Multiboot info struct pointer
    push eax        ; Multiboot magic number
    
    ; Kernel aufrufen
    call kernel_main
    
    ; Falls zurück: hängen
    cli
.hang:
    hlt
    jmp .hang

; ========================
; 4. INTERRUPT STUBS
; ========================

; Macro für ISR ohne Error Code
%macro ISR_NOERRCODE 1
_isr%1:
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

; Macro für ISR mit Error Code
%macro ISR_ERRCODE 1
_isr%1:
    push byte %1
    jmp isr_common_stub
%endmacro

; Macro für IRQ
%macro IRQ 2
_irq%1:
    push byte 0
    push byte %2
    jmp irq_common_stub
%endmacro

; CPU Exceptions (0-31)
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; Hardware IRQs (32-47)
IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; Common ISR Handler
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call isr_handler
    add esp, 4
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    add esp, 8
    iret

; Common IRQ Handler
irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call irq_handler
    add esp, 4
    
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    add esp, 8
    iret

; ========================
; 5. STACK
; ========================
section .bss
align 16
stack_bottom:
    resb 16384  ; 16KB Stack
stack_top:
