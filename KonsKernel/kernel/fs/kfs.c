// kernel/fs/kfs.c
// MAKE STRING AFTER SHELL
#include "kfs.h"
#include "../drivers/screen.h"
#include "../lib/string.h"
#include <stddef.h>

// RAM-Disk
uint8_t ramdisk[RAMDISK_SIZE];

// KFS Globale Variablen
struct kfs_superblock* superblock = (struct kfs_superblock*)ramdisk;
struct kfs_inode* inode_table = (struct kfs_inode*)(ramdisk + BLOCK_SIZE);
uint8_t* block_bitmap = (uint8_t*)(ramdisk + BLOCK_SIZE + (MAX_FILES * sizeof(struct kfs_inode)));
uint8_t* data_blocks = ramdisk + (BLOCK_SIZE * 10);
uint32_t current_dir_inode = 1;

void kfs_init(void) {
    if(superblock->magic != KFS_MAGIC) {
        kfs_format("KonsKernelFS");
    } else {
        kprint("[OK]\n", COLOR_GREEN_ON_BLUE);
        kprint("Volume: ", COLOR_WHITE_ON_BLUE);
        kprint(superblock->volume_name, COLOR_CYAN_ON_BLUE);
        kprint("\n", COLOR_WHITE_ON_BLUE);
    }
    current_dir_inode = 1;
}

void kfs_format(const char* volume_name) {
    kprint("\n", COLOR_YELLOW_ON_BLUE);

    superblock->magic = KFS_MAGIC;
    superblock->total_blocks = RAMDISK_SIZE / BLOCK_SIZE;
    superblock->free_blocks = superblock->total_blocks - 10;
    superblock->inode_count = MAX_FILES;
    superblock->block_size = BLOCK_SIZE;

    int i = 0;
    while(volume_name[i] && i < 31) {
        superblock->volume_name[i] = volume_name[i];
        i++;
    }
    superblock->volume_name[i] = '\0';

    for(i = 0; i < MAX_FILES; i++) {
        inode_table[i].id = 0;
        inode_table[i].name[0] = '\0';
        inode_table[i].size = 0;
        inode_table[i].type = 0;
        inode_table[i].parent = 0;
        inode_table[i].created = 0;
        inode_table[i].modified = 0;
        for(int j = 0; j < 16; j++) {
            inode_table[i].blocks[j] = 0;
        }
    }

    uint32_t bitmap_size = superblock->total_blocks / 8;
    for(i = 0; i < bitmap_size; i++) {
        block_bitmap[i] = 0xFF;
    }

    uint32_t meta_blocks = 10;
    for(i = 0; i < meta_blocks; i++) {
        int byte = i / 8;
        int bit = i % 8;
        block_bitmap[byte] &= ~(1 << bit);
        superblock->free_blocks--;
    }

    inode_table[1].id = 1;
    inode_table[1].type = 2;
    inode_table[1].parent = 1;
    inode_table[1].created = 123456;
    inode_table[1].modified = 123456;
    inode_table[1].name[0] = '/';
    inode_table[1].name[1] = '\0';

    kprint("\nKFS formatted successfully!\n", COLOR_GREEN_ON_BLUE);
    kprint("Total blocks: ", COLOR_WHITE_ON_BLUE);

    char blocks_str[16];
    char* ptr = blocks_str;
    uint32_t n = superblock->total_blocks;
    if(n == 0) *ptr++ = '0';
    else {
        char temp[16];
        int j = 0;
        while(n > 0) { temp[j++] = '0' + (n % 10); n /= 10; }
        while(j > 0) *ptr++ = temp[--j];
    }
    *ptr = '\0';
    kprint(blocks_str, COLOR_CYAN_ON_BLUE);
    kprint(" (", COLOR_WHITE_ON_BLUE);

    ptr = blocks_str;
    n = superblock->free_blocks;
    if(n == 0) *ptr++ = '0';
    else {
        char temp[16];
        int j = 0;
        while(n > 0) { temp[j++] = '0' + (n % 10); n /= 10; }
        while(j > 0) *ptr++ = temp[--j];
    }
    *ptr++ = ' ';
    *ptr++ = 'f';
    *ptr++ = 'r';
    *ptr++ = 'e';
    *ptr++ = 'e';
    *ptr = '\0';

    kprint(blocks_str, COLOR_GREEN_ON_BLUE);
    kprint(")\n", COLOR_WHITE_ON_BLUE);
}

int find_free_block(void) {
    uint32_t total_blocks = superblock->total_blocks;

    for(uint32_t i = 0; i < total_blocks; i++) {
        int byte = i / 8;
        int bit = i % 8;

        if(block_bitmap[byte] & (1 << bit)) {
            block_bitmap[byte] &= ~(1 << bit);
            superblock->free_blocks--;
            return i;
        }
    }
    return -1;
}

void free_block(int block_idx) {
    if(block_idx < 0) return;

    int byte = block_idx / 8;
    int bit = block_idx % 8;

    block_bitmap[byte] |= (1 << bit);
    superblock->free_blocks++;
}

int find_free_inode(void) {
    for(int i = 1; i < MAX_FILES; i++) {
        if(inode_table[i].id == 0) {
            return i;
        }
    }
    return -1;
}

