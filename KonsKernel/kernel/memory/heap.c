#include <stddef.h>
#include <stdint.h>
#include "heap.h"
#include "../drivers/screen.h"
#include "../lib/utils.h"

// === Heap Variablen ===
uint32_t heap_pointer = HEAP_START;
uint32_t heap_end = HEAP_END;  // NICHT static! (weil extern in .h)
alloc_info_t allocations[MAX_ALLOCS];
int alloc_count = 0;

// === Memory Variablen ===
uint32_t total_memory = 0;
uint32_t free_memory = 0;
uint32_t used_memory = 0;
uint32_t kernel_memory = 0;
uint8_t page_bitmap[BITMAP_SIZE];
uint32_t total_pages = 0;
uint32_t free_pages = 0;
uint32_t used_pages = 0;

// === Multiboot Struktur (hier definiert, da nicht in eigenem Header) ===
struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    // ... weitere Felder (nur die brauchen wir)
};

// === Hilfsfunktionen (static) ===
static void hex_to_str(uint32_t num, char* str) {
    const char* digits = "0123456789ABCDEF";
    for(int i = 0; i < 8; i++) {
        str[i] = digits[(num >> (28 - i*4)) & 0xF];
    }
    str[8] = '\0';
}

static void int_to_str(int num, char* str) {
    if(num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    char temp[16];
    int i = 0;
    int is_negative = 0;

    if(num < 0) {
        is_negative = 1;
        num = -num;
    }

    while(num > 0) {
        temp[i++] = '0' + (num % 10);
        num /= 10;
    }

    int j = 0;
    if(is_negative) {
        str[j++] = '-';
    }

    for(i = i - 1; i >= 0; i--) {
        str[j++] = temp[i];
    }
    str[j] = '\0';
}

// === Freien Slot finden ===
static int find_free_slot(void) {
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(!allocations[i].used) {
            return i;
        }
    }
    return -1;
}

// === Multiboot Memory Info ===
void read_multiboot_info(uint32_t addr) {
    struct multiboot_info* mbi = (struct multiboot_info*)addr;

    if (mbi->flags & 1) {  // Bit 0 gesetzt = mem_* gültig
        total_memory = (mbi->mem_lower + mbi->mem_upper) * 1024;
    } else {
        total_memory = 16 * 1024 * 1024; // Fallback: 16 MB
    }

    // Kernel nimmt ersten 2 MB an (1MB - 3MB)
    kernel_memory = 0x200000; // 2 MB
    used_memory = kernel_memory;
    free_memory = total_memory - used_memory;

    total_pages = total_memory / PAGE_SIZE;
    used_pages = used_memory / PAGE_SIZE;
    free_pages = total_pages - used_pages;
}

// === Page-Bitmap initialisieren ===
static void init_page_bitmap(void) {
    // Alle Seiten als frei markieren (1 = frei)
    for(int i = 0; i < BITMAP_SIZE; i++) {
        page_bitmap[i] = 0xFF;
    }

    // Kernel-Bereich (1MB - 3MB) als belegt markieren
    uint32_t kernel_start = 0x100000;  // 1 MB
    uint32_t kernel_end = 0x300000;    // 3 MB

    for(uint32_t addr = kernel_start; addr < kernel_end; addr += PAGE_SIZE) {
        int page_idx = addr / PAGE_SIZE;
        int byte_idx = page_idx / 8;
        int bit_idx = page_idx % 8;

        if(byte_idx < BITMAP_SIZE) {
            page_bitmap[byte_idx] &= ~(1 << bit_idx);  // Bit löschen = belegt
        }
    }

    // VGA-Bereich (0xB8000 - 0xC0000) als belegt markieren
    uint32_t vga_start = 0xB8000;
    uint32_t vga_end = 0xC0000;

    for(uint32_t addr = vga_start; addr < vga_end; addr += PAGE_SIZE) {
        int page_idx = addr / PAGE_SIZE;
        int byte_idx = page_idx / 8;
        int bit_idx = page_idx % 8;

        if(byte_idx < BITMAP_SIZE) {
            page_bitmap[byte_idx] &= ~(1 << bit_idx);
        }
    }

    // Statistiken aktualisieren
    used_pages = 0;
    for(int i = 0; i < BITMAP_SIZE; i++) {
        for(int bit = 0; bit < 8; bit++) {
            if(!(page_bitmap[i] & (1 << bit))) {
                used_pages++;
            }
        }
    }
    free_pages = total_pages - used_pages;
    used_memory = used_pages * PAGE_SIZE;
    free_memory = free_pages * PAGE_SIZE;
}

