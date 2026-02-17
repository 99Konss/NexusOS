#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

// Maus Zustand
typedef struct {
    int x;
    int y;
    uint8_t buttons;  // Bit 0: Links, Bit 1: Rechts, Bit 2: Mitte
    int dx;           // Bewegung seit letztem Poll
    int dy;
} mouse_t;

extern mouse_t g_mouse;

// Funktionen
void mouse_init(void);
void mouse_handler(void);  // Wird vom Interrupt aufgerufen
void mouse_get_state(mouse_t* state);

void mouse_set_position(int x, int y);


#endif
