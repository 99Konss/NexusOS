#ifndef GUI_CORE_H
#define GUI_CORE_H

#include <stdint.h>
#include "framebuffer.h"

// === Grundlegende Typen (bleiben für immer!) ===
typedef struct {
    int x, y;
} point_t;

typedef struct {
    int width, height;
} size_t;

typedef struct {
    int x, y, width, height;
} rect_t;

typedef struct {
    uint8_t r, g, b, a;
} color_t;

// Vordefinierte Farben
#define COLOR_BLACK   (color_t){0, 0, 0, 255}
#define COLOR_WHITE   (color_t){255, 255, 255, 255}
#define COLOR_RED     (color_t){255, 0, 0, 255}
#define COLOR_GREEN   (color_t){0, 255, 0, 255}
#define COLOR_BLUE    (color_t){0, 0, 255, 255}
#define COLOR_CYAN    (color_t){0, 255, 255, 255}
#define COLOR_MAGENTA (color_t){255, 0, 255, 255}
#define COLOR_YELLOW  (color_t){255, 255, 0, 255}

// === Events (bleiben stabil) ===
typedef enum {
    EVENT_NONE,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_DOWN,
    EVENT_MOUSE_UP,
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_WINDOW_CLOSE,
    EVENT_WINDOW_RESIZE,
    EVENT_WINDOW_FOCUS,
    EVENT_TIMER,
    EVENT_PAINT,        // Zum Neuzeichnen
    EVENT_USER = 1000   // Für eigene Events
} event_type_t;

typedef struct {
    event_type_t type;
    union {
        struct { int x, y; } mouse;
        struct { uint8_t keycode; } key;
        struct { int width, height; } resize;
    } data;
    uint32_t timestamp;
} event_t;

// === GUI-Objekt (Basis für ALLES) ===
typedef struct gui_object gui_object_t;

struct gui_object {
    // Metadaten
    char name[32];
    uint32_t id;

    // Position und Größe
    rect_t bounds;
    int visible;
    int enabled;

    // Eltern-Kind Beziehung
    gui_object_t* parent;
    gui_object_t** children;
    int child_count;

    // Stil
    color_t bg_color;
    color_t fg_color;
    color_t border_color;
    int border_width;

    // Virtuelle Funktionen (Polymorphie!)
    void (*draw)(gui_object_t* self);
    void (*handle_event)(gui_object_t* self, event_t* event);
    void (*destroy)(gui_object_t* self);
    void* (*get_user_data)(gui_object_t* self);

    // User Data (für Erweiterungen)
    void* user_data;
};

// === Core-Funktionen (dein stabiles Fundament) ===
void gui_init(void);
void gui_run(void);  // Hauptloop
void gui_shutdown(void);

// Objekt-Management
gui_object_t* gui_create_object(rect_t bounds, const char* name);
void gui_destroy_object(gui_object_t* obj);
void gui_add_child(gui_object_t* parent, gui_object_t* child);
void gui_remove_child(gui_object_t* parent, gui_object_t* child);

// Zeichnen
void gui_invalidate_rect(rect_t rect);  // Bereich zum Neuzeichnen markieren
void gui_render_all(void);               // Alles zeichnen

// Events
void gui_post_event(event_t* event);
void gui_dispatch_events(void);

// Hilfsfunktionen
int gui_point_in_rect(point_t p, rect_t r);
rect_t gui_rect_intersect(rect_t a, rect_t b);

#endif