// === Heap-Initialisierung ===
void init_heap(void) {
    heap_pointer = HEAP_START;
    heap_end = HEAP_END;

    // Allocation-Array initialisieren
    for(int i = 0; i < MAX_ALLOCS; i++) {
        allocations[i].ptr = NULL;
        allocations[i].size = 0;
        allocations[i].pages = 0;
        allocations[i].used = 0;
        allocations[i].aligned = 0;
        allocations[i].file = NULL;
        allocations[i].line = 0;
        allocations[i].magic = 0;
    }
    alloc_count = 0;

    // Page-Bitmap initialisieren
    init_page_bitmap();

    // Erfolgsmeldung
    kprint("Heap: ", 0x1F);  // White on blue
    char buf[16];
    hex_to_str(HEAP_START, buf);
    kprint(buf, 0x2F);  // Green on blue
    kprint(" - ", 0x1F);
    hex_to_str(HEAP_END, buf);
    kprint(buf, 0x2F);
    kprint(" (", 0x1F);
    int_to_str(HEAP_SIZE / (1024 * 1024), buf);
    kprint(buf, 0x3F);  // Cyan on blue
    kprint(" MB)\n", 0x1F);
}

// === malloc_debug (interne Funktion) ===
static void* malloc_debug(uint32_t size, const char* file, int line) {
    if(size == 0) return NULL;

    // Auf Block-Größe ausrichten
    uint32_t original_size = size;
    size = (size + BLOCK_SIZE - 1) & ~(BLOCK_SIZE - 1);

    // Prüfen ob genug Platz im Heap
    if(heap_pointer + size > heap_end) {
        kprint("HEAP OVERFLOW! ", 0x4F);  // Red on white
        char buf[16];
        int_to_str(size, buf);
        kprint(buf, 0x4F);
        kprint(" bytes\n", 0x4F);
        return NULL;
    }

    // Freien Slot finden
    int slot = find_free_slot();
    if(slot == -1) {
        kprint("HEAP: No free allocation slots!\n", 0x4F);
        return NULL;
    }

    // Speicher allozieren
    void* ptr = (void*)heap_pointer;
    allocations[slot].ptr = ptr;
    allocations[slot].size = size;
    allocations[slot].pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    allocations[slot].used = 1;
    allocations[slot].aligned = 0;
    allocations[slot].file = file;
    allocations[slot].line = line;
    allocations[slot].magic = HEAP_MAGIC;

    heap_pointer += size;
    alloc_count++;
    used_memory += size;
    free_memory -= size;

    // Speicher mit 0xAA markieren (für Debug)
    char* cptr = (char*)ptr;
    for(uint32_t i = 0; i < size; i++) {
        cptr[i] = 0xAA;
    }

    return ptr;
}

// === kmalloc_safe (öffentliche Funktion) ===
void* kmalloc_safe(uint32_t size) {
    return malloc_debug(size, NULL, 0);
}

// === Alignierte Allokation ===
void* malloc_aligned(uint32_t size, uint32_t alignment) {
    if(alignment < BLOCK_ALIGN) alignment = BLOCK_ALIGN;

    // Extra Platz für Alignment + Header
    uint32_t header_size = alignment + sizeof(void*);
    uint8_t* raw = (uint8_t*)kmalloc_safe(size + header_size);
    if(!raw) return NULL;

    // Alignierten Zeiger berechnen
    uintptr_t raw_addr = (uintptr_t)raw;
    uintptr_t aligned = (raw_addr + alignment - 1) & ~(alignment - 1);

    // Header vor dem aligned pointer speichern
    ((uintptr_t*)aligned)[-1] = raw_addr;

    // In allocations aligned flag setzen
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(allocations[i].ptr == raw) {
            allocations[i].aligned = 1;
            break;
        }
    }

    return (void*)aligned;
}

