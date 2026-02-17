#include "gui_core.h"
#include "../../memory/heap.h"
#include "../../drivers/screen.h"
#include "../../lib/utils.h"

// Globale GUI-Zustände
static struct {
    gui_object_t* root;           // Root-Objekt (Desktop)
    gui_object_t* focused;         // Fokussiertes Objekt
    gui_object_t* hover;           // Überfahrenes Objekt
    point_t mouse_pos;
    int running;
    uint32_t next_id;
    event_t event_queue[256];
    int event_head, event_tail;
    rect_t dirty_rects[64];        // Zu aktualisierende Bereiche
    int dirty_count;
} gui;

// === Hilfsfunktionen ===
int gui_point_in_rect(point_t p, rect_t r) {
    return (p.x >= r.x && p.x < r.x + r.width &&
            p.y >= r.y && p.y < r.y + r.height);
}

rect_t gui_rect_intersect(rect_t a, rect_t b) {
    rect_t result;
    result.x = (a.x > b.x) ? a.x : b.x;
    result.y = (a.y > b.y) ? a.y : b.y;
    result.width = ((a.x + a.width < b.x + b.width) ?
                    a.x + a.width : b.x + b.width) - result.x;
    result.height = ((a.y + a.height < b.y + b.height) ?
                     a.y + a.height : b.y + b.height) - result.y;

    if (result.width < 0) result.width = 0;
    if (result.height < 0) result.height = 0;

    return result;
}

// === Objekt-Erstellung ===
gui_object_t* gui_create_object(rect_t bounds, const char* name) {
    gui_object_t* obj = (gui_object_t*)malloc(sizeof(gui_object_t));
    if (!obj) return NULL;

    // Basis-Initialisierung
    obj->id = gui.next_id++;
    obj->bounds = bounds;
    obj->visible = 1;
    obj->enabled = 1;
    obj->parent = NULL;
    obj->children = NULL;
    obj->child_count = 0;

    // Default-Farben
    obj->bg_color = COLOR_WHITE;
    obj->fg_color = COLOR_BLACK;
    obj->border_color = COLOR_BLACK;
    obj->border_width = 1;

    // Virtuelle Funktionen (können überschrieben werden)
    obj->draw = NULL;  // Kindklassen setzen ihre eigenen
    obj->handle_event = NULL;
    obj->destroy = NULL;
    obj->get_user_data = NULL;

    obj->user_data = NULL;

    if (name) {
        int i;
        for (i = 0; name[i] && i < 31; i++) {
            obj->name[i] = name[i];
        }
        obj->name[i] = '\0';
    } else {
        obj->name[0] = '\0';
    }

    return obj;
}

// === Objekt-Zerstörung ===
void gui_destroy_object(gui_object_t* obj) {
    if (!obj) return;

    // Erst alle Kinder zerstören
    for (int i = 0; i < obj->child_count; i++) {
        if (obj->children[i]) {
            gui_destroy_object(obj->children[i]);
        }
    }

    // Eigene Destroy-Funktion aufrufen
    if (obj->destroy) {
        obj->destroy(obj);
    }

    // Children-Array freigeben
    if (obj->children) {
        free(obj->children);
    }

    // Referenzen entfernen
    if (gui.focused == obj) gui.focused = NULL;
    if (gui.hover == obj) gui.hover = NULL;

    // Objekt freigeben
    free(obj);
}

// === Kinder verwalten ===
void gui_add_child(gui_object_t* parent, gui_object_t* child) {
    if (!parent || !child) return;

    // Children-Array vergrößern
    gui_object_t** new_children = (gui_object_t**)realloc(
        parent->children,
        (parent->child_count + 1) * sizeof(gui_object_t*)
    );

    if (!new_children) return;

    parent->children = new_children;
    parent->children[parent->child_count] = child;
    parent->child_count++;

    child->parent = parent;
}

// === Bereich zum Neuzeichnen markieren ===
void gui_invalidate_rect(rect_t rect) {
    if (gui.dirty_count >= 64) return;

    // Mit vorhandenen dirty rects vereinigen (optimieren)
    for (int i = 0; i < gui.dirty_count; i++) {
        // Wenn das neue Rect das alte komplett einschließt, ersetzen
        if (rect.x <= gui.dirty_rects[i].x &&
            rect.y <= gui.dirty_rects[i].y &&
            rect.x + rect.width >= gui.dirty_rects[i].x + gui.dirty_rects[i].width &&
            rect.y + rect.height >= gui.dirty_rects[i].y + gui.dirty_rects[i].height) {
            gui.dirty_rects[i] = rect;
            return;
        }
    }

    gui.dirty_rects[gui.dirty_count++] = rect;
}

