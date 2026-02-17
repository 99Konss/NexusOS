// kernel/drivers/screen.h
#ifndef KERNEL_DRIVERS_SCREEN_H
#define KERNEL_DRIVERS_SCREEN_H

// ========================
// BILDSCHIRM KONSTANTEN
// ========================
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define VIDEO_MEMORY 0xB8000

// ========================
// BASIS FARBEN
// ========================
#define COLOR_BLACK         0x00
#define COLOR_BLUE          0x01
#define COLOR_GREEN         0x02
#define COLOR_CYAN          0x03
#define COLOR_RED           0x04
#define COLOR_MAGENTA       0x05
#define COLOR_BROWN         0x06
#define COLOR_LIGHT_GRAY    0x07
#define COLOR_DARK_GRAY     0x08
#define COLOR_LIGHT_BLUE    0x09
#define COLOR_LIGHT_GREEN   0x0A
#define COLOR_LIGHT_CYAN    0x0B
#define COLOR_LIGHT_RED     0x0C
#define COLOR_LIGHT_MAGENTA 0x0D
#define COLOR_YELLOW        0x0E
#define COLOR_WHITE         0x0F

// ========================
// THEME SYSTEM
// ========================
#define THEME_BACKGROUND COLOR_DARK_GRAY
#define CURRENT_BG THEME_BACKGROUND

// Dunkles Theme (Standard)
#if THEME_BACKGROUND == COLOR_BLACK || THEME_BACKGROUND == COLOR_DARK_GRAY
    #define TXT_NORMAL    ((THEME_BACKGROUND << 4) | COLOR_WHITE)
    #define TXT_SUCCESS   ((THEME_BACKGROUND << 4) | COLOR_LIGHT_GREEN)
    #define TXT_ERROR     ((THEME_BACKGROUND << 4) | COLOR_LIGHT_RED)
    #define TXT_WARNING   ((THEME_BACKGROUND << 4) | COLOR_YELLOW)
    #define TXT_INFO      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_CYAN)
    #define TXT_BLUE      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_BLUE)
    #define TXT_MAGENTA   ((THEME_BACKGROUND << 4) | COLOR_LIGHT_MAGENTA)
    #define TXT_GRAY      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_GRAY)
    #define TXT_CYAN    ((THEME_BACKGROUND << 4) | COLOR_CYAN)
    #define TXT_YELLOW    ((THEME_BACKGROUND << 4) | COLOR_YELLOW)  // ← NEU!
#else
    // Helles Theme
    #define TXT_NORMAL    ((THEME_BACKGROUND << 4) | COLOR_WHITE)
    #define TXT_SUCCESS   ((THEME_BACKGROUND << 4) | COLOR_GREEN)
    #define TXT_ERROR     ((THEME_BACKGROUND << 4) | COLOR_RED)
    #define TXT_WARNING   ((THEME_BACKGROUND << 4) | COLOR_YELLOW)
    #define TXT_INFO      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_CYAN)
    #define TXT_BLUE      ((THEME_BACKGROUND << 4) | COLOR_BLUE)
    #define TXT_MAGENTA   ((THEME_BACKGROUND << 4) | COLOR_MAGENTA)
    #define TXT_GRAY      ((THEME_BACKGROUND << 4) | COLOR_LIGHT_GRAY)
    #define TXT_CYAN    ((THEME_BACKGROUND << 4) | COLOR_CYAN)
    #define TXT_YELLOW    ((THEME_BACKGROUND << 4) | COLOR_YELLOW)  // ← NEU!
#endif

// ========================
// BLACK BACKGROUND COLORS
// ========================
#define COLOR_WHITE_ON_BLACK    (COLOR_WHITE)
#define COLOR_GREEN_ON_BLACK    (COLOR_GREEN)
#define COLOR_RED_ON_BLACK      (COLOR_RED)
#define COLOR_CYAN_ON_BLACK     (COLOR_LIGHT_CYAN)
#define COLOR_YELLOW_ON_BLACK   (COLOR_YELLOW)
#define COLOR_BLUE_ON_BLACK     (COLOR_BLUE)
#define COLOR_MAGENTA_ON_BLACK  (COLOR_MAGENTA)

// ========================
// BLUE BACKGROUND COLORS (Kompatibilität)
// ========================
#define COLOR_WHITE_ON_BLUE     TXT_NORMAL
#define COLOR_GREEN_ON_BLUE     TXT_SUCCESS
#define COLOR_RED_ON_BLUE       TXT_ERROR
#define COLOR_YELLOW_ON_BLUE    TXT_WARNING
#define COLOR_CYAN_ON_BLUE      TXT_INFO
#define COLOR_BLUE_ON_BLUE      TXT_BLUE
#define COLOR_MAGENTA_ON_BLUE   TXT_MAGENTA
#define COLOR_BLACK_ON_BLUE     ((THEME_BACKGROUND << 4) | COLOR_BLACK)

// ========================
// DARK GRAY BACKGROUND COLORS
// ========================
#define COLOR_WHITE_ON_DARK_GRAY TXT_NORMAL
#define COLOR_GREEN_ON_DARK_GRAY TXT_SUCCESS
#define COLOR_RED_ON_DARK_GRAY   TXT_ERROR
#define COLOR_YELLOW_ON_DARK_GRAY TXT_WARNING
#define COLOR_CYAN_ON_DARK_GRAY  TXT_INFO

// ========================
// FUNKTIONEN
// ========================
void print_char_color(char c, int x, int y, unsigned char color);
void kprint(const char* str, unsigned char color);
void kprint_at(const char* str, int x, int y, unsigned char color);
void kprint_no_scroll(const char* str, unsigned char color);
void scroll_screen(void);
void clear_screen(unsigned char bg_color);
void set_cursor(int x, int y);
void get_cursor(int* x, int* y);
void show_ascii_boot(void);

// ========================
// GLOBALE VARIABLEN
// ========================
extern int cursor_x;
extern int cursor_y;

#endif
