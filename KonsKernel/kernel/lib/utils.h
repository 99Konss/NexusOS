// kernel/lib/utils.h
#ifndef KERNEL_LIB_UTILS_H
#define KERNEL_LIB_UTILS_H

#include <stdint.h>

// Port I/O
static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// NEU: outw für 16-bit Ports!
static inline void outw(unsigned short port, unsigned short val) {
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

// NEU: inw für 16-bit Lesen
static inline unsigned short inw(unsigned short port) {
    unsigned short ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// NEU: outl für 32-bit
static inline void outl(unsigned short port, unsigned int val) {
    asm volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned int inl(unsigned short port) {
    unsigned int ret;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

void delay_ms(int milliseconds);
void hex_to_string(unsigned int value, char* buffer);
void int_to_string(int num, char* str);
void hex_to_string_byte(unsigned char value, char* buffer);

#endif