// === Zeichen-Hierarchie ===
static void render_object(gui_object_t* obj, rect_t clip) {
    if (!obj || !obj->visible) return;

    // Prüfen ob Objekt im Clip-Bereich liegt
    rect_t obj_rect = obj->bounds;
    rect_t intersection = gui_rect_intersect(obj_rect, clip);
    if (intersection.width <= 0 || intersection.height <= 0) return;

    // Eigenes Zeichnen
    if (obj->draw) {
        obj->draw(obj);
    } else {
        // Default: Gefülltes Rechteck
        fb_fill_rect(obj->bounds.x, obj->bounds.y,
                     obj->bounds.width, obj->bounds.height,
                     (obj->bg_color.r << 16) | (obj->bg_color.g << 8) | obj->bg_color.b);

        // Border
        if (obj->border_width > 0) {
            uint32_t border = (obj->border_color.r << 16) |
                             (obj->border_color.g << 8) | obj->border_color.b;
            for (int i = 0; i < obj->border_width; i++) {
                fb_fill_rect(obj->bounds.x + i, obj->bounds.y + i,
                            obj->bounds.width - 2*i, 1, border); // Top
                fb_fill_rect(obj->bounds.x + i, obj->bounds.y + obj->bounds.height - i - 1,
                            obj->bounds.width - 2*i, 1, border); // Bottom
                fb_fill_rect(obj->bounds.x + i, obj->bounds.y + i,
                            1, obj->bounds.height - 2*i, border); // Left
                fb_fill_rect(obj->bounds.x + obj->bounds.width - i - 1, obj->bounds.y + i,
                            1, obj->bounds.height - 2*i, border); // Right
            }
        }
    }

    // Kinder rendern
    for (int i = 0; i < obj->child_count; i++) {
        render_object(obj->children[i], clip);
    }
}

// === Event-Dispatching ===
static void dispatch_event_to_object(gui_object_t* obj, event_t* event) {
    if (!obj || !obj->visible || !obj->enabled) return;

    // Prüfen ob Event für dieses Objekt (bei Maus-Events)
    if (event->type == EVENT_MOUSE_MOVE ||
        event->type == EVENT_MOUSE_DOWN ||
        event->type == EVENT_MOUSE_UP) {

        if (!gui_point_in_rect((point_t){event->data.mouse.x, event->data.mouse.y},
                               obj->bounds)) {
            return;
        }
    }

    // Event an Kind-Objekte weiterleiten (von hinten nach vorne)
    for (int i = obj->child_count - 1; i >= 0; i--) {
        dispatch_event_to_object(obj->children[i], event);
    }

    // Eigenen Handler aufrufen
    if (obj->handle_event) {
        obj->handle_event(obj, event);
    }
}

// === Event Queue ===
void gui_post_event(event_t* event) {
    int next = (gui.event_head + 1) % 256;
    if (next != gui.event_tail) {
        gui.event_queue[gui.event_head] = *event;
        gui.event_head = next;
    }
}

void gui_dispatch_events(void) {
    while (gui.event_tail != gui.event_head) {
        event_t event = gui.event_queue[gui.event_tail];
        gui.event_tail = (gui.event_tail + 1) % 256;

        // Root-Objekt bekommt alle Events
        if (gui.root) {
            dispatch_event_to_object(gui.root, &event);
        }
    }
}

// === Hauptloop ===
void gui_run(void) {
    gui.running = 1;

    while (gui.running) {
        // 1. Events verarbeiten
        gui_dispatch_events();

        // 2. Neuzeichnen nur wenn nötig
        if (gui.dirty_count > 0) {
            for (int i = 0; i < gui.dirty_count; i++) {
                render_object(gui.root, gui.dirty_rects[i]);
            }
            gui.dirty_count = 0;

            // Bei Double-Buffering: Buffer tauschen
            fb_swap_buffers();
        }

        // 3. Kurze Pause (CPU sparen)
        for (volatile int i = 0; i < 1000000; i++);
    }
}

// === Initialisierung ===
void gui_init(void) {
    // Framebuffer initialisieren
    framebuffer_init();

    // Double-Buffering für flüssige Animationen
    fb_init_double_buffer();

    // GUI-Zustand initialisieren
    gui.root = NULL;
    gui.focused = NULL;
    gui.hover = NULL;
    gui.mouse_pos.x = g_fb.width / 2;
    gui.mouse_pos.y = g_fb.height / 2;
    gui.running = 0;
    gui.next_id = 1;
    gui.event_head = gui.event_tail = 0;
    gui.dirty_count = 0;

    // Desktop als Root erstellen
    rect_t desktop_bounds = {0, 0, g_fb.width, g_fb.height};
    gui.root = gui_create_object(desktop_bounds, "Desktop");
    gui.root->bg_color = (color_t){40, 40, 40, 255}; // Dunkelgrau

    kprint("GUI Core initialized\n", COLOR_GREEN);
}

void gui_shutdown(void) {
    if (gui.root) {
        gui_destroy_object(gui.root);
        gui.root = NULL;
    }
    kprint("GUI Core shutdown\n", COLOR_YELLOW);
}
