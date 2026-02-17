// kernel/memory/idt.c
#include "idt.h"
#include "isr.h"  // für isr_handler/irq_handle
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../drivers/pic.h"
#include "../drivers/mouse.h"

#define IDT_ENTRIES 256

// IDT Tabellen - HIER werden sie definiert!
struct idt_entry idt[IDT_ENTRIES];
struct idt_ptr idtp;

static void (*irq_handlers[16])(void) = {0};  // Array von Funktionspointern

void irq_install_handler(int irq, void (*handler)(void)) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = handler;  // ← Jetzt funktioniert '='
    }
}


static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].base_low = (base & 0xFFFF);
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

void isr_install(void) {
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (unsigned int)&idt;

    // Clear IDT
    for(int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // CPU Exceptions (0-31)
    idt_set_gate(0, (unsigned long)_isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned long)_isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned long)_isr2, 0x08, 0x8E);
    // ... bis 31
    idt_set_gate(31, (unsigned long)_isr31, 0x08, 0x8E);

    // Hardware IRQs (32-47)
    idt_set_gate(32, (unsigned long)_irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned long)_irq1, 0x08, 0x8E);
    // ... bis 47
    idt_set_gate(47, (unsigned long)_irq15, 0x08, 0x8E);

    // Load IDT
    asm volatile("lidt (%0)" : : "r" (&idtp));
}

void irq_install(void) {
    // Mask ALL interrupts first
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    // Unmask only Timer (IRQ0) and Keyboard (IRQ1)
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
}

void irq_handler(struct regs *r) {
    unsigned char irq_num = r->int_no - 32;

    // Timer
    if (irq_num == 0) {
        static uint32_t timer_ticks = 0;
        timer_ticks++;

        if (timer_ticks % 18 == 0) {
            // Update uptime
            int old_x = cursor_x;
            int old_y = cursor_y;
            set_cursor(68, 0);

            uint32_t seconds = timer_ticks / 18;
            char sec_str[10];
            int i = 0;
            uint32_t n = seconds;

            if (n == 0) sec_str[i++] = '0';
            else {
                char temp[10];
                int j = 0;
                while (n > 0) {
                    temp[j++] = '0' + (n % 10);
                    n /= 10;
                }
                while (j > 0) sec_str[i++] = temp[--j];
            }
            sec_str[i] = '\0';

            kprint("Uptime: ", COLOR_CYAN_ON_BLUE);
            kprint(sec_str, COLOR_WHITE_ON_BLUE);
            kprint("s   ", COLOR_CYAN_ON_BLUE);

            set_cursor(old_x, old_y);
        }
        pic_send_eoi(irq_num);
    }
    // Keyboard - NUR den Aufruf, der Code bleibt in keyboard.c!
    else if (irq_num == 1) {
        unsigned char scancode = inb(0x60);

        if (debug_mode) {
            kprint("[", COLOR_YELLOW_ON_BLUE);
            if (scancode & 0x80) kprint("BRK:", COLOR_RED_ON_BLUE);
            else kprint("MAK:", COLOR_GREEN_ON_BLUE);

            char hex[3];
            hex[0] = "0123456789ABCDEF"[(scancode >> 4) & 0xF];
            hex[1] = "0123456789ABCDEF"[scancode & 0xF];
            hex[2] = '\0';
            kprint(hex, COLOR_WHITE_ON_BLUE);
            kprint("]", COLOR_YELLOW_ON_BLUE);
        }

        // Hier rufst du später deine keyboard_handler() auf!
        keyboard_handler();

        pic_send_eoi(irq_num);
    }
    // Andere IRQs
    else {
        pic_send_eoi(irq_num);
    }
}

// IRQ12 = Maus
void irq12_handler(void) {
    mouse_handler();
    pic_send_eoi(12); // Ende des Interrupts
}
