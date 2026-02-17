// kernel/fs/kfs.h
#ifndef KERNEL_FS_KFS_H
#define KERNEL_FS_KFS_H

#include <stdint.h>
#include "../kernel.h"

// ========================
// KFS KONSTANTEN
// ========================

#define KFS_MAGIC 0x4B46531A      // "KFS" + 0x1A
#define BLOCK_SIZE 512            // Wie echte Disks
#define MAX_FILES 64             // Maximale Dateien
#define MAX_NAME_LEN 28          // Dateinamenlänge
#define MAX_BLOCKS_PER_FILE 16   // Max Blöcke pro Datei
#define RAMDISK_SIZE (4 * 1024 * 1024)  // 4MB RAM-Disk

// ========================
// KFS STRUKTUREN
// ========================

struct kfs_superblock {
    uint32_t magic;           // 0x4B46531A
    uint32_t total_blocks;    // Gesamtblöcke
    uint32_t free_blocks;     // Freie Blöcke
    uint32_t inode_count;     // Anzahl INodes
    uint32_t block_size;      // Sollte 512 sein
    char volume_name[32];     // Volume Name
};

struct kfs_inode {
    uint32_t id;              // INode Nummer
    char name[MAX_NAME_LEN];  // Dateiname
    uint32_t size;            // Dateigröße
    uint32_t blocks[16];      // Block-Pointer
    uint8_t type;             // 1=Datei, 2=Verzeichnis
    uint32_t parent;          // Eltern-INode
    uint32_t created;         // Erstellungszeit
    uint32_t modified;        // Änderungszeit
};

struct kfs_dir_entry {
    uint32_t inode_id;        // Verweis auf INode
    char name[MAX_NAME_LEN];  // Eintragsname
};

// ========================
// GLOBALE VARIABLEN (extern)
// ========================

extern struct kfs_superblock* superblock;
extern struct kfs_inode* inode_table;
extern uint8_t* block_bitmap;
extern uint8_t* data_blocks;
extern uint32_t current_dir_inode;
extern uint8_t ramdisk[RAMDISK_SIZE];

// ========================
// FUNKTIONEN
// ========================

void kfs_init(void);
void kfs_format(const char* volume_name);
int kfs_create(const char* name, uint8_t type);
int kfs_write(int inode_idx, const void* data, uint32_t size);
int kfs_read(int inode_idx, void* buffer, uint32_t size);
int kfs_delete(const char* name);
int find_file(const char* name);
int find_free_inode(void);
int find_free_block(void);
void free_block(int block_idx);

#endif
