// kernel/shell/commands.c - VOLLSTÄNDIG!
#include "commands.h"
#include "../drivers/screen.h"
#include "../memory/heap.h"
#include "../fs/kfs.h"
#include "../lib/string.h"
#include "../drivers/acpi.h"
#include "../drivers/pci.h"

extern int debug_mode;
extern struct kfs_inode* inode_table;
extern uint32_t current_dir_inode;
extern struct kfs_superblock* superblock;

// ========================
// HELP COMMAND
// ========================
void cmd_help(void) {
    kprint("\n=== KonsShell Commands ===\n", TXT_INFO);
    kprint("help     - Show this help\n", TXT_SUCCESS);
    kprint("clear    - Clear screen\n", TXT_SUCCESS);
    kprint("echo     - Print text\n", TXT_SUCCESS);
    kprint("info     - System information\n", TXT_SUCCESS);
    kprint("mem      - Memory information\n", TXT_SUCCESS);
    kprint("mtest    - Test memory allocation\n", TXT_SUCCESS);
    kprint("mdebug   - Memory debug info\n", TXT_SUCCESS);
    kprint("color    - Change text color\n", TXT_SUCCESS);
    kprint("reboot   - Reboot system\n", TXT_WARNING);
    kprint("shutdown - Shutdown system\n", TXT_WARNING);
    kprint("about    - About KonsKernel\n", TXT_SUCCESS);
    kprint("debug    - Toggle debug mode\n", TXT_WARNING);
    kprint("ls/dir   - List files\n", TXT_SUCCESS);
    kprint("touch    - Create file\n", TXT_SUCCESS);
    kprint("mkdir    - Create directory\n", TXT_SUCCESS);
    kprint("cat      - Show file\n", TXT_SUCCESS);
    kprint("rm       - Delete file\n", TXT_SUCCESS);
    kprint("fsinfo/df- Filesystem info\n", TXT_SUCCESS);
    kprint("format   - Format filesystem\n", TXT_ERROR);
}

// ========================
// ABOUT COMMAND
// ========================
void cmd_about(void) {
    kprint("============= KonsKernel =============\n", TXT_INFO);
    kprint("This is a nice Hobby Kernel made only by me\n", TXT_NORMAL);
    kprint("This kernel is made with love, energy drinks and curiosity\n", TXT_NORMAL);
    kprint("Your data stays on your device. ", TXT_NORMAL);
    kprint("Type 'help' for available commands\n", TXT_NORMAL);
}

// ========================
// CLEAR COMMAND
// ========================
void cmd_clear(void) {
    clear_screen(THEME_BACKGROUND);
    set_cursor(6, 1);
}

// ========================
// ECHO COMMAND (with Redirection)
// ========================
void cmd_echo(char* text) {
    // Suche nach " > " für Redirection
    char* redirect_pos = NULL;
    char* current_pos = text;

    while(*current_pos) {
        if(current_pos[0] == ' ' && current_pos[1] == '>' && current_pos[2] == ' ') {
            redirect_pos = current_pos;
            break;
        }
        current_pos++;
    }

    if(redirect_pos != NULL) {
        *redirect_pos = '\0';
        char* filename = redirect_pos + 3;
        while(*filename == ' ') filename++;

        if(*filename == '\0') {
            kprint("Error: Missing filename after '>'\n", TXT_ERROR);
        } else {
            int inode_idx = find_file(filename);
            if(inode_idx == -1) {
                inode_idx = kfs_create(filename, 1);
            }
            if(inode_idx != -1) {
                kfs_write(inode_idx, text, strlen(text));
                kprint("Written to file: '", TXT_SUCCESS);
                kprint(filename, TXT_INFO);
                kprint("'\n", TXT_SUCCESS);
            } else {
                kprint("Error: Could not create/write file\n", TXT_ERROR);
            }
        }
    } else {
        kprint("\n", TXT_NORMAL);
        kprint(text, TXT_CYAN);
        kprint("\n", TXT_NORMAL);
    }
}

// ========================
// INFO COMMAND
// ========================
void cmd_info(void) {
    kprint("\n=== System Information ===\n", TXT_INFO);
    kprint("Kernel:   KonsKernel v1.4.0\n", TXT_NORMAL);
    kprint("Shell:    KonsShell v1.1\n", TXT_NORMAL);
    kprint("Arch:     32-bit Protected Mode\n", TXT_NORMAL);
    kprint("Interrupts: Enabled\n", TXT_SUCCESS);
    kprint("Memory:   Full Memory Management\n", TXT_SUCCESS);
    kprint("Heap:     1MB available\n", TXT_NORMAL);
    kprint("Display:  80x25 VGA Text\n", TXT_NORMAL);
}

