#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>

typedef struct {
    uint32_t* address;        // Aktuelle Adresse (zum Malen)
    uint32_t* front_buffer;    // Echter Framebuffer (Hardware)
    uint32_t* back_buffer;     // Backbuffer (zum Malen)
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t pitch;
    uint32_t size;
} framebuffer_t;

extern framebuffer_t g_fb;

// Init
void framebuffer_init(void);

// Double-Buffering
void fb_init_double_buffer(void);
void fb_swap_buffers(void);

// Basis-Primitive (malen ALLE in back_buffer!)
void fb_putpixel(int x, int y, uint32_t color);
void fb_fill_rect(int x, int y, int w, int h, uint32_t color);
void fb_clear(uint32_t color);

void fb_draw_mouse_cursor(int x, int y);
void fb_erase_mouse_cursor(int x, int y);

#endif
