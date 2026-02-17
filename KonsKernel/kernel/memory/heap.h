#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

// === Heap Konstanten ===
#define HEAP_START      0x01000000  // 16 MB
#define HEAP_SIZE       0x0F000000  // 240 MB
#define HEAP_END        (HEAP_START + HEAP_SIZE)  // 256 MB total

#define BLOCK_SIZE      16
#define BLOCK_ALIGN     8
#define MAX_ALLOCS      8192
#define HEAP_MAGIC      0xDEADBEEF

// === Memory Management ===
#define PAGE_SIZE       4096
#define BITMAP_SIZE     8192       // Für 32MB Bitmap (8192*8*4096 = 256GB)

// === Volle alloc_info Struktur (mit ALLEN Feldern) ===
typedef struct alloc_info {
    void* ptr;              // Zeiger auf allokierten Speicher
    uint32_t size;          // Größe in Bytes
    uint32_t pages;         // Anzahl belegter Seiten
    uint8_t used;           // 1 = belegt, 0 = frei
    uint8_t aligned;        // 1 = alignierte Allokation
    const char* file;       // Dateiname für Debug
    int line;               // Zeilennummer für Debug
    uint32_t magic;         // Magic Number für Corruption-Check
} alloc_info_t;

// === Externe Variablen ===
extern alloc_info_t allocations[MAX_ALLOCS];
extern int alloc_count;
extern uint32_t heap_pointer;
extern uint32_t heap_end;

extern uint32_t heap_pointer;  // bleibt

// === Memory Variablen ===
extern uint32_t total_memory;
extern uint32_t free_memory;
extern uint32_t used_memory;
extern uint32_t kernel_memory;
extern uint8_t page_bitmap[BITMAP_SIZE];
extern uint32_t total_pages;
extern uint32_t free_pages;
extern uint32_t used_pages;

// === Basis-Funktionen ===
void init_heap(void);
void* kmalloc_safe(uint32_t size);
void kfree_safe(void* ptr);
void print_memory_info(void);
void debug_memory(void);
void read_multiboot_info(uint32_t addr);

// === Erweiterte Funktionen ===
void* malloc_aligned(uint32_t size, uint32_t alignment);
void* calloc(uint32_t num, uint32_t size);
void* realloc(void* ptr, uint32_t new_size);
void check_heap_corruption(void);
void memory_leak_check(void);

// === Alias für Kompatibilität ===
#define malloc(size) kmalloc_safe(size)
#define free(ptr) kfree_safe(ptr)

#endif
