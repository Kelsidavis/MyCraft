#ifndef KERNEL_UI_H
#define KERNEL_UI_H

#include <stdint.h>

// UI event types
#define UIE_MOUSE_DOWN 1
#define UIE_MOUSE_UP 2
#define UIE_KEY_DOWN 3
#define UIE_KEY_UP 4

// UI event structure
typedef struct {
    int type;
    int x;
    int y;
    int button;
} ui_event_t;

// Rectangle structure
typedef struct {
    int x, y, w, h;
} rect_t;

// Forward declarations
void ui_init(void* fb, int w, int h, int pitch);
void ui_draw(void);
void ui_move_cursor(int dx, int dy);
void ui_cursor_pos(int* x, int* y);
void ui_click(int x, int y);
int ui_next_event(ui_event_t* out);

// Syscall implementations
int ui_sys_fill_rect(int x, int y, int w, int h, uint32_t argb);
int ui_sys_present(void);
int ui_sys_draw_text(int x, int y, const char* s, int len, uint32_t argb);

#endif // KERNEL_UI_H
