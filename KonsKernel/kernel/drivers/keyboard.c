// kernel/drivers/keyboard.c
#include "keyboard.h"
#include "screen.h"
#include "../shell/shell.h"
#include "../shell/history.h"
#include "../lib/string.h"
#include "../lib/utils.h"

extern int cursor_x;
extern int cursor_y;
extern int debug_mode;

// Tastatur Zustand
int shift_pressed = 0;
int ctrl_pressed = 0;
int alt_pressed = 0;
int caps_lock = 0;

// Deutsche Tastatur - Normal
static const char scancode_ascii_de[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
    'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// Deutsche Tastatur - Mit Shift
static const char scancode_ascii_shift_de[] = {
    0, 0, '!', '"', 0, '$', '%', '&', '/', '(', ')', '=', '?', '`', 0, 0,
    'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\'', '~', 0, '|',
    'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', '_', '/', 0, '*', 0, ' '
};

// Externe Variablen
extern char cmd_buffer[CMD_BUFFER_SIZE];
extern int cmd_pos;
extern int cursor_x;
extern int cursor_y;
extern int debug_mode;

void handle_scancode(uint8_t scancode) {
    // Nur Tastendrücke (Make), keine Releases
    if (scancode & 0x80) {
        // Release - Shift/Ctrl/Alt tracking
        uint8_t release = scancode & ~0x80;
        if (release == 0x2A || release == 0x36) shift_pressed = 0;
        if (release == 0x1D) ctrl_pressed = 0;
        if (release == 0x38) alt_pressed = 0;
        return;
    }

    // Shift/Ctrl/Alt tracking (Make)
    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; return; }
    if (scancode == 0x1D) { ctrl_pressed = 1; return; }
    if (scancode == 0x38) { alt_pressed = 1; return; }
    if (scancode == 0x9D) {ctrl_pressed = 0; return;}

    // Caps Lock
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return;
    }

    // Bei 'c' (0x2E) mit Ctrl:
    if (scancode == 0x2E && ctrl_pressed) {
        // Ctrl+C - Zeile abbrechen
        kprint("\n^C\n", COLOR_RED_ON_BLUE);
        kprint("kons> ", COLOR_WHITE_ON_BLUE);
        cmd_buffer[0] = '\0';
        cmd_pos = 0;
        set_cursor(6, cursor_y);
        return;
    }

    // Bei 'l' (0x26) mit Ctrl:
    if (scancode == 0x26 && ctrl_pressed) {
        // Ctrl+L - Screen clearen
        clear_screen(THEME_BACKGROUND);
        kprint_at("kons> ", 0, 1, COLOR_WHITE_ON_BLUE);
        cursor_x = 6;
        cursor_y = 1;
        set_cursor(cursor_x, cursor_y);
        return;
    }

    // Debug Mode - Scancodes anzeigen
    if (debug_mode) {
        kprint("[", COLOR_YELLOW_ON_BLUE);
        char hex[3];
        hex[0] = "0123456789ABCDEF"[(scancode >> 4) & 0xF];
        hex[1] = "0123456789ABCDEF"[scancode & 0xF];
        hex[2] = '\0';
        kprint(hex, COLOR_WHITE_ON_BLUE);
        kprint("]", COLOR_YELLOW_ON_BLUE);
    }

    // Pfeiltasten - History
    if (scancode == 0x48) {  // Up
        if (history_count > 0) {
            if (history_index > 0) history_index--;
            show_history_command();
        }
        return;
    }

    if (scancode == 0x50) {  // Down
        if (history_count > 0) {
            if (history_index < history_count - 1) {
                history_index++;
                show_history_command();
            } else {
                history_index = history_count;
                // Clear line
                set_cursor(6, cursor_y);
                for (int i = 0; i < cmd_pos; i++) {
                    kprint(" ", COLOR_WHITE_ON_BLUE);
                }
                set_cursor(6, cursor_y);
                cmd_buffer[0] = '\0';
                cmd_pos = 0;
            }
        }
        return;
    }

    if (scancode == 0x4B) {  // Left
        if (cursor_x > 6) {
            cursor_x--;
            set_cursor(cursor_x, cursor_y);
        }
        return;
    }

    if (scancode == 0x4D) {  // Right
        if (cursor_x < 6 + cmd_pos) {
            cursor_x++;
            set_cursor(cursor_x, cursor_y);
        }
        return;
    }

    // Normale Tasten
    char key = 0;

    // Shift oder Caps Lock für Großbuchstaben
    if (shift_pressed) {
        if (scancode < sizeof(scancode_ascii_shift_de)) {
            key = scancode_ascii_shift_de[scancode];
        }
    } else {
        if (scancode < sizeof(scancode_ascii_de)) {
            key = scancode_ascii_de[scancode];
        }
    }

    // Caps Lock für Buchstaben umschalten
    if (caps_lock && key >= 'a' && key <= 'z') {
        key = key - 'a' + 'A';
    } else if (caps_lock && key >= 'A' && key <= 'Z') {
        key = key - 'A' + 'a';
    }

    if (key != 0) {
        if (key == '\n') {
            // Enter - Command ausführen
            cmd_buffer[cmd_pos] = '\0';
            if (cmd_pos > 0) {
                history_add(cmd_buffer);
            }
            execute_command(cmd_buffer);
            cmd_pos = 0;
            history_index = history_count;
            kprint("\nkons> ", COLOR_WHITE_ON_BLUE);
            set_cursor(6, cursor_y);
        }
        else if (key == '\b') {
            // Backspace
            if (cmd_pos > 0 && cursor_x > 6) {
                cmd_pos--;
                for (int i = cursor_x - 6; i < cmd_pos; i++) {
                    cmd_buffer[i] = cmd_buffer[i + 1];
                }
                cmd_buffer[cmd_pos] = '\0';
                cursor_x--;
                for (int i = cursor_x - 6; i <= cmd_pos; i++) {
                    char c = (i < cmd_pos) ? cmd_buffer[i] : ' ';
                    print_char_color(c, 6 + i, cursor_y, COLOR_WHITE_ON_BLUE);
                }
                set_cursor(cursor_x, cursor_y);
            }
        }
        else if (key == '\t') {
            // Tab - 4 Spaces
            kprint("    ", COLOR_WHITE_ON_BLUE);
            for(int i = 0; i < 4; i++) {
                if (cmd_pos < CMD_BUFFER_SIZE - 1) {
                    cmd_buffer[cmd_pos++] = ' ';
                }
            }
            set_cursor(cursor_x, cursor_y);
        }
        else {
            // Normaler Buchstabe/Zahl
            if (cmd_pos < CMD_BUFFER_SIZE - 1) {
                if (cursor_x < 6 + cmd_pos) {
                    int insert_pos = cursor_x - 6;
                    for (int i = cmd_pos; i > insert_pos; i--) {
                        cmd_buffer[i] = cmd_buffer[i-1];
                    }
                    cmd_buffer[insert_pos] = key;
                    for (int i = insert_pos; i <= cmd_pos; i++) {
                        print_char_color(cmd_buffer[i], 6 + i, cursor_y, COLOR_WHITE_ON_BLUE);
                    }
                    cursor_x++;
                } else {
                    cmd_buffer[cmd_pos++] = key;
                    char str[2] = {key, '\0'};
                    kprint(str, COLOR_WHITE_ON_BLUE);
                }
                set_cursor(cursor_x, cursor_y);
            }
        }
    }

    if (scancode == 0x0E) {
        if (cmd_pos > 0 && cursor_x > 6) {
            cmd_pos--;

            // Buffer aktualisieren
            for (int i = cursor_x - 6; i < cmd_pos; i++) {
                cmd_buffer[i] = cmd_buffer[i + 1];
            }
            cmd_buffer[cmd_pos] = '\0';

            // Cursor zurück
            cursor_x--;

            // Zeile neu zeichnen
            for (int i = cursor_x - 6; i <= cmd_pos; i++) {
                char c = (i < cmd_pos) ? cmd_buffer[i] : ' ';
                print_char_color(c, 6 + i, cursor_y, COLOR_WHITE_ON_BLUE);
            }

            set_cursor(cursor_x, cursor_y);
        }
        return;
    }
}

// Wird von isr.c aufgerufen!
void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);
    handle_scancode(scancode);
}