// ========================
// MEMORY COMMAND
// ========================
void cmd_mem(void) {
    print_memory_info();
}

// ========================
// MEMORY TEST COMMAND
// ========================
void cmd_mtest(void) {
    kprint("\n=== Memory Allocation Test ===\n", TXT_INFO);

    kprint("Test 1: Allocating 64 bytes... ", TXT_NORMAL);
    void* ptr1 = kmalloc_safe(64);
    if(ptr1) kprint("[OK]\n", TXT_SUCCESS);
    else kprint("[FAILED]\n", TXT_ERROR);

    kprint("Test 2: Allocating 1024 bytes... ", TXT_NORMAL);
    void* ptr2 = kmalloc_safe(1024);
    if(ptr2) kprint("[OK]\n", TXT_SUCCESS);
    else kprint("[FAILED]\n", TXT_ERROR);

    kprint("Test 3: Freeing allocations... ", TXT_NORMAL);
    if(ptr1) kfree_safe(ptr1);
    if(ptr2) kfree_safe(ptr2);
    kprint("[DONE]\n", TXT_SUCCESS);

    print_memory_info();
}

// ========================
// MEMORY DEBUG COMMAND
// ========================
void cmd_mdebug(void) {
    debug_memory();
}

// ========================
// REBOOT COMMAND
// ========================
void cmd_reboot(void) {
    kprint("\nRebooting...\n", TXT_WARNING);
    acpi_reboot();
}

// ========================
// SHUTDOWN COMMAND
// ========================
void cmd_shutdown(void) {
    kprint("\nShutting down...\n", TXT_WARNING);
    acpi_shutdown();
}

// ========================
// DEBUG COMMAND
// ========================
void cmd_debug(void) {
    debug_mode = !debug_mode;
    kprint("\nDebug mode: ", TXT_INFO);
    if(debug_mode) {
        kprint("ON\n", TXT_SUCCESS);
    } else {
        kprint("OFF\n", TXT_ERROR);
    }
}

// ========================
// LS COMMAND
// ========================
void cmd_ls(void) {
    kprint("\n", TXT_NORMAL);

    int count = 0;
    for(int i = 1; i < MAX_FILES; i++) {
        if(inode_table[i].id != 0 && inode_table[i].parent == current_dir_inode) {
            if(inode_table[i].type == 2) {
                kprint("[DIR]  ", TXT_CYAN);
            } else {
                kprint("[FILE] ", TXT_SUCCESS);
            }

            kprint(inode_table[i].name, TXT_NORMAL);

            if(inode_table[i].type == 1) {
                kprint(" (", TXT_NORMAL);
                char size_str[16];
                char* ptr = size_str;
                uint32_t n = inode_table[i].size;
                if(n == 0) *ptr++ = '0';
                else {
                    char temp[16];
                    int j = 0;
                    while(n > 0) { temp[j++] = '0' + (n % 10); n /= 10; }
                    while(j > 0) *ptr++ = temp[--j];
                }
                *ptr++ = 'B';
                *ptr = '\0';
                kprint(size_str, TXT_WARNING);
                kprint(")", TXT_NORMAL);
            }
            kprint("\n", TXT_NORMAL);
            count++;
        }
    }

    if(count == 0) {
        kprint("Directory empty\n", TXT_WARNING);
    } else {
        kprint("\nTotal: ", TXT_NORMAL);
        char count_str[16];
        char* ptr = count_str;
        int n = count;
        if(n == 0) *ptr++ = '0';
        else {
            char temp[16];
            int i = 0;
            while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
            while(i > 0) *ptr++ = temp[--i];
        }
        *ptr++ = ' ';
        *ptr++ = 'e';
        *ptr++ = 'n';
        *ptr++ = 't';
        *ptr++ = 'r';
        *ptr++ = 'i';
        *ptr++ = 'e';
        *ptr++ = 's';
        *ptr = '\0';
        kprint(count_str, TXT_INFO);
        kprint("\n", TXT_NORMAL);
    }
}

// ========================
// TOUCH COMMAND
// ========================
void cmd_touch(char* filename) {
    while(*filename == ' ') filename++;

    if(*filename == '\0') {
        kprint("Usage: touch <filename>\n", TXT_ERROR);
    } else {
        int result = kfs_create(filename, 1);
        if(result == -1) {
            kprint("Error: Could not create file '", TXT_ERROR);
            kprint(filename, TXT_NORMAL);
            kprint("'\n", TXT_ERROR);
        } else {
            kprint("File created: '", TXT_SUCCESS);
            kprint(filename, TXT_INFO);
            kprint("'\n", TXT_SUCCESS);
        }
    }
}

