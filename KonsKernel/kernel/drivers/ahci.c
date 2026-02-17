#include "ahci.h"
#include "pci.h"
#include "screen.h"
#include "../lib/utils.h"

void ahci_init(void) {
    kprint("\n=== AHCI Check ===\n", TXT_CYAN);
    for (uint32_t slot = 0; slot < 32; slot++) {
        uint32_t vendev = pci_config_read(0, slot, 0, 0);
        if ((vendev & 0xFFFF) == 0xFFFF) continue;

        uint32_t class = pci_config_read(0, slot, 0, 0x08);
        uint8_t class_code = (class >> 24) & 0xFF;
        uint8_t subclass  = (class >> 16) & 0xFF;
        uint8_t prog_if    = (class >> 8) & 0xFF;

        if (class_code == 0x01 && subclass == 0x06 && prog_if == 0x01) {
            kprint("AHCI Controller found at slot ", TXT_SUCCESS);
            char c = '0' + slot;
            kprint(&c, TXT_YELLOW);
            kprint("\n", TXT_NORMAL);

            uint32_t bar5 = pci_config_read(0, slot, 0, 0x24);
            kprint("ABAR: ", TXT_INFO);
            char hex[16];
            hex_to_string(bar5 & ~0xF, hex);
            kprint(hex, TXT_INFO);
            kprint("\n", TXT_NORMAL);
            return;
        }
    }
    kprint("No AHCI controller found.\n", TXT_ERROR);
}
