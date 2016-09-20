/*
    KolibriGUI demobox
    -Picture Button
    -StaticText
    -File Open/Save Dialog
    -Filebrowser (planned)

    Free for all

    Initially written by Siemargl, 2016


    ToDo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kos32sys.h"
#include "kolibri_gui.h"
#include "kolibri_opendialog.h"
#include "kolibri_libimg.h"

char temp_path[4096];
char** sys_path = (char**)0x20; // hack - get path from KOS header

int main(int argc, char **argv)
{
    /* Load all libraries, initialize global tables like system color table and
    operations table. kolibri_gui_init() will EXIT with mcall -1 if it fails
    to do it's job. This is all you need to call and all libraries and GUI
    elements can be used after a successful call to this function
    */
    kolibri_gui_init();
    kolibri_proclib_init();  // opensave && color dialogs
    kolibri_libimg_init();  // png handling


    int gui_event = KOLIBRI_EVENT_REDRAW;
    uint32_t pressed_button = 0;
//    uint32_t mouse_button;
//    pos_t   mouse_pos;
    oskey_t keypress;

    // load image for buttons
    const int icon_rgb_size = 16*16*3; // every icons 16x16 24bpp
    char *image_data_rgb = malloc(icon_rgb_size * 3),
         *image_data, *pc;
    // make full path + argv
    strcpy(temp_path, *sys_path);
    pc = strrchr(temp_path, '/');
    if (pc) pc[1] = 0;
    strcat(temp_path, "reload_16x16_8b.png");
    debug_board_write_str(temp_path);
    FILE *ficon = fopen(temp_path, "rb");
    if (!ficon)
    {
        debug_board_write_str("no icons file reload_16x16_8b.png ");
        return 1;
    }
    int ficon_size = fread(image_data_rgb, 1, icon_rgb_size * 3, ficon);
    if (ferror(ficon))
    {
        debug_board_write_str("error reading file reload_16x16_8b.png ");
        return 1;
    }
    fclose(ficon);

    // определяем вид изображения и переводим его во временный буфер image_data
    image_data = (*img_decode)(image_data_rgb, ficon_size, 0);
    // преобразуем изображение к формату rgb
    (*img_to_rgb2)(image_data, image_data_rgb);
    // удаляем временный буфер image_data
    (*img_destroy)(image_data);

    // creating GUI using library functions
    kolibri_window *main_window = kolibri_new_window(50, 40, 400, 160, "PictureButton and File dialog demo");

    pict_button tbar[3];
    gui_add_pict_button(main_window, kolibri_pict_button(&tbar[0], X_Y(10, 16), X_Y(10, 16), image_data_rgb, image_data_rgb + icon_rgb_size, image_data_rgb + icon_rgb_size * 2, 24, NULL, 0));
    gui_add_pict_button(main_window, kolibri_pict_button(&tbar[1], X_Y(35, 16), X_Y(10, 16), image_data_rgb, image_data_rgb + icon_rgb_size, image_data_rgb + icon_rgb_size * 2, 24, NULL, 0));
    gui_add_pict_button(main_window, kolibri_pict_button(&tbar[2], X_Y(60, 16), X_Y(10, 16), image_data_rgb, image_data_rgb + icon_rgb_size, image_data_rgb + icon_rgb_size * 2, 24, NULL, 0));

    statictext labels[3];  //  tips
    gui_add_statictext(main_window, kolibri_statictext_def(&labels[0], X_Y(5, 28), "Open"));
    gui_add_statictext(main_window, kolibri_statictext_def(&labels[1], X_Y(35, 28), "Save"));
    gui_add_statictext(main_window, kolibri_statictext_def(&labels[2], X_Y(65, 28), "Select Dir"));

    open_dialog *dlg_opensave = kolibri_new_open_dialog(OPEN, 10, 10, 420, 320);
    (*OpenDialog_init)(dlg_opensave);

    pathview pview;
    gui_add_pathview(main_window, kolibri_pathview(&pview, X_Y(10, 50), 330, 1, 0, dlg_opensave->openfile_path, temp_path, 0, 0)); // black font, no background, font 1

    do  /* Start of main activity loop */
    {
        switch(gui_event)
        {
        case KOLIBRI_EVENT_REDRAW:
            kolibri_handle_event_redraw(main_window);
            break;
        case KOLIBRI_EVENT_NONE:
			break;
        case KOLIBRI_EVENT_KEY:
            keypress = get_key();
            kolibri_handle_event_key(main_window);
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

            if(tbar[0].click)  // open
            {
                tbar[0].click = 0;
                dlg_opensave->mode = OPEN;
                (*OpenDialog_start)(dlg_opensave);
                //debug_board_printf("status == %d, buf = %s\n", dlg_opensave->status, dlg_opensave->openfile_path);
                if (dlg_opensave->status != 2 && dlg_opensave->status != 0) // fail or cancel
                {
                    (*path_show_prepare)(&pview);
                    (*path_show_draw)(&pview);
                }
            }
            if(tbar[1].click)  // save
            {
                tbar[1].click = 0;
                dlg_opensave->mode = SAVE;
                (*OpenDialog_start)(dlg_opensave);
                if (dlg_opensave->status != 2 && dlg_opensave->status != 0) // fail or cancel
                    (*path_show_prepare)(&pview);
                kolibri_handle_event_redraw(main_window);
            }
            if(tbar[2].click)  // select
            {
                tbar[2].click = 0;
                dlg_opensave->mode = SELECT;
                (*OpenDialog_start)(dlg_opensave);
                if (dlg_opensave->status != 2 && dlg_opensave->status != 0) // fail or cancel
                    (*path_show_prepare)(&pview);
                kolibri_handle_event_redraw(main_window);
            }

            break;
        }

        gui_event = get_os_event();
    } while(1) ; /* End of main activity loop */

  return 0;
}

