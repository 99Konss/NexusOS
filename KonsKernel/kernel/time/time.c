#include "time.h"
#include "../lib/utils.h"

// dein Offset (wird vom Command geändert)
int timezone_offset = 0;

// CMOS helpers
uint8_t read_cmos(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

// Hauptfunktion
void get_time(int *hour, int *min, int *sec) {

    *sec  = bcd_to_bin(read_cmos(0x00));
    *min  = bcd_to_bin(read_cmos(0x02));
    *hour = bcd_to_bin(read_cmos(0x04));

    // timezone anwenden
    *hour = (*hour + timezone_offset + 24) % 24;
}
