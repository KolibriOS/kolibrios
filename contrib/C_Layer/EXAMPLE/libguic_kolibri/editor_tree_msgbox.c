
/*
    KolibriGUI demobox
    -Editor (multiline edit)
    -TreeView
    -MsgBox Dialog

    Free for all

    Initially written by Siemargl, 2016


    ToDo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "kos32sys.h"
#include "kolibri_gui.h"
#include "kolibri_libimg.h"
#include "kolibri_msgbox.h"

char temp_path[4096];

char*   load_file_inmem(char* fname, int32_t* read_sz); // see below

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
    char *image_data_rgb,
         *image_data,
         *filedata;
    // make full path + argv
    strcpy(temp_path, argv[0]);
    char *pc = strrchr(temp_path, '/');  // this fails if has params with '/' within. use argv[0] instead
    if (pc) pc[1] = 0;
    strcat(temp_path, "reload_16x16_8b.png");
//    debug_board_write_str(temp_path);

    int32_t read_bytes;
    filedata = load_file_inmem(temp_path, &read_bytes);
    image_data_rgb = malloc(icon_rgb_size * 3);  // we know size
    // определяем вид изображения и переводим его во временный буфер image_data
    image_data = (*img_decode)(filedata, read_bytes, 0);
    // преобразуем изображение к формату rgb
    (*img_to_rgb2)(image_data, image_data_rgb);
    // удаляем временный буфер image_data
    (*img_destroy)(image_data);
    free(filedata);

    // creating GUI using library functions
    kolibri_window *main_window = kolibri_new_window(50, 40, 470, 500, "Editor, TreeView and MsgBox demo");

    editor *ed;
    editor *ed_lock;

    gui_add_editor(main_window, ed = kolibri_new_editor(X_Y(10, 440), X_Y(20, 150), 0x1001, 2048, &ed_lock));
    ed_lock = ed;


    filedata = load_file_inmem("/rd/1/File managers/z_icons.png", &read_bytes);
    image_data_rgb = malloc(icon_rgb_size * 20);  // we know size
    // определяем вид изображения и переводим его во временный буфер image_data
    image_data = (*img_decode)(filedata, read_bytes, 0);
    // преобразуем изображение к формату rgb
    (*img_to_rgb2)(image_data, image_data_rgb);
    // удаляем временный буфер image_data
    (*img_destroy)(image_data);
    free(filedata);

    filedata = load_file_inmem("/rd/1/File managers/icons.ini", &read_bytes);


    int extended_key = 0, act = 0;
    msgbox* box = kolibri_new_msgbox("Exit", "Are\rYOU\rSure?", 3, "YES", "Absolute", "Not Yet", NULL);  // default NOT

    do  /* Start of main activity loop */
    {
        switch(gui_event)
        {
        case KOLIBRI_EVENT_REDRAW:
            if (box->retval == 1 || box->retval == 2) return 0;  // msgbox closes

            kolibri_handle_event_redraw(main_window);
            break;
        case KOLIBRI_EVENT_NONE:
			break;
        case KOLIBRI_EVENT_KEY:
            if (ed_lock == ed)
                editor_key(ed);
            else
                kolibri_handle_event_key(main_window);
			break;
        case KOLIBRI_EVENT_BUTTON:
            pressed_button = get_os_button();
            switch (pressed_button)
            {
              case BTN_QUIT:
                  if (box->retval == 3 || box->retval == 0) // not started or cancelled, analyze when redraw after closing msgbox
                    kolibri_start_msgbox(box, NULL);
                return 0;
                break;
            }
            break;
        case KOLIBRI_EVENT_MOUSE:
            kolibri_handle_event_mouse(main_window);
            break;
        }

        gui_event = get_os_event();
    } while(1) ; /* End of main activity loop */

  return 0;
}
