// kernel/shell/shell.c
#include "shell.h"
#include "commands.h"
#include "history.h"
#include "../lib/string.h"
#include "../drivers/acpi.h"

void cmd_reboot(void);
void cmd_shutdown(void);

char cmd_buffer[CMD_BUFFER_SIZE];
int cmd_pos = 0;

void execute_command(char *cmd) {
    while(*cmd == ' ') cmd++;
    if(*cmd == '\0') return;

    if(strcmp(cmd, "help") == 0) cmd_help();
    else if(strcmp(cmd, "clear") == 0) cmd_clear();
    else if(strcmp(cmd, "info") == 0) cmd_info();
    else if(strcmp(cmd, "mem") == 0 || strcmp(cmd, "memory") == 0) cmd_mem();
    else if(strcmp(cmd, "ls") == 0 || strcmp(cmd, "dir") == 0) cmd_ls();
    else if(strcmp(cmd, "debug") == 0) cmd_debug();
    else if(strstart(cmd, "echo ")) cmd_echo(cmd + 5);
    else if(strstart(cmd, "touch ")) cmd_touch(cmd + 6);
    else if(strstart(cmd, "cat ")) cmd_cat(cmd + 4);
    else if(strstart(cmd, "rm ")) cmd_rm(cmd + 3);
    else if (strcmp(cmd, "reboot") == 0) cmd_reboot();
    else if (strcmp(cmd, "shutdown") == 0) cmd_shutdown();
    else if (strcmp(cmd, "pci") == 0) pci_scan();
    else unknown_command(cmd);
}
