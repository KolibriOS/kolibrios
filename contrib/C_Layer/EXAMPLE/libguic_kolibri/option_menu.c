/*
    KolibriGUI demobox
    -OptionBox
    -MenuBar
    -Frame

    Free for all

    Initially written by Siemargl, 2016


    ToDo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kos32sys.h"
#include "kolibri_gui.h"

#define SCAN_CODE_ALTM  50
#define SCAN_CODE_ALTE  18

static inline uint32_t get_os_keyb_modifiers()
{
    register uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(66), "b"(3));
    return val;
};

#define KEY_LSHIFT     0x1
#define KEY_RSHIFT     0x2
#define KEY_LCTRL      0x4
#define KEY_RCTRL      0x8
#define KEY_LALT       0x10
#define KEY_RALT       0x20
#define KEY_CAPSLOCK   0x40
#define KEY_NUMLOCK    0x80
#define KEY_SCROLLLOCK 0x100
#define KEY_LWIN       0x200
#define KEY_RWIN       0x400

static inline void set_os_keyb_mode(int mode)
// 0 - ASCII, 1 - SCAN
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(66), "b"(1), "c"(mode));
};

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


    // creating GUI using library functions
    kolibri_window *main_window = kolibri_new_window(50, 40, 400, 160, "OptionBox and Menu demo");
    //check_box *checkbox = kolibri_new_check_box(20, 45, 12, 12, "Append BOARDMSG to entered message.");

    option_box opts1[3];
    option_box *option1sel = opts1; // intially selected RED

    option_box *op1_1 = gui_optionbox(opts1, X_Y(20, 50), "G1 Item RED", &option1sel);
    option_box *op1_2 = gui_optionbox(opts1+1, X_Y(20, 70), "G1 Item GREEN", &option1sel);
    option_box *op1_3 = gui_optionbox(opts1+2, X_Y(20, 90), "G1 Item BLUE", &option1sel);
    option_box* option_group1[] = {op1_1, op1_2, op1_3, NULL};

    option_box opts2[3];
    option_box *option2sel = opts2 + 1; // intially selected #2
    option_box *op2_1 = gui_optionbox(&opts2[0], X_Y(140, 50), "G2 Item 1st", &option2sel);
    option_box *op2_2 = gui_optionbox(&opts2[1], X_Y(140, 70), "G2 Item 2nd", &option2sel);
    option_box *op2_3 = gui_optionbox(&opts2[2], X_Y(140, 90), "G2 Item 3rd", &option2sel);
    option_box* option_group2[] = {op2_1, op2_2, op2_3, NULL};

    frame *fr1 = kolibri_new_frame_def(X_Y(12, 110), X_Y(40, 70), "Option 1");
    frame *fr2 = kolibri_new_frame_def(X_Y(132, 100), X_Y(40, 70), "Option 2");

    gui_add_optiongroup(main_window, option_group1);  // new syntax
    gui_add_optiongroup(main_window, option_group2);
    gui_add_frame(main_window, fr1);
    gui_add_frame(main_window, fr2);

    int option_index1 = 0;  // index of selected option
    int option_index2 = 0;

    char *menu1stru[] = {"Menu1", "Set RED", "Set GREEN", "Set BLUE", NULL};
    menubar* menu1 = kolibri_new_menubar_def(X_Y(10, 40), X_Y(5, 15), 80, 100, menu1stru);
    gui_add_menubar(main_window, menu1);

    char *menu2stru[] = {"mEnu2", "Set Option 1", "Set Option 2", "Set Option 3", NULL};
    menubar* menu2 = kolibri_new_menubar_def(X_Y(50, 40), X_Y(5, 15), 80, 100, menu2stru);
    gui_add_menubar(main_window, menu2);

    char *menu3stru[] = {"Quit", NULL};
    menubar* menu3 = kolibri_new_menubar_def(X_Y(90, 40), X_Y(5, 15), 0, 0, menu3stru);
    menu3->type = 1;  // no subitems
    gui_add_menubar(main_window, menu3);



    set_os_keyb_mode(1); // needed for keyboard use in menu


    do  /* Start of main activity loop */
    {
        if(option_index1 != option1sel - opts1)
            debug_board_printf("Option1 change to %d\n", option1sel - opts1);
        if(option_index2 != option2sel - opts2)
            debug_board_printf("Option2 change to %d\n", option2sel - opts2);
        option_index1 = option1sel - opts1;
        option_index2 = option2sel - opts2;

        switch(gui_event)
        {
        case KOLIBRI_EVENT_REDRAW:
            kolibri_handle_event_redraw(main_window);
            break;
        case KOLIBRI_EVENT_NONE:
			break;
        case KOLIBRI_EVENT_KEY:
            keypress = get_key();
            debug_board_printf("Key pressed state(%d) code(%d) ctrl_key(%d)  modifiers(%#x)\n", keypress.state, keypress.code, keypress.ctrl_key, get_os_keyb_modifiers());
            kolibri_handle_event_key(main_window, keypress);

            if(keypress.code == SCAN_CODE_ALTM && get_os_keyb_modifiers() & (KEY_LALT | KEY_RALT))
                (*menu_bar_activate)(menu1); // wont work, immediately redraw command closes menu (  . but Alt+F1 worked in opendial.asm:463

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
            kolibri_handle_event_mouse(main_window);
            if(menu1->click && menu1->cursor_out)
            {
                option1sel = opts1 + menu1->cursor_out - 1; // check bounds ?
                (*option_box_draw)(option_group1);
            }
            if(menu2->click && menu2->cursor_out)
            {
                option2sel = opts2 + menu2->cursor_out - 1; // check bounds ?
                (*option_box_draw)(option_group2);
            }
            if(menu3->click && menu3->cursor_out)
            {
                return 0; // quit
            }

            break;
        }

        gui_event = get_os_event();
    } while(1) ; /* End of main activity loop */

  return 0;
}

