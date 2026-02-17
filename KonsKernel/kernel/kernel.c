/* kernel.c - KonsKernel v1.4.0 */
#include <stdint.h>
// ========================
// KERNEL HEADER (zentrale Structs)
// ========================
#include "kernel.h"

// ========================
// MEMORY MANAGEMENT
// ========================
#include "GUI/core/gui_core.h"
#include "memory/gdt.h"
#include "memory/isr.h"
#include "memory/heap.h"
#include "memory/idt.h"

// ========================
// DRIVERS
// ========================
#include "drivers/pic.h"
#include "drivers/screen.h"
#include "drivers/keyboard.h"
#include "drivers/pci.h"
#include "drivers/ahci.h"
#include"drivers/mouse.h"

// ========================
// FILE SYSTEM
// ========================
#include "fs/kfs.h"

// ========================
// SHELL
// ========================
#include "shell/shell.h"
#include "shell/history.h"

// ========================
// LIBRARIES
// ========================
#include "lib/string.h"
#include "lib/utils.h"

// ========================
// GUI
// ========================
#include "GUI/include/gui.h"

// ========================
// MULTIBOOT HEADER
// ========================
__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,           // Magic number
    0x00000003,           // Flags (align modules + memory map)
    -(0x1BADB002 + 0x00000003)  // Checksum
};

// ========================
// GLOBALE VARIABLEN
// ========================
int debug_mode = 0;

// Von screen.c
extern int cursor_x;
extern int cursor_y;

extern void show_ascii_boot(void);
extern void clear_screen(unsigned char bg_color);
extern void kprint_at(const char* str, int x, int y, unsigned char color);
extern void kprint_no_scroll(const char* str, unsigned char color);
extern void set_cursor(int x, int y);

// Von shell.c
extern char cmd_buffer[CMD_BUFFER_SIZE];
extern int cmd_pos;
extern char cmd_history[CMD_HISTORY_SIZE][CMD_BUFFER_SIZE];
extern int history_count;
extern int history_index;

// Von heap.c
extern unsigned int total_memory;
extern unsigned int free_memory;
extern unsigned int used_memory;
extern unsigned int kernel_memory;

extern void outb(unsigned short port, unsigned char val);
extern unsigned char inb(unsigned short port);

// ========================
// KERNEL MAIN
// ========================
void kernel_main(unsigned int magic, unsigned int addr) {
    debug_mode = 0;

    // Boot
    show_ascii_boot();
    delay_ms(1000);
    clear_screen(THEME_BACKGROUND);

    // Initialisierung - ALLES aus Modulen!
    read_multiboot_info(addr);
    init_heap();
    gdt_install();
    kfs_init();
    pic_remap(0x20, 0x28);
    isr_install();
    irq_install();
    pci_init();
    ahci_init();


    gui_init();
    mouse_init();


    gui_test();    // Rechtecke malen
    gui_set_mouse_enabled(1);  // Jetzt mit echter Maus!

    gui_run();  // Startet Hauptloop


    // PIT Timer
    outb(0x43, 0x36);
    outb(0x40, 0xFF);
    outb(0x40, 0xFF);

    // Keyboard
    while (inb(0x64) & 0x02) { }
    outb(0x64, 0xAE);
    while (inb(0x64) & 0x01) { inb(0x60); }

    // Interrupts aktivieren
    asm volatile("sti");

    // Status anzeigen
    unsigned int total_mb = total_memory / (1024 * 1024);
    kprint_at("KonsKernel v1.4.0 loaded. ", 0, 7, TXT_SUCCESS);

    char mem_str[16];
    char* ptr = mem_str;
    unsigned int n = total_mb;
    if(n == 0) *ptr++ = '0';
    else {
        char temp[16];
        int i = 0;
        while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
        while(i > 0) *ptr++ = temp[--i];
    }
    *ptr++ = 'M';
    *ptr++ = 'B';
    *ptr++ = ' ';
    *ptr++ = 'R';
    *ptr++ = 'A';
    *ptr++ = 'M';
    *ptr = '\0';
    kprint(mem_str, TXT_CYAN);

    // Shell starten
    kprint_at("kons> ", 0, 9, TXT_NORMAL);
    cursor_x = 6;
    cursor_y = 9;
    set_cursor(cursor_x, cursor_y);

    // History leeren
    for(int i = 0; i < CMD_HISTORY_SIZE; i++) {
        cmd_history[i][0] = '\0';
    }
    history_count = 0;
    history_index = -1;

    // Hauptschleife
    while(1) {
        asm volatile("hlt");
    }
}
