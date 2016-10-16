
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

char run_path[4096];
char fname[4096];

char*   load_file_inmem(char* fname, int32_t* read_sz); // see below
char*   load_image_file(char* fname); // see below

int main(int argc, char **argv)
{
    /* Load all libraries, initialize global tables like system color table and
    operations table. kolibri_gui_init() will EXIT with mcall -1 if it fails
    to do it's job. This is all you need to call and all libraries and GUI
    elements can be used after a successful call to this function
    */
    kolibri_gui_init();
//    kolibri_proclib_init();  // opensave && color dialogs
    kolibri_libimg_init();  // png handling

    int gui_event = KOLIBRI_EVENT_REDRAW;
    uint32_t pressed_button = 0;
//    uint32_t mouse_button;
//    pos_t   mouse_pos;
    oskey_t keypress;

    // make full path + argv
    strcpy(run_path, argv[0]);
    char *pc = strrchr(run_path, '/');  // this fails if has params with '/' within. use argv[0] instead
    if (pc) pc[1] = 0;
//    debug_board_write_str(temp_path);

    // creating GUI using library functions
    kolibri_window *main_window = kolibri_new_window(50, 40, 490, 500, "Editor, TreeView and MsgBox demo");

    editor *ed;
    editor *ed_lock;

    gui_add_editor(main_window, ed = kolibri_new_editor(X_Y(0, 440), X_Y(20, 150), 0x1001, 2048, &ed_lock));
    ed_lock = ed;

    treelist *tl = kolibri_new_treelist(X_Y(0, 440), X_Y(200, 200), 16, X_Y(16, 16), 100, 50, 0, 0, TL_KEY_NO_EDIT | TL_DRAW_PAR_LINE, &ed_lock, 0x8080ff, 0x0000ff, 0xffffff);
    treelist_data_init(tl);

    // читаем файл с курсорами и линиями
    strcpy(fname, run_path);
    strcat(fname, "tl_sys_16.png");
    tl->data_img_sys = load_image_file(fname);

    // читаем файл с иконками узлов
    strcpy(fname, run_path);
    strcat(fname, "tl_nod_16.png");
    tl->data_img = load_image_file(fname);

    treelist_node_add(tl, "node1", 1, 0, 0); // где 1 номер иконки с книгой
    treelist_cursor_next(tl);

    treelist_node_add(tl, "node2", 1, 0, 0);
    treelist_cursor_next(tl);

    treelist_node_add(tl, "node3", 1, 0, 0);
    treelist_cursor_next(tl);

    treelist_cursor_begin(tl); //;ставим курсор на начало списка
    gui_add_treelist(main_window, tl);

    msgbox* box = kolibri_new_msgbox("Exit", "Are\rYOU\rSure?", 3, "YES", "Absolute", "Not Yet", NULL);  // default NOT

    do  /* Start of main activity loop */
    {
        switch(gui_event)
        {
        case KOLIBRI_EVENT_REDRAW:
            if (box->retval == 1 || box->retval == 2) goto clearing;  // msgbox closes

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
                break;
            }
            break;
        case KOLIBRI_EVENT_MOUSE:
            kolibri_handle_event_mouse(main_window);
            break;
        }

        gui_event = get_os_event();
    } while(1) ; /* End of main activity loop */

clearing:
    editor_delete(ed);
    treelist_data_clear(tl);

  return 0;
}


char*   load_file_inmem(char* fname, int32_t* read_sz)
{
    FILE *f = fopen(fname, "rb");
    if (!f) {
        debug_board_printf("Can't open file: %s", fname);
        exit(1);
    }
    if (fseek(f, 0, SEEK_END)) {
        debug_board_printf("Can't SEEK_END file: %s", fname);
        exit(1);
    }
    int filesize = ftell(f);
    rewind(f);
    char* fdata = malloc(filesize);
    if(!fdata) {
        debug_board_printf("No memory for file %s", fname);
        exit(1);
    }
    *read_sz = fread(fdata, 1, filesize, f);
    if (ferror(f)) {
        debug_board_printf("Error reading file %s", fname);
        exit(1);
    }
    fclose(f);

    return fdata;
}

char*   load_image_file(char* fname)
{
    int32_t     read_bytes = -1, w, h;
    char    *image_data = 0, *image_data_rgb = 0, *filedata = 0;

    filedata = load_file_inmem(fname, &read_bytes);
    // определяем вид изображения и переводим его во временный буфер image_data
    image_data = (*img_decode)(filedata, read_bytes, 0);
    w = *(int*)(image_data +4);
    h = *(int*)(image_data +8);
    image_data_rgb = malloc(w * h * 3);
    // преобразуем изображение к формату rgb
    (*img_to_rgb2)(image_data, image_data_rgb);
    // удаляем временный буфер image_data
    (*img_destroy)(image_data);
    free(filedata);

    return image_data_rgb;
}

