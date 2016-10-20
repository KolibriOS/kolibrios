/*
    KolibriGUI demobox
    -Picture Button
    -StaticText
    -File Open/Save Dialog
    -Filebrowser
    -Controlling minimal window size

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
#include "kolibri_opendialog.h"
#include "kolibri_libimg.h"

char temp_path[4096];
char** sys_path = (char**)0x20; // hack - get path from KOS header. analog argv[0]

char*   load_file_inmem(char* fname, int32_t* read_sz); // see below
void* read_folderdata(char* name);
void control_minimal_window_size(int wmin, int hmin);

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
    strcpy(temp_path, *sys_path);
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
    kolibri_window *main_window = kolibri_new_window(50, 40, 430, 500, "PictureButton and File dialog demo");

    pict_button tbar[3];
    gui_add_pict_button(main_window, kolibri_pict_button(&tbar[0], X_Y(10, 16), X_Y(10, 16), image_data_rgb, image_data_rgb + icon_rgb_size, image_data_rgb + icon_rgb_size * 2, 24, NULL, 0));
    gui_add_pict_button(main_window, kolibri_pict_button(&tbar[1], X_Y(35, 16), X_Y(10, 16), image_data_rgb, image_data_rgb + icon_rgb_size, image_data_rgb + icon_rgb_size * 2, 24, NULL, 0));
    gui_add_pict_button(main_window, kolibri_pict_button(&tbar[2], X_Y(60, 16), X_Y(10, 16), image_data_rgb, image_data_rgb + icon_rgb_size, image_data_rgb + icon_rgb_size * 2, 24, NULL, 0));

    statictext labels[3];  //  tips
    gui_add_statictext(main_window, kolibri_statictext_def(&labels[0], X_Y(5, 28), "Open"));
    gui_add_statictext(main_window, kolibri_statictext_def(&labels[1], X_Y(35, 28), "Save"));
    gui_add_statictext(main_window, kolibri_statictext_def(&labels[2], X_Y(65, 28), "Select Dir & browse"));

    open_dialog *dlg_opensave = kolibri_new_open_dialog(OPEN, 10, 10, 420, 320);
    (*OpenDialog_init)(dlg_opensave);

    pathview pview;
    gui_add_pathview(main_window, kolibri_pathview(&pview, X_Y(10, 50), 330, 1, 0, dlg_opensave->openfile_path, temp_path, 0, 0)); // black font, no background, font 1

    filebrowser brows;
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
    gui_add_filebrowser(main_window, kolibri_filebrowser(&brows, X_Y(10, 400), X_Y(80, 300), X_Y(6, 9), X_Y(16, 16), image_data_rgb, NULL, 24,
                                         filedata, filedata + read_bytes,
                                         0x00FF00, 0xbbddff, 0x000000, 0xFFFFFF, 0xFF0000));

    // try devices "/" - good
    brows.folder_data = read_folderdata("/rd/1");
    brows.select_panel_counter = 1;  // if want to show selection

    do  /* Start of main activity loop */
    {
        switch(gui_event)
        {
        case KOLIBRI_EVENT_REDRAW:
            control_minimal_window_size(430, 500);
            brows.all_redraw = 1;
            kolibri_handle_event_redraw(main_window);
            brows.all_redraw = 0;
            break;
        case KOLIBRI_EVENT_NONE:
			break;
        case KOLIBRI_EVENT_KEY:
            keypress = get_key();
            filebrowser_key(&brows, keypress);
            //kolibri_handle_event_key(main_window);
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
            brows.select_flag = 0;
            kolibri_handle_event_mouse(main_window);


            if (brows.mouse_keys_delta == 3)  // double clicked in browser
            {
                debug_board_printf("mouse_keys_delta == 3, name %s\n", brows.selected_BDVK_adress->fname);
                brows.mouse_keys_delta = 0;
            }

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

                // just calling line below draws incomplete
                // kolibri_handle_event_redraw(main_window);
            }
            if(tbar[2].click)  // select
            {
                tbar[2].click = 0;
                dlg_opensave->mode = SELECT;
                (*OpenDialog_start)(dlg_opensave);
                if (dlg_opensave->status != 2 && dlg_opensave->status != 0) // fail or cancel
                {
                    (*path_show_prepare)(&pview);
                    free(brows.folder_data);
                    brows.folder_data = read_folderdata(dlg_opensave->openfile_path);
                    brows.start_draw_line = brows.start_draw_cursor_line = 0;
                }
                // we may redraw here, or just wait next redraw event
                brows.all_redraw = 1;
                kolibri_handle_event_redraw(main_window);
                brows.all_redraw = 0;
            }

            break;
        }

        gui_event = get_os_event();
    } while(1) ; /* End of main activity loop */

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

void* read_folderdata(char* name)
{
    struct fs_dirinfo di;
    struct fs_dirheader dhead;
    assert(sizeof di == 25);

    memset(&di, 0, sizeof di);
    di.ppath = name;
    di.retval = (uint32_t)&dhead;
    int rc = sf_file(1, &di);  // read dir size
    if(rc) {
        debug_board_printf("Error reading dir size %s", name);
        exit(1);
    }
    di.size = dhead.totl_blocks;

    char *retdir = malloc(sizeof dhead + dhead.totl_blocks * sizeof(struct fsBDFE));
    if(!retdir) {
        debug_board_printf("No memory for dir %s", name);
        exit(1);
    }
    di.retval = (uint32_t)retdir;
    rc = sf_file(1, &di);  // read dir size
    if(rc) {
        debug_board_printf("Error 2 reading dir size %s", name);
        exit(1);
    }

    // manual clear mark flag (random junk in fname free space)
    int i;
    for (i = 0; i < dhead.totl_blocks; i++)
        ((struct fsBDFE*)(retdir+32))[i].fname[259] = 0;

    debug_board_printf("Loaded dir [%s] etnries %d,\n first file [%s]\n", name, ((struct fs_dirheader*)(retdir))->curn_blocks, ((struct fsBDFE*)(retdir+32))->fname);

    return retdir;
}



void control_minimal_window_size(int wmin, int hmin)
{
    char pinfo[1024];
    get_proc_info(pinfo);

    int win_hight = *(int*)(pinfo + 46),
        win_width = *(int*)(pinfo + 42);
    char win_status = pinfo[70];

    if (win_status & 7) return;  // maximized, minimized or titlebar mode

    if (win_width < wmin)
        __asm__ __volatile__("int $0x40" ::"a"(67), "b"(-1), "c"(-1), "d"(wmin), "S"(-1));  // SF_CHANGE_WINDOW x,y,w,h
    if (win_hight < hmin)
        __asm__ __volatile__("int $0x40" ::"a"(67), "b"(-1), "c"(-1), "d"(-1), "S"(hmin));  // x,y,w,h

}


