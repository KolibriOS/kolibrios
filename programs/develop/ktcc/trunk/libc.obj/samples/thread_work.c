/*
 * An example of using threads to create a copy of a window.
 * Built on top of the /programs/develop/examples/thread/trunk/thread.asm example.
 *
 * Created by turbocat (Maxim Logaev) 2021.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ksys.h>

#define TH_STACK_SIZE 1024

enum BUTTONS {
    BTN_QUIT = 1,
    BTN_CREATE_TH = 2,
};

ksys_colors_table_t sys_colors;

extern int main();

void redraw_window(void)
{
    ksys_pos_t mouse_pos = _ksys_get_mouse_pos(KSYS_MOUSE_SCREEN_POS);
    _ksys_start_draw();
    _ksys_create_window(mouse_pos.x, mouse_pos.y, 140, 60, "Threads", sys_colors.work_area, 0x14);
    _ksys_define_button(10, 30, 120, 20, BTN_CREATE_TH, sys_colors.work_button);
    _ksys_draw_text("Create thread!", 15, 34, 0, 0x90000000 | sys_colors.work_button_text);
    _ksys_end_draw();
}

void create_thread(void)
{
    unsigned tid;                           // New thread ID
    void* th_stack = malloc(TH_STACK_SIZE); // Allocate memory for thread stack
    if (!th_stack) {
        _ksys_debug_puts("Memory allocation error for thread!");
        return;
    }
    tid = _ksys_create_thread(main, th_stack + TH_STACK_SIZE); // Create new thread with entry "main"
    if (tid == -1) {
        _ksys_debug_puts("Unable to create a new thread!");
        return;
    }
    debug_printf("New thread created (TID=%u)\n", tid);
}

int main()
{
    _ksys_get_system_colors(&sys_colors);
    int gui_event;
    redraw_window();

    while (1) {
        gui_event = _ksys_get_event();
        switch (gui_event) {
        case KSYS_EVENT_REDRAW:
            redraw_window();
            break;
        case KSYS_EVENT_BUTTON:
            switch (_ksys_get_button()) {
            case BTN_CREATE_TH:
                create_thread();
                break;
            case BTN_QUIT:
                _ksys_exit();
            }
            break;
        }
    }
}
