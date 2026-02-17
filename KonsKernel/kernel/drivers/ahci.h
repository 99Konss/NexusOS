#ifndef KERNEL_DRIVERS_AHCI_H
#define KERNEL_DRIVERS_AHCI_H

#include <stdint.h>

// AHCI PCI Class Code
#define PCI_CLASS_MASS_STORAGE 0x01
#define PCI_SUBCLASS_SATA      0x06
#define PCI_PROG_IF_AHCI       0x01

// AHCI Register Offsets
#define AHCI_CAP             0x00
#define AHCI_GHC             0x04
#define AHCI_IS              0x08
#define AHCI_PI              0x0C
#define AHCI_VERSION         0x10
#define AHCI_PORT_BASE       0x100
#define AHCI_PORT_SIZE       0x80

// Global Host Control
#define AHCI_GHC_AE          (1 << 31)  // AHCI Enable
#define AHCI_GHC_HR          (1 << 0)   // HBA Reset

// Port Command and Status
#define AHCI_PORT_CMD_ST     (1 << 0)   // Start
#define AHCI_PORT_CMD_FRE    (1 << 4)   // FIS Receive Enable
#define AHCI_PORT_CMD_FR     (1 << 14)  // FIS Receive Running
#define AHCI_PORT_CMD_CR     (1 << 15)  // Command List Running

// Port Interrupt Status
#define AHCI_PORT_IS_DHRS    (1 << 0)   // Device to Host FIS
#define AHCI_PORT_IS_PSS     (1 << 1)   // PIO Setup FIS
#define AHCI_PORT_IS_DPS     (1 << 5)   // DMA Setup FIS
#define AHCI_PORT_IS_PRCS    (1 << 22)  // PhyRdy Change

// Port Signature
#define AHCI_PORT_SIG_ATA    0x00000101  // SATA Drive
#define AHCI_PORT_SIG_ATAPI  0xEB140101  // ATAPI Drive

// Command List Entry
struct ahci_cmd_header {
    uint16_t flags;
    uint16_t prdtl;      // Physical Region Descriptor Table Length
    uint32_t prdbc;      // PRD Byte Count
    uint32_t cmd_table;  // Command Table Address
    uint32_t reserved[4];
} __attribute__((packed));

// Command Table
struct ahci_cmd_table {
    uint8_t cfis[64];    // Command FIS
    uint8_t acmd[16];    // ATAPI Command
    uint8_t reserved[48];
    struct ahci_prdt {
        uint32_t dba;    // Data Base Address
        uint32_t dbau;   // Data Base Address Upper
        uint32_t reserved;
        uint32_t dbc;    // Data Byte Count
    } prdt[1];           // Physical Region Descriptor Table
} __attribute__((packed));

// Port Register Set
struct ahci_port {
    uint32_t clb;        // Command List Base Address
    uint32_t clbu;       // Command List Base Address Upper
    uint32_t fb;         // FIS Base Address
    uint32_t fbu;        // FIS Base Address Upper
    uint32_t is;         // Interrupt Status
    uint32_t ie;         // Interrupt Enable
    uint32_t cmd;        // Command and Status
    uint32_t reserved0;
    uint32_t tfd;        // Task File Data
    uint32_t sig;        // Signature
    uint32_t ssts;       // SATA Status
    uint32_t sctl;       // SATA Control
    uint32_t serr;       // SATA Error
    uint32_t sact;       // SATA Active
    uint32_t ci;         // Command Issue
    uint32_t sntf;       // SATA Notification
    uint32_t fbs;        // FIS-based Switching
    uint32_t reserved1[11];
    uint32_t vendor[4];
} __attribute__((packed));

// HBA Memory Space
struct ahci_hba {
    uint32_t cap;        // Host Capabilities
    uint32_t ghc;        // Global Host Control
    uint32_t is;         // Interrupt Status
    uint32_t pi;         // Ports Implemented
    uint32_t vs;         // Version
    uint32_t ccc_ctl;    // Command Completion Coalescing Control
    uint32_t ccc_pts;    // Command Completion Coalescing Ports
    uint32_t em_loc;     // Enclosure Management Location
    uint32_t em_ctl;     // Enclosure Management Control
    uint32_t cap2;       // Host Capabilities Extended
    uint32_t bohc;       // BIOS/OS Handoff Control
    uint8_t  reserved[0xA0-0x2C];
    uint8_t  vendor[0x100-0xA0];
    struct ahci_port ports[32];
} __attribute__((packed));

// Funktionen
void ahci_init(void);
void ahci_scan_pci(void);
int ahci_read_sectors(int port, uint64_t lba, uint32_t sector_count, void* buffer);
int ahci_write_sectors(int port, uint64_t lba, uint32_t sector_count, void* buffer);

#endif