int find_file(const char* name) {
    for(int i = 1; i < MAX_FILES; i++) {
        if(inode_table[i].id != 0 &&
           inode_table[i].parent == current_dir_inode) {
            if(strcmp(inode_table[i].name, name) == 0) {
                return i;
            }
        }
    }
    return -1;
}

int kfs_create(const char* name, uint8_t type) {
    kprint("Creating: ", COLOR_WHITE_ON_BLUE);
    kprint(name, COLOR_CYAN_ON_BLUE);
    kprint("... ", COLOR_WHITE_ON_BLUE);

    if(find_file(name) != -1) {
        kprint("[FAILED - exists]\n", COLOR_RED_ON_BLUE);
        return -1;
    }

    int inode_idx = find_free_inode();
    if(inode_idx == -1) {
        kprint("[FAILED - no inodes]\n", COLOR_RED_ON_BLUE);
        return -1;
    }

    struct kfs_inode* inode = &inode_table[inode_idx];
    inode->id = inode_idx;
    inode->type = type;
    inode->parent = current_dir_inode;
    inode->size = 0;
    inode->created = 123456;
    inode->modified = 123456;

    int i = 0;
    while(name[i] && i < MAX_NAME_LEN - 1) {
        inode->name[i] = name[i];
        i++;
    }
    inode->name[i] = '\0';

    if(type == 2) {
        int block = find_free_block();
        if(block == -1) {
            inode->id = 0;
            kprint("[FAILED - no blocks]\n", COLOR_RED_ON_BLUE);
            return -1;
        }

        inode->blocks[0] = block;

        struct kfs_dir_entry* dir = (struct kfs_dir_entry*)(data_blocks + (block * BLOCK_SIZE));

        dir[0].inode_id = inode_idx;
        dir[0].name[0] = '.';
        dir[0].name[1] = '\0';

        dir[1].inode_id = current_dir_inode;
        dir[1].name[0] = '.';
        dir[1].name[1] = '.';
        dir[1].name[2] = '\0';

        for(i = 2; i < BLOCK_SIZE / sizeof(struct kfs_dir_entry); i++) {
            dir[i].inode_id = 0;
            dir[i].name[0] = '\0';
        }

        inode->size = 2 * sizeof(struct kfs_dir_entry);
    }

    kprint("[OK]\n", COLOR_GREEN_ON_BLUE);
    return inode_idx;
}

int kfs_write(int inode_idx, const void* data, uint32_t size) {
    if(inode_idx <= 0 || inode_idx >= MAX_FILES) return -1;

    struct kfs_inode* inode = &inode_table[inode_idx];
    if(inode->id == 0) return -1;

    for(int i = 0; i < 16; i++) {
        if(inode->blocks[i] != 0) {
            free_block(inode->blocks[i]);
            inode->blocks[i] = 0;
        }
    }

    uint32_t blocks_needed = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if(blocks_needed > 16) {
        kprint("File too large! (max 8KB)\n", COLOR_RED_ON_BLUE);
        return -1;
    }

    const uint8_t* src = (const uint8_t*)data;
    uint32_t written = 0;

    for(uint32_t i = 0; i < blocks_needed; i++) {
        int block = find_free_block();
        if(block == -1) {
            kprint("Out of disk space!\n", COLOR_RED_ON_BLUE);
            return written;
        }

        inode->blocks[i] = block;

        uint32_t to_copy = size - written;
        if(to_copy > BLOCK_SIZE) to_copy = BLOCK_SIZE;

        uint8_t* dst = data_blocks + (block * BLOCK_SIZE);
        for(uint32_t j = 0; j < to_copy; j++) {
            dst[j] = src[written + j];
        }

        written += to_copy;
    }

    inode->size = size;
    inode->modified = 123456;

    return written;
}

int kfs_read(int inode_idx, void* buffer, uint32_t size) {
    if(inode_idx <= 0 || inode_idx >= MAX_FILES) return -1;

    struct kfs_inode* inode = &inode_table[inode_idx];
    if(inode->id == 0) return -1;

    if(size > inode->size) size = inode->size;

    uint8_t* dst = (uint8_t*)buffer;
    uint32_t read = 0;

    for(int i = 0; i < 16 && read < size; i++) {
        if(inode->blocks[i] == 0) break;

        uint8_t* src = data_blocks + (inode->blocks[i] * BLOCK_SIZE);
        uint32_t to_read = size - read;
        if(to_read > BLOCK_SIZE) to_read = BLOCK_SIZE;

        for(uint32_t j = 0; j < to_read; j++) {
            dst[read + j] = src[j];
        }

        read += to_read;
    }

    return read;
}

int kfs_delete(const char* name) {
    kprint("Deleting: ", COLOR_WHITE_ON_BLUE);
    kprint(name, COLOR_CYAN_ON_BLUE);
    kprint("... ", COLOR_WHITE_ON_BLUE);

    int inode_idx = find_file(name);
    if(inode_idx == -1) {
        kprint("[FAILED - not found]\n", COLOR_RED_ON_BLUE);
        return -1;
    }

    struct kfs_inode* inode = &inode_table[inode_idx];

    for(int i = 0; i < 16; i++) {
        if(inode->blocks[i] != 0) {
            free_block(inode->blocks[i]);
        }
    }

    inode->id = 0;
    inode->name[0] = '\0';
    inode->size = 0;

    kprint("[OK]\n", COLOR_GREEN_ON_BLUE);
    return 0;
}
