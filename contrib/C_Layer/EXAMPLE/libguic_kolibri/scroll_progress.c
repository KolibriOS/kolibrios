/*
    KolibriGUI demobox
    -Scrollbar
    -Progressbar
    -StaticText
    -StaticNum

    Free for all

    Initially written by Siemargl, 2016


    ToDo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kos32sys.h"
#include "kolibri_gui.h"

// codes copied from \programs\cmm\lib\keyboard.h, but they're decimal
//ASCII KEYS
#define ASCII_KEY_BS    8
#define ASCII_KEY_TAB   9
#define ASCII_KEY_ENTER 13
#define ASCII_KEY_ESC   27
#define ASCII_KEY_DEL   182
#define ASCII_KEY_INS   185
#define ASCII_KEY_SPACE 32

#define ASCII_KEY_LEFT  176
#define ASCII_KEY_RIGHT 179
#define ASCII_KEY_DOWN  177
#define ASCII_KEY_UP    178
#define ASCII_KEY_HOME  180
#define ASCII_KEY_END   181
#define ASCII_KEY_PGDN  183
#define ASCII_KEY_PGUP  184

//SCAN CODE KEYS
#define SCAN_CODE_BS    14
#define SCAN_CODE_TAB   15
#define SCAN_CODE_ENTER 28
#define SCAN_CODE_ESC   1
#define SCAN_CODE_DEL   83
#define SCAN_CODE_INS   82
#define SCAN_CODE_SPACE 57

#define SCAN_CODE_LEFT  75
#define SCAN_CODE_RIGHT 77
#define SCAN_CODE_DOWN  80
#define SCAN_CODE_UP    72
#define SCAN_CODE_HOME  71
#define SCAN_CODE_END   79
#define SCAN_CODE_PGDN  81
#define SCAN_CODE_PGUP  73

#define KEY_LSHIFT     00000000001b
#define KEY_RSHIFT     00000000010b
#define KEY_LCTRL      00000000100b
#define KEY_RCTRL      00000001000b
#define KEY_LALT       00000010000b
#define KEY_RALT       00000100000b
#define KEY_CAPSLOCK   00001000000b
#define KEY_NUMLOCK    00010000000b
#define KEY_SCROLLLOCK 00100000000b
#define KEY_LWIN       01000000000b
#define KEY_RWIN       10000000000b


int main()
{
    /* Load all libraries, initialize global tables like system color table and
    operations table. kolibri_gui_init() will EXIT with mcall -1 if it fails
    to do it's job. This is all you need to call and all libraries and GUI
    elements can be used after a successful call to this function
    */
    kolibri_gui_init();
    int gui_event = KOLIBRI_EVENT_REDRAW;
    uint32_t pressed_button = 0;
