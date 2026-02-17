// kernel/shell/history.h
#ifndef KERNEL_SHELL_HISTORY_H
#define KERNEL_SHELL_HISTORY_H

#include "shell.h"

// Command History
extern char cmd_history[CMD_HISTORY_SIZE][CMD_BUFFER_SIZE];
extern int history_count;
extern int history_index;

// History Funktionen
void history_add(const char* cmd);
void show_history_command(void);

#endif