// ========================
// MKDIR COMMAND
// ========================
void cmd_mkdir(char* dirname) {
    while(*dirname == ' ') dirname++;

    if(*dirname == '\0') {
        kprint("Usage: mkdir <dirname>\n", TXT_ERROR);
    } else {
        int result = kfs_create(dirname, 2);
        if(result == -1) {
            kprint("Error creating directory: ", TXT_ERROR);
            kprint(dirname, TXT_NORMAL);
            kprint("\n", TXT_ERROR);
        } else {
            kprint("Directory created: ", TXT_SUCCESS);
            kprint(dirname, TXT_INFO);
            kprint("\n", TXT_SUCCESS);
        }
    }
}

// ========================
// CAT COMMAND
// ========================
void cmd_cat(char* filename) {
    while(*filename == ' ') filename++;

    if(*filename == '\0') {
        kprint("Usage: cat <filename>\n", TXT_ERROR);
    } else {
        int inode_idx = find_file(filename);
        if(inode_idx == -1) {
            kprint("File not found: ", TXT_ERROR);
            kprint(filename, TXT_NORMAL);
            kprint("\n", TXT_ERROR);
        } else {
            kprint("\n", TXT_NORMAL);
            char buffer[4096];
            int bytes_read = kfs_read(inode_idx, buffer, sizeof(buffer));
            if(bytes_read > 0) {
                if(bytes_read < sizeof(buffer)) {
                    buffer[bytes_read] = '\0';
                } else {
                    buffer[sizeof(buffer)-1] = '\0';
                }
                kprint(buffer, TXT_NORMAL);
            }
            kprint("\n", TXT_NORMAL);
        }
    }
}

// ========================
// RM COMMAND
// ========================
void cmd_rm(char* filename) {
    while(*filename == ' ') filename++;

    if(*filename == '\0') {
        kprint("Usage: rm <filename>\n", TXT_ERROR);
    } else {
        int result = kfs_delete(filename);
        if(result == -1) {
            kprint("Error deleting file: ", TXT_ERROR);
            kprint(filename, TXT_NORMAL);
            kprint("\n", TXT_ERROR);
        } else {
            kprint("File deleted: ", TXT_SUCCESS);
            kprint(filename, TXT_INFO);
            kprint("\n", TXT_SUCCESS);
        }
    }
}

// ========================
// FSINFO COMMAND
// ========================
void cmd_fsinfo(void) {
    kprint("\n=== KFS Information ===\n", TXT_INFO);

    kprint("Volume:     ", TXT_NORMAL);
    kprint(superblock->volume_name, TXT_INFO);
    kprint("\n", TXT_NORMAL);

    uint32_t total_kb = (superblock->total_blocks * BLOCK_SIZE) / 1024;
    uint32_t free_kb = (superblock->free_blocks * BLOCK_SIZE) / 1024;
    uint32_t used_kb = total_kb - free_kb;

    kprint("Total:      ", TXT_NORMAL);
    char num_str[32];
    char* ptr = num_str;
    uint32_t n = total_kb;
    if(n == 0) *ptr++ = '0';
    else {
        char temp[32];
        int i = 0;
        while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
        while(i > 0) *ptr++ = temp[--i];
    }
    *ptr++ = 'K'; *ptr++ = 'B'; *ptr = '\0';
    kprint(num_str, TXT_NORMAL);
    kprint("\n", TXT_NORMAL);

    kprint("Free:       ", TXT_NORMAL);
    ptr = num_str;
    n = free_kb;
    if(n == 0) *ptr++ = '0';
    else {
        char temp[32];
        int i = 0;
        while(n > 0) { temp[i++] = '0' + (n % 10); n /= 10; }
        while(i > 0) *ptr++ = temp[--i];
    }
    *ptr++ = 'K'; *ptr++ = 'B'; *ptr = '\0';
    kprint(num_str, free_kb > (total_kb/2) ? TXT_SUCCESS : TXT_WARNING);
    kprint("\n", TXT_NORMAL);
}

// ========================
// UNKNOWN COMMAND
// ========================
void unknown_command(char* cmd) {
    kprint("\nUnknown command: '", TXT_ERROR);
    kprint(cmd, TXT_NORMAL);
    kprint("'\n", TXT_ERROR);
    kprint("Type 'help' for available commands\n", TXT_ERROR);
}

void pci_scan(void) {
    pci_scan_and_print();
}