// === calloc ===
void* calloc(uint32_t num, uint32_t size) {
    uint32_t total = num * size;
    void* ptr = kmalloc_safe(total);
    if(ptr) {
        char* cptr = (char*)ptr;
        for(uint32_t i = 0; i < total; i++) {
            cptr[i] = 0;
        }
    }
    return ptr;
}

// === realloc ===
void* realloc(void* ptr, uint32_t new_size) {
    if(!ptr) return kmalloc_safe(new_size);
    if(new_size == 0) {
        kfree_safe(ptr);
        return NULL;
    }

    // Alte Größe finden
    uint32_t old_size = 0;
    int old_slot = -1;
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(allocations[i].used && allocations[i].ptr == ptr) {
            old_size = allocations[i].size;
            old_slot = i;
            break;
        }
    }

    if(old_size == 0) return NULL; // Ungültiger Pointer

    // Neuen Speicher allozieren
    void* new_ptr = kmalloc_safe(new_size);
    if(!new_ptr) return NULL;

    // Daten kopieren
    uint32_t copy_size = (old_size < new_size) ? old_size : new_size;
    char* src = (char*)ptr;
    char* dst = (char*)new_ptr;
    for(uint32_t i = 0; i < copy_size; i++) {
        dst[i] = src[i];
    }

    // Alten Speicher freigeben
    kfree_safe(ptr);

    return new_ptr;
}

// === kfree_safe ===
void kfree_safe(void* ptr) {
    if(!ptr) return;

    // Prüfen ob es ein alignierter Pointer ist
    uintptr_t aligned_ptr = (uintptr_t)ptr;
    uintptr_t raw_ptr = aligned_ptr;

    // Gucken ob der Pointer von malloc_aligned kommt
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(allocations[i].used) {
            if(allocations[i].ptr == ptr) {
                raw_ptr = (uintptr_t)ptr;
                break;
            }
            // Prüfen ob ptr im aligned Bereich liegt
            if(allocations[i].aligned) {
                uintptr_t start = (uintptr_t)allocations[i].ptr;
                uintptr_t end = start + allocations[i].size;
                if(aligned_ptr >= start && aligned_ptr < end) {
                    raw_ptr = start;
                    break;
                }
            }
        }
    }

    // Slot finden und freigeben
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(allocations[i].used && (uintptr_t)allocations[i].ptr == raw_ptr) {
            // Magic Number prüfen
            if(allocations[i].magic != HEAP_MAGIC) {
                kprint("HEAP CORRUPTION DETECTED!\n", 0x4F);
                return;
            }

            allocations[i].used = 0;
            alloc_count--;
            used_memory -= allocations[i].size;
            free_memory += allocations[i].size;
            return;
        }
    }

    kprint("WARNING: Free on unknown pointer ", 0x1E);  // Yellow on blue
    char buf[16];
    hex_to_str((uint32_t)ptr, buf);
    kprint(buf, 0x1E);
    kprint("\n", 0x1E);
}

// === Memory Info ausgeben ===
void print_memory_info(void) {
    kprint("\n", 0x07);
    kprint("╔════════════════════════════════════╗\n", 0x03);
    kprint("║       MEMORY INFORMATION           ║\n", 0x03);
    kprint("╚════════════════════════════════════╝\n", 0x03);

    char buf[32];

    uint32_t total_mb = total_memory / (1024 * 1024);
    uint32_t free_mb = free_memory / (1024 * 1024);
    uint32_t used_mb = used_memory / (1024 * 1024);
    uint32_t kernel_mb = kernel_memory / (1024 * 1024);
    uint32_t heap_mb = (heap_pointer - HEAP_START) / (1024 * 1024);

    kprint(" Total RAM:  ", 0x07);
    int_to_str(total_mb, buf);
    kprint(buf, 0x03);
    kprint(" MB\n", 0x07);

    kprint(" Used:       ", 0x07);
    int_to_str(used_mb, buf);
    kprint(buf, 0x0E);
    kprint(" MB\n", 0x07);

    kprint(" Free:       ", 0x07);
    int_to_str(free_mb, buf);
    kprint(buf, 0x0A);
    kprint(" MB\n", 0x07);

    kprint(" Kernel:     ", 0x07);
    int_to_str(kernel_mb, buf);
    kprint(buf, 0x0D);
    kprint(" MB\n", 0x07);

    kprint(" Heap used:  ", 0x07);
    int_to_str(heap_mb, buf);
    kprint(buf, 0x03);
    kprint(" MB\n", 0x07);

    kprint(" Allocs:     ", 0x07);
    int_to_str(alloc_count, buf);
    kprint(buf, 0x03);
    kprint("/", 0x07);
    int_to_str(MAX_ALLOCS, buf);
    kprint(buf, 0x08);
    kprint("\n\n", 0x07);
}

