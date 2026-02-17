// kernel/shell/commands.h
#ifndef KERNEL_SHELL_COMMANDS_H
#define KERNEL_SHELL_COMMANDS_H

// Command Funktionen
void cmd_help(void);
void cmd_clear(void);
void cmd_echo(char* text);
void cmd_info(void);
void cmd_mem(void);
void cmd_ls(void);
void cmd_touch(char* filename);
void cmd_cat(char* filename);
void cmd_rm(char* filename);
void cmd_debug(void);
void pci_scan(void);
void unknown_command(char* cmd);


#endif