//    uint32_t mouse_button;
//    pos_t   mouse_pos;
    oskey_t keypress;

    int value = 40; // showed value
    int valuechange = 0;

    // creating GUI using library functions
    kolibri_window *main_window = kolibri_new_window(50, 40, 400, 400, "Scrollbar and progressbar showcase");
    statictext *txt = kolibri_new_statictext_def(X_Y(10, 20), "StaticText default 6x9. Use Arrows or PgUp/PgDn");
    statictext *txt2 = kolibri_new_statictext(X_Y(10, 30), "StaticText 8x16 x2:", CP866, 1, kolibri_color_table.color_work_text, 0);
    staticnum *num = kolibri_new_staticnum_def(X_Y(10 + (strlen("StaticText 8x16 x2:") + 2) * 8 * 2, 30), 3, value);
    scrollbar *sbh = kolibri_new_scrollbar(X_Y(30, 300), X_Y(350, 15), 15, 100, 10, value, kolibri_color_table.color_work_area, kolibri_color_table.color_work_button, 0);
    scrollbar *sbv = kolibri_new_scrollbar(X_Y(370, 15), X_Y(40, 300), 15, 100, 10, value, kolibri_color_table.color_work_area, kolibri_color_table.color_work_button, 0);
    progressbar *pg = kolibri_new_progressbar(0, 100, value, 10, 70, 200, 20);

    kolibri_window_add_element(main_window, KOLIBRI_STATICTEXT, txt);
    kolibri_window_add_element(main_window, KOLIBRI_STATICTEXT, txt2);
    kolibri_window_add_element(main_window, KOLIBRI_STATICNUM, num);
    kolibri_window_add_element(main_window, KOLIBRI_SCROLL_BAR_H, sbh);
    kolibri_window_add_element(main_window, KOLIBRI_SCROLL_BAR_V, sbv);
    kolibri_window_add_element(main_window, KOLIBRI_PROGRESS_BAR, pg);

    do  /* Start of main activity loop */
    {
        switch(gui_event)
        {
        case KOLIBRI_EVENT_REDRAW:
            sbh->all_redraw = sbv->all_redraw = 1; // resetted by sbar
            kolibri_handle_event_redraw(main_window);
            valuechange = 0;
            break;
        case KOLIBRI_EVENT_NONE:
			break;
        case KOLIBRI_EVENT_KEY:
            keypress = get_key();
            // add logic to find focused active widget
            // we have only one reaction
            switch (keypress.ctrl_key)
            {
              case SCAN_CODE_UP:  case SCAN_CODE_RIGHT:
                  if(value < 100)
                    {
                        value++; valuechange = 1;
                        progressbar_progress(pg); // +1 and redraw self
                    }
                break;
              case SCAN_CODE_PGUP:
                  if(value < 100)
                    {
                        value += 10; valuechange = 1;
                        if (value > 100) value = 100;
                    }
                break;
              case SCAN_CODE_DOWN:  case SCAN_CODE_LEFT:
                  if(value > 0)
                    {
                        value--; valuechange = 1;
                    }
                break;
              case SCAN_CODE_PGDN:
                  if(value > 0)
                    {
                        value -= 10; valuechange = 1;
                        if (value < 0) value = 0;
                    }
                break;
            }
            kolibri_handle_event_key(main_window, keypress);
			break;
        case KOLIBRI_EVENT_BUTTON:
            pressed_button = get_os_button();
            switch (pressed_button)
            {
              case BTN_QUIT:
                return 0;
                break;
            }
            break;
        case KOLIBRI_EVENT_MOUSE:
//            mouse_pos = get_mouse_pos(POS_WINDOW); // window relative
//            mouse_button = get_mouse_eventstate();
            // add logic to find widget under mouse
            kolibri_handle_event_mouse(main_window);
            // we can optimize a lot, if sb->delta2 == 1 then call sb->mouse() and redraw only if sb->redraw with flag all_redraw = 0 (only runner). Then reset redraw. OMG (
            if (sbh->position != value)  // scrollbars was changed
            {
                value = sbh->position;
                valuechange = 1;
            }else
            if (sbv->position != value)  // scrollbars was changed
            {
                value = sbv->position;
                valuechange = 1;
            }
/*            debug_board_printf("mouse ev (%d,%d)%x\n", mouse_pos.x, mouse_pos.y, mouse_button);
            if (mouse_button & (1<<24)) // double click
            {
            }
            // ignore
*/
            break;
        }
        if(valuechange)
        {
            debug_board_printf("value change (%d)\n", value);
            num->number = value;
            sbh->position = value;
            sbv->position = value;
            pg->value = value;
            gui_event = KOLIBRI_EVENT_REDRAW;
            continue;
        }

        gui_event = wait_for_event(10); // 100 = 1 sec
    } while(1) ; /* End of main activity loop */

  return 0;
}
/*
void __attribute__ ((noinline)) debug_board_write_str(const char* str){
  while(*str)
    debug_board_write_byte(*str++);
}

void __attribute__ ((noinline)) debug_board_printf(const char *format,...)
{
        va_list ap;
        char log_board[300];

        va_start (ap, format);
        tiny_vsnprintf(log_board, sizeof log_board, format, ap);
        va_end(ap);
        debug_board_write_str(log_board);

}
*/
