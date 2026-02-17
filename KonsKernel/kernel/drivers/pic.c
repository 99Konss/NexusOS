// kernel/drivers/pic.c
#include "pic.h"

// inb/outb DIREKT hier rein - kein utils.h nötig!
static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void pic_remap(int offset1, int offset2) {
    unsigned char a1, a2;

    a1 = inb(0x21);
    a2 = inb(0xA1);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, offset1);
    outb(0xA1, offset2);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, a1);
    outb(0xA1, a2);
}

void pic_send_eoi(unsigned char irq) {
    if(irq >= 8) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);
}
