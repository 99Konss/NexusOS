// kernel/drivers/acpi.c
#include "acpi.h"
#include "../lib/utils.h"
#include "screen.h"
#include <stddef.h>

static struct acpi_rsdp* rsdp = NULL;
static struct acpi_fadt* fadt = NULL;

int acpi_is_available(void) {
    return rsdp != NULL;
}

void acpi_init(void) {
    kprint("Initializing ACPI... ", TXT_INFO);

    // RSDP im BIOS-Bereich suchen (0xE0000 - 0xFFFFF)
    for (uint32_t addr = 0xE0000; addr < 0x100000; addr += 16) {
        char* ptr = (char*)addr;

        if (ptr[0] == 'R' && ptr[1] == 'S' && ptr[2] == 'D' && ptr[3] == ' ') {
            rsdp = (struct acpi_rsdp*)addr;

            // Checksum prüfen
            uint8_t sum = 0;
            for (int i = 0; i < 20; i++) {
                sum += ((uint8_t*)rsdp)[i];
            }

            if ((sum & 0xFF) == 0) {
                kprint("[OK]\n", TXT_SUCCESS);

                // RSDT durchsuchen nach FADT
                struct acpi_rsdt* rsdt = (struct acpi_rsdt*)(uint32_t)rsdp->rsdt_address;
                int entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

                for (int i = 0; i < entries; i++) {
                    struct acpi_sdt_header* header = (struct acpi_sdt_header*)(uint32_t)rsdt->entries[i];

                    if (header->signature[0] == 'F' && header->signature[1] == 'A' &&
                        header->signature[2] == 'C' && header->signature[3] == 'P') {
                        fadt = (struct acpi_fadt*)header;
                        kprint("FADT found\n", TXT_SUCCESS);
                        return;
                    }
                }
            }
        }
    }

    kprint("[FAILED]\n", TXT_ERROR);
    rsdp = NULL;
}

void acpi_reboot(void) {
    kprint("Rebooting...\n", TXT_WARNING);

    // Methode 1: 8042 Tastatur-Controller
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);

    // Methode 2: ACPI Reset (falls verfügbar)
    if (fadt && fadt->reset_reg[8]) {  // reset_reg.address
        uint8_t reset_value = fadt->reset_value;
        uint8_t address_space = fadt->reset_reg[0];

        if (address_space == 1) {  // System I/O
            outb(*(uint16_t*)&fadt->reset_reg[4], reset_value);
        }
    }

    // Methode 3: Triple Fault (Notfall)
    asm volatile("int3");
}

void acpi_shutdown(void) {
    kprint("Shutting down...\n", TXT_WARNING);

    // QEMU
    outw(0xB004, 0x2000);
    outw(0x604, 0x2000);

    // VirtualBox
    outw(0x4004, 0x3400);

    // Bochs
    outw(0x8900, 0x0);
    outw(0x8900, 0x1);

    // Falls nichts geht - HALT
    asm volatile("cli; hlt");
}
