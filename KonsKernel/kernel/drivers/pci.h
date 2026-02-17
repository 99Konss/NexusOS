#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// Funktionen
void pci_init(void);  // bleibt für init
void pci_scan_and_print(void);  // NEU: nur scannen und ausgeben
uint32_t pci_config_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
int pci_device_exists(uint8_t bus, uint8_t slot, uint8_t func);

#endif
