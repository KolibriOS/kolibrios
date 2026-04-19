// BOXLIB EXAMPLE (scrollbar, progressbar, editbox and checkbox)
// Writed by maxcodehack and superturbocat2001

#include <clayer/boxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

#define WIN_W 640
#define WIN_H 563

#define ED_BUFF_LEN 50
#define TEXT_SIZE 0x10000000
#define SCROLL_BUTTON_SIZE 15
#define SCROLL_MAX_LEN 215
#define BLACK 0x000000
#define WHITE 0xFFFFFF
#define BLUE 0x0000FF
#define X_W(X, W) ((X << 16) + W)
#define Y_H X_W

uint32_t wheels;
char* title = "Boxlib example";
char ed_buff[ED_BUFF_LEN];

scrollbar scroll = { 15, WIN_W - 26, WIN_H - 29, 0, 0, 2, 215, SCROLL_BUTTON_SIZE, 0, 0x707070, 0xD2CED0, 0x555555 };
progressbar pg = { 0, 10, 10, 270, 35, 1, 0, 200, 0xB4B4B4, 0x2728FF, 0xA9A9A9 };
edit_box ed = { WIN_W - 140, 10, 60, 0xFFFFFF, 0x6a9480, 0, 0x6a9480, BLACK | TEXT_SIZE, ED_BUFF_LEN, ed_buff, NULL, ed_focus };
check_box output_off = { X_W(10, 15), Y_H(120, 15), 10, WHITE, BLUE, BLACK | TEXT_SIZE, "Disable duplicate output", 0 };

void draw_window()
{
    _ksys_start_draw();
    _ksys_create_window(215, 100, WIN_W, WIN_H, title, 0x858585, 0x34);
    edit_box_draw(&ed);
    check_box_draw2(&output_off);
    if (!output_off.flags) {
        _ksys_draw_text(ed_buff, 10, 90, strlen(ed_buff), BLACK | TEXT_SIZE);
    }
    scrollbar_v_draw(&scroll);
    progressbar_draw(&pg);
    _ksys_end_draw();
}

int main()
{
    init_checkbox2(&output_off);
    _ksys_set_event_mask(KSYS_EVM_REDRAW + KSYS_EVM_KEY + KSYS_EVM_BUTTON + KSYS_EVM_MOUSE + KSYS_EVM_MOUSE_FILTER);
    while (1) {
        switch (_ksys_get_event()) {
        case KSYS_EVENT_BUTTON:
            if (_ksys_get_button() == 1)
                return 0;
            break;

        case KSYS_EVENT_KEY:
            edit_box_key_safe(&ed, _ksys_get_key());
            draw_window();
            break;

        case KSYS_EVENT_REDRAW:
            draw_window();
            break;

        case KSYS_EVENT_MOUSE:
            edit_box_mouse(&ed);
            scrollbar_v_mouse(&scroll);
            pg.value = scroll.position;
            progressbar_draw(&pg);
            check_box_mouse2(&output_off);
            unsigned int scroll_strong = 10;
            wheels = _ksys_get_mouse_wheels();
            if (wheels & 0xFFFF) {
                if ((short)wheels > 0) {
                    scroll.position += scroll_strong;
                    if (scroll.position > scroll.max_area - scroll.cur_area) {
                        scroll.position = scroll.max_area - scroll.cur_area;
                    }
                } else if ((short)wheels < 0 && scroll.position > 0) {
                    scroll.position -= scroll_strong;
                    if ((int)scroll.position < 0) {
                        scroll.position = 0;
                    }
                }
                scrollbar_v_draw(&scroll);
            }
            break;
        }
    }
    return 0;
}