// === Heap Corruption Check ===
void check_heap_corruption(void) {
    int corrupted = 0;
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(allocations[i].used && allocations[i].magic != HEAP_MAGIC) {
            corrupted++;
            kprint("Corrupted block at ", 0x04);
            char buf[16];
            hex_to_str((uint32_t)allocations[i].ptr, buf);
            kprint(buf, 0x04);
            kprint("\n", 0x04);
        }
    }

    if(corrupted == 0) {
        kprint("Heap corruption check: OK\n", 0x0A);
    } else {
        kprint("Heap corruption detected!\n", 0x04);
    }
}

// === Memory Leak Check ===
void memory_leak_check(void) {
    if(alloc_count == 0) {
        kprint("No memory leaks detected.\n", 0x0A);
        return;
    }

    kprint("Potential memory leaks:\n", 0x0E);
    for(int i = 0; i < MAX_ALLOCS; i++) {
        if(allocations[i].used) {
            char buf[16];
            kprint("  - ", 0x0E);
            hex_to_str((uint32_t)allocations[i].ptr, buf);
            kprint(buf, 0x03);
            kprint(" (", 0x07);
            int_to_str(allocations[i].size, buf);
            kprint(buf, 0x07);
            kprint(" bytes)", 0x07);

            if(allocations[i].file) {
                kprint(" at ", 0x07);
                kprint((char*)allocations[i].file, 0x08);
                kprint(":", 0x07);
                int_to_str(allocations[i].line, buf);
                kprint(buf, 0x08);
            }
            kprint("\n", 0x07);
        }
    }
}

// === Debug Memory (früher debug_heap) ===
void debug_memory(void) {
    kprint("\n", 0x07);
    kprint("╔════════════════════════════════════╗\n", 0x0D);
    kprint("║          HEAP DEBUG                ║\n", 0x0D);
    kprint("╚════════════════════════════════════╝\n", 0x0D);

    char buf[32];

    kprint(" Heap start: 0x", 0x07);
    hex_to_str(HEAP_START, buf);
    kprint(buf, 0x03);
    kprint("\n", 0x07);

    kprint(" Heap ptr:   0x", 0x07);
    hex_to_str(heap_pointer, buf);
    kprint(buf, 0x0E);
    kprint("\n", 0x07);

    kprint(" Heap end:   0x", 0x07);
    hex_to_str(heap_end, buf);
    kprint(buf, 0x03);
    kprint("\n", 0x07);

    kprint(" Used:       ", 0x07);
    int_to_str(used_memory / 1024, buf);
    kprint(buf, 0x0E);
    kprint(" KB\n", 0x07);

    kprint(" Free:       ", 0x07);
    int_to_str(free_memory / 1024, buf);
    kprint(buf, 0x0A);
    kprint(" KB\n", 0x07);

    kprint(" Allocations: ", 0x07);
    int_to_str(alloc_count, buf);
    kprint(buf, 0x03);
    kprint("\n\n", 0x07);

    // Erste 10 Allokationen anzeigen
    if(alloc_count > 0) {
        kprint("Active allocations:\n", 0x08);
        for(int i = 0; i < MAX_ALLOCS && i < 10; i++) {
            if(allocations[i].used) {
                kprint("  [", 0x08);
                int_to_str(i, buf);
                kprint(buf, 0x08);
                kprint("] 0x", 0x08);
                hex_to_str((uint32_t)allocations[i].ptr, buf);
                kprint(buf, 0x03);
                kprint(" (", 0x08);
                int_to_str(allocations[i].size, buf);
                kprint(buf, 0x07);
                kprint(" bytes)", 0x08);
                if(allocations[i].aligned) {
                    kprint(" [aligned]", 0x0A);
                }
                kprint("\n", 0x08);
            }
        }
        kprint("\n", 0x07);
    }
}
