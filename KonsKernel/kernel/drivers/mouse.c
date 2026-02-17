#include "mouse.h"
#include "../lib/utils.h"
#include "screen.h"

// PS/2 Maus Ports
#define PS2_DATA_PORT       0x60
#define PS2_STATUS_PORT     0x64
#define PS2_COMMAND_PORT    0x64

// Maus Commands
#define MOUSE_ENABLE        0xF4
#define MOUSE_DISABLE       0xF5
#define MOUSE_SET_DEFAULTS  0xF6
#define MOUSE_RESET         0xFF

// Maus Zustand
mouse_t g_mouse;
static int mouse_cycle = 0;
static uint8_t mouse_packet[3];
static int mouse_ready = 0;

// Auf Tastatur-Controller warten
static void mouse_wait(uint8_t type) {
    if (type == 0) { // Warten bis Daten bereit
        while (!(inb(PS2_STATUS_PORT) & 1));
    } else { // Warten bis schreiben möglich
        while (inb(PS2_STATUS_PORT) & 2);
    }
}

// Befehl an Maus senden
static void mouse_send_command(uint8_t cmd) {
    // Maus aufwecken
    mouse_wait(1);
    outb(PS2_COMMAND_PORT, 0xD4);

    mouse_wait(1);
    outb(PS2_DATA_PORT, cmd);

    // Auf Antwort warten
    mouse_wait(0);
    uint8_t ack = inb(PS2_DATA_PORT);

    if (ack != 0xFA) {
        kprint("Mouse: Command failed\n", COLOR_RED);
    }
}

// Maus initialisieren
void mouse_init(void) {
    kprint("Initializing PS/2 mouse... ", COLOR_WHITE_ON_BLUE);

    // Maus-Paket-Variablen zurücksetzen
    mouse_cycle = 0;
    g_mouse.x = 512;  // Start in der Mitte
    g_mouse.y = 384;
    g_mouse.buttons = 0;

    // Maus aktivieren
    mouse_wait(1);
    outb(PS2_COMMAND_PORT, 0xA8);  // Maus Interface aktivieren

    // Standardwerte setzen
    mouse_send_command(MOUSE_SET_DEFAULTS);

    // Maus aktivieren
    mouse_send_command(MOUSE_ENABLE);

    kprint("OK\n", COLOR_GREEN_ON_BLUE);
}

// Maus Interrupt Handler (wird von IRQ12 aufgerufen!)
// In mouse.c - die bestehende mouse_handler Funktion
void mouse_handler(void) {
    uint8_t status = inb(PS2_STATUS_PORT);

    // Prüfen ob Daten von Maus
    if (!(status & 0x20)) return;

    // Daten lesen
    uint8_t data = inb(PS2_DATA_PORT);

    // Maus-Paket zusammensetzen (3 Bytes)
    switch(mouse_cycle) {
        case 0: // Byte 0: Buttons + Vorzeichen
            if (!(data & 0x08)) { // Prüfen ob gültiges Paket
                mouse_packet[0] = data;
                mouse_cycle = 1;
            }
            break;

        case 1: // Byte 1: X-Bewegung
            mouse_packet[1] = data;
            mouse_cycle = 2;
            break;

        case 2: // Byte 2: Y-Bewegung
            mouse_packet[2] = data;
            mouse_cycle = 0;

            // Buttons extrahieren
            g_mouse.buttons = mouse_packet[0] & 0x07;

            // X-Bewegung (mit Vorzeichen)
            g_mouse.dx = mouse_packet[1];
            if (mouse_packet[0] & 0x10) { // X negative?
                g_mouse.dx -= 256;
            }

            // Y-Bewegung (mit Vorzeichen - umgekehrt!)
            g_mouse.dy = mouse_packet[2];
            if (mouse_packet[0] & 0x20) { // Y negative?
                g_mouse.dy -= 256;
            }
            g_mouse.dy = -g_mouse.dy; // Y-Achse umkehren

            // Position aktualisieren
            g_mouse.x += g_mouse.dx;
            g_mouse.y += g_mouse.dy;

            // Im Bildschirm halten
            if (g_mouse.x < 0) g_mouse.x = 0;
            if (g_mouse.x >= 1024) g_mouse.x = 1023;
            if (g_mouse.y < 0) g_mouse.y = 0;
            if (g_mouse.y >= 768) g_mouse.y = 767;
            break;
    }
}

void mouse_get_state(mouse_t* state) {
    *state = g_mouse;
}

void mouse_set_position(int x, int y) {
    g_mouse.x = x;
    g_mouse.y = y;
    g_mouse.buttons = 0;
    g_mouse.dx = 0;
    g_mouse.dy = 0;
}
