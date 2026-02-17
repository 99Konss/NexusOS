// kernel/drivers/screen.c
#include "screen.h"
#include "../lib/utils.h"

// Globale Variablen
int cursor_x = 0;
int cursor_y = 0;

// -----------------------------------------------------------------
// BASICS
// -----------------------------------------------------------------

void print_char_color(char c, int x, int y, unsigned char color) {
    unsigned short* buffer = (unsigned short*)VIDEO_MEMORY;
    int position = y * SCREEN_WIDTH + x;
    buffer[position] = (color << 8) | c;
}

void set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;

    unsigned short position = (y * SCREEN_WIDTH) + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(position & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((position >> 8) & 0xFF));
}

void get_cursor(int* x, int* y) {
    *x = cursor_x;
    *y = cursor_y;
}

// -----------------------------------------------------------------
// KPRINT
// -----------------------------------------------------------------

void kprint(const char* str, unsigned char color) {
    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];

        if (c == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            print_char_color(c, cursor_x, cursor_y, color);
            cursor_x++;

            if (cursor_x >= SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y++;
            }
        }
    }
    set_cursor(cursor_x, cursor_y);
}

void kprint_no_scroll(const char* str, unsigned char color) {
    int saved_x = cursor_x;
    int saved_y = cursor_y;

    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];

        if (c == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            print_char_color(c, cursor_x, cursor_y, color);
            cursor_x++;

            if (cursor_x >= SCREEN_WIDTH) {
                cursor_x = 0;
                cursor_y++;
            }
        }

        if (cursor_y >= SCREEN_HEIGHT) {
            cursor_y = SCREEN_HEIGHT - 1;
            break;
        }
    }

    cursor_x = saved_x;
    cursor_y = saved_y;
    set_cursor(cursor_x, cursor_y);
}

void kprint_at(const char* str, int x, int y, unsigned char color) {
    int saved_x = cursor_x;
    int saved_y = cursor_y;

    cursor_x = x;
    cursor_y = y;
    kprint_no_scroll(str, color);

    cursor_x = saved_x;
    cursor_y = saved_y;
    set_cursor(cursor_x, cursor_y);
}

// -----------------------------------------------------------------
// SCREEN MANAGEMENT
// -----------------------------------------------------------------

void scroll_screen(void) {
    unsigned short* buffer = (unsigned short*)VIDEO_MEMORY;

    for (int y = 0; y < SCREEN_HEIGHT - 1; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            buffer[y * SCREEN_WIDTH + x] = buffer[(y + 1) * SCREEN_WIDTH + x];
        }
    }

    unsigned short clear_char = (COLOR_DARK_GRAY << 4) | COLOR_WHITE;
    clear_char = (clear_char << 8) | ' ';

    for (int x = 0; x < SCREEN_WIDTH; x++) {
        buffer[(SCREEN_HEIGHT - 1) * SCREEN_WIDTH + x] = clear_char;
    }
}

void clear_screen(unsigned char bg_color) {
    unsigned char color = (bg_color << 4) | COLOR_WHITE;

    for (int y = 1; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            print_char_color(' ', x, y, color);
        }
    }

    for (int x = 0; x < 68; x++) {
        print_char_color(' ', x, 0, color);
    }

    cursor_x = 0;
    cursor_y = 1;
    set_cursor(cursor_x, cursor_y);
}

// -----------------------------------------------------------------
// BOOTSCREEN
// -----------------------------------------------------------------

void delay_ms(int milliseconds) {
    for(int i = 0; i < milliseconds * 1000; i++) {
        asm volatile("nop");
    }
}

void show_ascii_boot(void) {
    clear_screen(THEME_BACKGROUND);

    const char* art[] = {
        "  _  __            _  __                 _",
        " | |/ /___ _ _  __| |/ /___ _ _ _ _  ___| |",
        " | ' </ _ \\ ' \\(_-< ' </ -_) '_| ' \\/ -_) |",
        " |_|\\_\\___/_||_/__/_|\\_\\___|_| |_||_\\___|_|",
    };

    for(int i = 0; i < 4; i++) {
        kprint_at(art[i], 15, 5 + i, TXT_INFO);
        delay_ms(150);
    }

    kprint_at("Initializing", 30, 12, TXT_NORMAL);

    for(int i = 0; i < 3; i++) {
        kprint_at(".", 43 + i, 12, TXT_INFO);
        delay_ms(300);
    }

    delay_ms(500);
}
