// kernel/shell/shell.h
#ifndef KERNEL_SHELL_SHELL_H
#define KERNEL_SHELL_SHELL_H

#include "../kernel.h"

// Shell Konstanten
#define CMD_HISTORY_SIZE 10
#define CMD_BUFFER_SIZE 256

// Globale Variablen
extern char cmd_buffer[CMD_BUFFER_SIZE];
extern int cmd_pos;
extern char cmd_history[CMD_HISTORY_SIZE][CMD_BUFFER_SIZE];
extern int history_count;
extern int history_index;

// Funktionen
void execute_command(char *cmd);
void history_add(const char* cmd);
void show_history_command(void);

#endif
