#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <kos/libimg.h>
#include <kos32sys1.h>

#define BTN_QUIT  1

struct kolibri_system_colors sys_color_table;

char proc_info[1024];
char temp_path[4096];
char* image_data_rgb;  // decoded image

/* load file to heap */
char*   load_file_inmem(char* fname, int32_t* read_sz);

void draw_window()
{
    int win_hight, win_width, i, pos_x = 5, pos_y = get_skin_height();

    begin_draw();

    sys_create_window(10, 40, 600, 400, "My window", 0xFFFFFF, 0x13);

    get_proc_info(proc_info);
    win_width = *(int*)(proc_info + 0x3E); // client, 2A windows
    win_hight = *(int*)(proc_info + 0x42); // client, 2E windows

    draw_bitmap(image_data_rgb, pos_x, pos_y , 400, 600);
    draw_bitmap(image_data_rgb, pos_x, pos_y, 16, 16);

    end_draw();
}

int main(int argc, char **argv)
{
    int gui_event;
    uint32_t pressed_button = 0, mouse_button;
    pos_t   mouse_pos;

    if (-1 == kolibri_libimg_init())  // png handling
    {
                debug_board_printf("error loading 'lin_img.obj'\n");
                exit(1);
        }

    // load image
    const int icon_rgb_size = 2400*2000; // file demo is 2400x2000x8bpp
    char *image_data, *filedata;
    strcpy(temp_path, "/kolibrios/res/wallpapers/in_the_wind.png");
    debug_board_write_str(temp_path);

    int32_t read_bytes;
    filedata = load_file_inmem(temp_path, &read_bytes);
    image_data_rgb = malloc(icon_rgb_size * 3);
    image_data = img_decode(filedata, read_bytes, 0);
    img_to_rgb2(image_data, image_data_rgb);
    img_destroy(image_data);
    free(filedata);



    get_system_colors(&sys_color_table);
    set_event_mask(0xC0000027); // mouse events only when focused window and mouse inside

    do  /* Start of main activity loop */
    {
//        gui_event = wait_for_event(10); // 100 = 1 sec, case you have background work
        gui_event = get_os_event();
        switch(gui_event)
        {
        case KOLIBRI_EVENT_NONE:
            // background work
                        break;
        case KOLIBRI_EVENT_REDRAW:
            draw_window();
                        break;
        case KOLIBRI_EVENT_KEY:
            // scroll
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
            mouse_pos = get_mouse_pos(POS_WINDOW); // window relative
            mouse_button = get_mouse_eventstate();
            debug_board_printf("mouse ev (%d,%d)%x\n", mouse_pos.x, mouse_pos.y, mouse_button);
            if (mouse_button & (1<<24)) // double click
            {
                debug_board_printf("double click\n");
            }
            // ignore
            break;
        }
    } while(1) ; /* End of main activity loop */

  return 0;
}

char*  load_file_inmem(char* fname, int32_t* read_sz)
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
