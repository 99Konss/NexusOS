#include "pci.h"
#include "screen.h"
#include "../lib/utils.h"
#include "keyboard.h"  // für input

static void wait_for_key(void) {
    while(!(inb(0x64) & 1));
    inb(0x60);
}

// PCI Konfiguration lesen
uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    if (slot > 31 || func > 7) return 0xFFFFFFFF;

    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDRESS, address);
    io_wait();
    return inl(PCI_CONFIG_DATA);
}

// Prüfen ob Gerät existiert
int pci_device_exists(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t vendev = pci_config_read(bus, slot, func, 0);
    return ((vendev & 0xFFFF) != 0xFFFF);
}

// Gerätetyp als String
const char* pci_get_device_type(uint8_t class_code, uint8_t subclass) {
    if (class_code == 0x01) {
        if (subclass == 0x01) return "IDE";
        if (subclass == 0x06) return "SATA/AHCI";
        if (subclass == 0x08) return "NVMe";
        return "Storage";
    }
    else if (class_code == 0x02) return "Network";
    else if (class_code == 0x03) return "Display";
    else if (class_code == 0x04) return "Multimedia";
    else if (class_code == 0x06) return "Bridge";
    else if (class_code == 0x0C) {
        if (subclass == 0x03) return "USB";
        if (subclass == 0x05) return "SMBus";
        return "Controller";
    }
    else return "Other";
}

// Farbe für Gerätetyp
uint8_t pci_get_device_color(uint8_t class_code) {
    if (class_code == 0x01) return TXT_SUCCESS;
    if (class_code == 0x02) return TXT_CYAN;
    if (class_code == 0x03) return TXT_MAGENTA;
    if (class_code == 0x04) return TXT_YELLOW;
    return TXT_INFO;
}

// Einzelnes Gerät ausgeben
void pci_print_device(uint8_t bus, uint8_t slot, uint8_t func) {
    char buf[16];

    uint32_t vendev = pci_config_read(bus, slot, func, 0);
    uint16_t vendor = vendev & 0xFFFF;
    uint16_t device = (vendev >> 16) & 0xFFFF;

    uint32_t class_reg = pci_config_read(bus, slot, func, 0x08);
    uint8_t class_code = (class_reg >> 24) & 0xFF;
    uint8_t subclass = (class_reg >> 16) & 0xFF;

    uint32_t header_reg = pci_config_read(bus, slot, func, 0x0C);
    uint8_t header_type = (header_reg >> 16) & 0xFF;
    int is_multi = (header_type & 0x80) ? 1 : 0;

    // Bus/Slot/Funktion
    kprint("  ", TXT_NORMAL);
    int_to_string(bus, buf);
    kprint(buf, TXT_GRAY);
    kprint(":", TXT_GRAY);
    int_to_string(slot, buf);
    kprint(buf, TXT_GRAY);
    if (func > 0) {
        kprint(".", TXT_GRAY);
        int_to_string(func, buf);
        kprint(buf, TXT_GRAY);
    }

    kprint("  ", TXT_NORMAL);

    // Vendor:Device
    hex_to_string(vendor, buf);
    kprint(buf, TXT_CYAN);
    kprint(":", TXT_NORMAL);
    hex_to_string(device, buf);
    kprint(buf, TXT_NORMAL);

    kprint("  [", TXT_GRAY);

    // Gerätetyp
    const char* type = pci_get_device_type(class_code, subclass);
    uint8_t color = pci_get_device_color(class_code);
    kprint(type, color);

    // AHCI Markierung
    if (class_code == 0x01 && subclass == 0x06) {
        kprint(" AHCI", COLOR_LIGHT_GREEN);
    }

    kprint("]", TXT_GRAY);

    if (is_multi) {
        kprint(" [MF]", TXT_YELLOW);
    }

    kprint("\n", TXT_NORMAL);
}

// NEU: Haupt-PCI-Scan-Funktion (für Command)
void pci_scan_and_print(void) {
    kprint("\n", TXT_NORMAL);
    kprint("╔════════════════════════════════════════════╗\n", TXT_CYAN);
    kprint("║           PCI DEVICE SCANNER              ║\n", TXT_CYAN);
    kprint("╚════════════════════════════════════════════╝\n", TXT_CYAN);

    int device_count = 0;
    int ahci_count = 0;

    // Teste PCI Verfügbarkeit
    uint32_t test_read = pci_config_read(0, 0, 0, 0);
    if ((test_read & 0xFFFF) == 0xFFFF) {
        kprint("❌ PCI nicht verfügbar!\n", TXT_ERROR);
        return;
    }

    kprint("\nBus Slot  Vendor:Device  [Type]\n", TXT_GRAY);
    kprint("────────────────────────────────────────────\n", TXT_GRAY);

    // Scanne Busse 0-2 (meistens ausreichend)
    for (uint8_t bus = 0; bus < 3; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            if (pci_device_exists(bus, slot, 0)) {
                pci_print_device(bus, slot, 0);
                device_count++;

                // Class für AHCI check
                uint32_t class_reg = pci_config_read(bus, slot, 0, 0x08);
                uint8_t class = (class_reg >> 24) & 0xFF;
                uint8_t subclass = (class_reg >> 16) & 0xFF;
                if (class == 0x01 && subclass == 0x06) ahci_count++;

                // Multifunktion?
                uint32_t header = pci_config_read(bus, slot, 0, 0x0C);
                if (header & 0x800000) {
                    for (uint8_t func = 1; func < 8; func++) {
                        if (pci_device_exists(bus, slot, func)) {
                            pci_print_device(bus, slot, func);
                            device_count++;

                            class_reg = pci_config_read(bus, slot, func, 0x08);
                            class = (class_reg >> 24) & 0xFF;
                            subclass = (class_reg >> 16) & 0xFF;
                            if (class == 0x01 && subclass == 0x06) ahci_count++;
                        }
                    }
                }
            }
        }
    }

    kprint("────────────────────────────────────────────\n", TXT_GRAY);

    char buf[16];
    int_to_string(device_count, buf);
    kprint("📊 Gefundene Geräte: ", TXT_SUCCESS);
    kprint(buf, TXT_CYAN);
    kprint("\n", TXT_NORMAL);

    if (ahci_count > 0) {
        int_to_string(ahci_count, buf);
        kprint("💾 AHCI Controller: ", TXT_SUCCESS);
        kprint(buf, TXT_CYAN);
        kprint("\n", TXT_NORMAL);
    } else {
        kprint("💾 AHCI: Keine gefunden\n", TXT_WARNING);
    }

    kprint("\nDrücke eine Taste für die Shell...", TXT_GRAY);
    wait_for_key();
}

// Alte init-Funktion (minimal)
void pci_init(void) {
    // Nur kurz prüfen ob PCI da ist, ohne Ausgabe
    uint32_t test_read = pci_config_read(0, 0, 0, 0);
    if ((test_read & 0xFFFF) == 0xFFFF) {
        kprint("PCI: Nicht verfügbar\n", TXT_ERROR);
    }
    // Keine Ausgabe - alles sauber!
}
