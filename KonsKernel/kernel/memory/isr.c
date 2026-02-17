#include "isr.h"
#include "idt.h"
#include "../drivers/screen.h"  // ← WICHTIG!
#include "../drivers/pic.h"
#include "../kernel.h"          // ← für struct regs
#include "../drivers/keyboard.h"

#define COLOR_YELLOW        0x0E
#define COLOR_YELLOW_ON_BLUE ((THEME_BACKGROUND << 4) | COLOR_YELLOW)

extern int debug_mode;
extern int debug_mode;  // kommt aus kernel.h/main.c

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(unsigned short port, unsigned char val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void isr_handler(struct regs *r) {
    if (r->int_no >= 32) {
        if (r->int_no >= 32 && r->int_no <= 47) {
            pic_send_eoi(r->int_no - 32);
        }
        return;
    }

    // CPU Exception
    kprint("\n[CPU EXCEPTION] INT 0x", COLOR_RED_ON_BLUE);

    char hex[5];
    hex[0] = '0';
    hex[1] = 'x';
    hex[2] = "0123456789ABCDEF"[(r->int_no >> 4) & 0xF];
    hex[3] = "0123456789ABCDEF"[r->int_no & 0xF];
    hex[4] = '\0';
    kprint(hex, COLOR_WHITE_ON_BLUE);

    if (r->int_no == 0) kprint(" (Division by zero)\n", COLOR_RED_ON_BLUE);
    else if (r->int_no == 8) kprint(" (Double Fault)\n", COLOR_RED_ON_BLUE);
    else if (r->int_no == 13) kprint(" (General Protection)\n", COLOR_RED_ON_BLUE);
    else if (r->int_no == 14) kprint(" (Page Fault)\n", COLOR_RED_ON_BLUE);
    else kprint("\n", COLOR_RED_ON_BLUE);
}
