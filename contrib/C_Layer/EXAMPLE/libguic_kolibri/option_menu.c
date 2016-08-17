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
    kolibri_window *main_window = kolibri_new_window(50, 50, 400, 160, "OptionBox and Menu demo");
    //check_box *checkbox = kolibri_new_check_box(20, 45, 12, 12, "Append BOARDMSG to entered message.");

    option_box opts1[3];
    option_box *option1sel = opts1; // intially selected RED

    option_box *op1_1 = gui_optionbox(opts1, X_Y(20, 60), "G1 Item RED", &option1sel);
    option_box *op1_2 = gui_optionbox(opts1+1, X_Y(20, 80), "G1 Item GREEN", &option1sel);
    option_box *op1_3 = gui_optionbox(opts1+2, X_Y(20, 100), "G1 Item BLUE", &option1sel);
    option_box* option_group1[] = {op1_1, op1_2, op1_3, NULL};

    option_box opts2[3];
    option_box *option2sel = opts2 + 1; // intially selected #2
    option_box *op2_1 = gui_optionbox(&opts2[0], X_Y(140, 60), "G2 Item 1st", &option2sel);
    option_box *op2_2 = gui_optionbox(&opts2[1], X_Y(140, 80), "G2 Item 2nd", &option2sel);
    option_box *op2_3 = gui_optionbox(&opts2[2], X_Y(140, 100), "G2 Item 3rd", &option2sel);
    option_box* option_group2[] = {op2_1, op2_2, op2_3, NULL};

    frame *fr1 = kolibri_new_frame_def(X_Y(12, 110), X_Y(50, 70), "Option 1");
    frame *fr2 = kolibri_new_frame_def(X_Y(132, 100), X_Y(50, 70), "Option 2");

    gui_add_optiongroup(main_window, option_group1);  // new syntax
    gui_add_optiongroup(main_window, option_group2);
    gui_add_frame(main_window, fr1);
    gui_add_frame(main_window, fr2);

    int option_index1 = 0;  // index of selected option
    int option_index2 = 0;

    static char *menu1t = "Menu1";
    static char *menu11t = "Set RED";
    static char *menu12t = "Set GREEN";
    static char *menu13t = "Set BLUE";
    static char *menu14t = "";
    menubar* menu1 = kolibri_new_menubar_def(X_Y(20, 40), X_Y(25, 15), 80, 100, menu1t, menu11t);
    gui_add_menubar(main_window, menu1);

    static char *menu2t = "Menu2";
    static char *menu21t = "Set Option 1";
    static char *menu22t = "Set Option 2";
    static char *menu23t = "Set Option 3";
    static char *menu24t = "";
    menubar* menu2 = kolibri_new_menubar_def(X_Y(60, 40), X_Y(25, 15), 80, 100, menu2t, menu21t);
    gui_add_menubar(main_window, menu2);




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
            kolibri_handle_event_key(main_window); // ???????
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
            break;
        }

        gui_event = get_os_event();
    } while(1) ; /* End of main activity loop */

  return 0;
}

