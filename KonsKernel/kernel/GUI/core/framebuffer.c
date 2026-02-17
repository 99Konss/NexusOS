#include "framebuffer.h"
#include "../../drivers/screen.h"
#include "../../memory/heap.h"
#include "../../lib/utils.h"

framebuffer_t g_fb;

// VESA/VBE Modi
#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA  0x01CF

#define VBE_DISPI_INDEX_ID      0
#define VBE_DISPI_INDEX_XRES    1
#define VBE_DISPI_INDEX_YRES    2
#define VBE_DISPI_INDEX_BPP     3
#define VBE_DISPI_INDEX_ENABLE  4
#define VBE_DISPI_INDEX_BANK    5
#define VBE_DISPI_INDEX_VIRT_WIDTH 6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 7
#define VBE_DISPI_INDEX_X_OFFSET 8
#define VBE_DISPI_INDEX_Y_OFFSET 9

#define VBE_DISPI_ENABLED       0x01
#define VBE_DISPI_LFB_ENABLED   0x40



static void vbe_write(uint16_t index, uint16_t value) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}

// Mauscursor als 12x12 Pixel mit Schatten
static const uint8_t mouse_cursor[] = {
    0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void fb_draw_mouse_cursor(int x, int y) {
    int cursor_size = 12;
    uint32_t color = 0xFFFFFF; // Weiß

    for (int j = 0; j < cursor_size; j++) {
        for (int i = 0; i < cursor_size; i++) {
            if (mouse_cursor[j * cursor_size + i]) {
                int px = x + i;
                int py = y + j;
                if (px >= 0 && px < g_fb.width && py >= 0 && py < g_fb.height) {
                    uint32_t* pixel = g_fb.address + (py * g_fb.width + px);
                    *pixel = color;
                }
            }
        }
    }
}

void framebuffer_init(void) {
    kprint("Initializing framebuffer... ", COLOR_WHITE_ON_BLUE);

    // VBE initialisieren
    vbe_write(VBE_DISPI_INDEX_ID, 0xB0C5);  // Magic number für VBE
    vbe_write(VBE_DISPI_INDEX_XRES, 1024);  // Breite
    vbe_write(VBE_DISPI_INDEX_YRES, 768);   // Höhe
    vbe_write(VBE_DISPI_INDEX_BPP, 32);     // 32 Bit Farbtiefe
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

    // Framebuffer-Info setzen
    g_fb.address = (uint32_t*)0xFD000000;   // Typische LFB-Adresse
    g_fb.width = 1024;
    g_fb.height = 768;
    g_fb.bpp = 32;
    g_fb.pitch = g_fb.width * (g_fb.bpp / 8);
    g_fb.size = g_fb.pitch * g_fb.height;

    kprint("OK\n", COLOR_GREEN_ON_BLUE);

    char buf[16];
    kprint("  Resolution: ", COLOR_WHITE_ON_BLUE);
    int_to_string(g_fb.width, buf);
    kprint(buf, COLOR_CYAN_ON_BLUE);
    kprint("x", COLOR_WHITE_ON_BLUE);
    int_to_string(g_fb.height, buf);
    kprint(buf, COLOR_CYAN_ON_BLUE);
    kprint("\n", COLOR_WHITE_ON_BLUE);

    // Test: Bildschirm blau färben
    fb_clear(0x0000FF); // Blau
}

void fb_putpixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= g_fb.width || y < 0 || y >= g_fb.height) return;

    uint32_t* pixel = g_fb.address + (y * g_fb.width + x);
    *pixel = color;
}

void fb_fill_rect(int x, int y, int w, int h, uint32_t color) {
    // Clipping
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > g_fb.width) w = g_fb.width - x;
    if (y + h > g_fb.height) h = g_fb.height - y;

    if (w <= 0 || h <= 0) return;

    for (int j = 0; j < h; j++) {
        uint32_t* line = g_fb.address + ((y + j) * g_fb.width + x);
        for (int i = 0; i < w; i++) {
            line[i] = color;
        }
    }
}

void fb_clear(uint32_t color) {
    fb_fill_rect(0, 0, g_fb.width, g_fb.height, color);
}

// NEU: Double-Buffering Initialisierung
void fb_init_double_buffer(void) {
    kprint("Initializing double buffer... ", COLOR_WHITE_ON_BLUE);

    // Backbuffer allozieren (4096-byte aligned für Performance)
    g_fb.back_buffer = (uint32_t*)malloc_aligned(g_fb.size, 4096);

    if (!g_fb.back_buffer) {
        kprint("FAILED - out of memory!\n", COLOR_RED_ON_BLUE);
        return;
    }

    // Frontbuffer merken (echter Framebuffer)
    g_fb.front_buffer = g_fb.address;

    // Zum Malen erstmal auf Backbuffer zeigen
    g_fb.address = g_fb.back_buffer;

    // Backbuffer mit Schwarz initialisieren
    fb_clear(0x000000);

    // Einmal swap damit Frontbuffer auch schwarz ist
    fb_swap_buffers();

    kprint("OK\n", COLOR_GREEN_ON_BLUE);

    char buf[16];
    int_to_string(g_fb.size / (1024*1024), buf);
    kprint("  Backbuffer: ", COLOR_WHITE_ON_BLUE);
    kprint(buf, COLOR_CYAN_ON_BLUE);
    kprint(" MB allocated\n", COLOR_WHITE_ON_BLUE);
}

// NEU: Buffer tauschen
void fb_swap_buffers(void) {
    if (!g_fb.back_buffer || !g_fb.front_buffer) return;

    // Backbuffer in Frontbuffer kopieren
    uint32_t* src = g_fb.back_buffer;
    uint32_t* dst = g_fb.front_buffer;

    // Schnelles Kopieren (32-Bit Blöcke)
    for (uint32_t i = 0; i < g_fb.size / 4; i++) {
        dst[i] = src[i];
    }

    // Für später: Nur geänderte Bereiche kopieren (Optimierung)
}

void fb_erase_mouse_cursor(int x, int y) {
    int cursor_size = 12;

    for (int j = 0; j < cursor_size; j++) {
        for (int i = 0; i < cursor_size; i++) {
            if (mouse_cursor[j * cursor_size + i]) {
                int px = x + i;
                int py = y + j;
                if (px >= 0 && px < g_fb.width && py >= 0 && py < g_fb.height) {
                    uint32_t* pixel = g_fb.address + (py * g_fb.width + px);
                    *pixel = 0x000000; // Schwarz (später: Hintergrund speichern!)
                }
            }
        }
    }
}
