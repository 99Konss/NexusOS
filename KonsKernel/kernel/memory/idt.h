// kernel/memory/idt.h
#ifndef KERNEL_MEMORY_IDT_H
#define KERNEL_MEMORY_IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256

struct idt_entry {
    unsigned short base_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short base_high;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

// Funktionen
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void isr_install(void);  // ← in idt.c!
void irq_install(void);  // ← in idt.c!

// Externe Variablen
extern struct idt_entry idt[IDT_ENTRIES];
extern struct idt_ptr idtp;

// ========================
// ASSEMBLY STUBS - ALLE HIER!
// ========================

// CPU Exceptions (0-31)
extern void _isr0(void);  extern void _isr1(void);  extern void _isr2(void);  extern void _isr3(void);
extern void _isr4(void);  extern void _isr5(void);  extern void _isr6(void);  extern void _isr7(void);
extern void _isr8(void);  extern void _isr9(void);  extern void _isr10(void); extern void _isr11(void);
extern void _isr12(void); extern void _isr13(void); extern void _isr14(void); extern void _isr15(void);
extern void _isr16(void); extern void _isr17(void); extern void _isr18(void); extern void _isr19(void);
extern void _isr20(void); extern void _isr21(void); extern void _isr22(void); extern void _isr23(void);
extern void _isr24(void); extern void _isr25(void); extern void _isr26(void); extern void _isr27(void);
extern void _isr28(void); extern void _isr29(void); extern void _isr30(void); extern void _isr31(void);

// Hardware IRQs (32-47) - DAS HIER HAT GEFEHLT!
extern void _irq0(void);  extern void _irq1(void);  extern void _irq2(void);  extern void _irq3(void);
extern void _irq4(void);  extern void _irq5(void);  extern void _irq6(void);  extern void _irq7(void);
extern void _irq8(void);  extern void _irq9(void);  extern void _irq10(void); extern void _irq11(void);
extern void _irq12(void); extern void _irq13(void); extern void _irq14(void); extern void _irq15(void);

#endif
