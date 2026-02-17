// kernel/shell/history.c
#include "history.h"
#include "../drivers/screen.h"
#include "../lib/string.h"

// Command History - Definitionen
char cmd_history[CMD_HISTORY_SIZE][CMD_BUFFER_SIZE];
int history_count = 0;
int history_index = -1;

// Externe Variablen
extern int cursor_x;
extern int cursor_y;
extern char cmd_buffer[CMD_BUFFER_SIZE];
extern int cmd_pos;

void history_add(const char* cmd) {
    if (strlen(cmd) == 0) return;

    if (history_count > 0 && strcmp(cmd_history[history_count-1], cmd) == 0) {
        return;
    }

    if (history_count < CMD_HISTORY_SIZE) {
        strcpy(cmd_history[history_count], cmd);
        history_count++;
        history_index = history_count;
    } else {
        for (int i = 1; i < CMD_HISTORY_SIZE; i++) {
            strcpy(cmd_history[i-1], cmd_history[i]);
        }
        strcpy(cmd_history[CMD_HISTORY_SIZE-1], cmd);
        history_index = CMD_HISTORY_SIZE;
    }
}

void show_history_command(void) {
    if (history_index < 0 || history_index >= history_count) {
        return;
    }

    set_cursor(6, cursor_y);

    for (int i = 0; i < SCREEN_WIDTH - 6; i++) {
        print_char_color(' ', 6 + i, cursor_y, COLOR_WHITE_ON_BLUE);
    }

    set_cursor(6, cursor_y);
    kprint_no_scroll(cmd_history[history_index], COLOR_WHITE_ON_BLUE);

    strcpy(cmd_buffer, cmd_history[history_index]);
    cmd_pos = strlen(cmd_buffer);

    set_cursor(6 + cmd_pos, cursor_y);
}
