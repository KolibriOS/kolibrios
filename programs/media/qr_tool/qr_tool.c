/* Copyright (C) 2021- Rustem Gimadutdinov (rgimad), GPLv2 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <kos32sys.h>

#include "kolibri_gui.h"
#include "kolibri_opendialog.h"
#include "kolibri_libimg.h"

#include "quirc.h"

/* ------------------------------------------------------- */

#define X_W(X, W) ((X<<16)+W)
#define Y_H X_W

#define WINDOW_WIDTH 650
#define WINDOW_HEIGHT 320

#define WINDOW_ROW1_Y 34
#define WINDOW_COL2_X 280

#define TEDIT1_MAXCHAR 2048
#define TEDIT1_W 340
#define TEDIT1_H 150

#define IMG1_DISPLAY_W 256
#define IMG1_DISPLAY_H 256

const char WINDOW_TITLE[] = "QR Tool 0.2b";
 
kolibri_system_colors sys_color_table;
const color_t DRAWTEXT_FLAG_DEFAULT = 0x90000000;

enum MYCOLORS {
    COL_GREEN = 0x067D06,
    COL_BLUE  = 0x0000FF,
    COL_RED   = 0xFF0000,
    COL_BLACK = 0x000000,
    COL_WHITE = 0xFFFFFF,
    COL_GREY  = 0x919191
};

enum MYBUTTONS {
    MYBTN_QUIT = 1,
    MYBTN_OPEN = 10/*,
    MYBTN_CLEAR = 20*/
    //
};

editor *tedit1;
void *tedit1_lock;

open_dialog *op_dialog1;

Image *img1 = NULL;
Image *img1_grayscale = NULL;
char *img1_path;

char cur_dir_path[256];


char* load_img(char* fname, uint32_t* read_sz) {
    FILE *f = fopen(fname, "rb");
    if (!f) {
        printf("Can't open file: %s\n", fname);
        exit(0);
    }
    if (fseek(f, 0, SEEK_END)) {
        printf("Can't SEEK_END file: %s\n", fname);
        exit(0);
    }
    int filesize = ftell(f);
    rewind(f);
    char* fdata = malloc(filesize);
    if(!fdata) {
        printf("No memory for file %s\n", fname);
        exit(0);
    }
    *read_sz = fread(fdata, 1, filesize, f);
    if (ferror(f)) {
        printf("Error reading file %s\n", fname);
        exit(0);
    }
    fclose(f);
    return fdata;
}

/* mode: 0 - ASCII, 1 - SCAN*/
void set_os_keyb_mode(int mode) { __asm__ __volatile__("int $0x40"::"a"(66), "b"(1), "c"(mode)); }

void redraw_window() {
    pos_t win_pos = get_mouse_pos(0);
    begin_draw();
    sys_create_window(win_pos.x, win_pos.y, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, sys_color_table.color_work_area, 0x14);
 
    //draw_text_sys("", /*x*/, /*y*/, 0, DRAWTEXT_FLAG_DEFAULT | sys_color_table.work_text);
 
    define_button(X_W(WINDOW_COL2_X,100), Y_H(WINDOW_ROW1_Y,30), MYBTN_OPEN, sys_color_table.color_work_button);
    draw_text_sys("Open image", WINDOW_COL2_X + 12, WINDOW_ROW1_Y + 7, 0, DRAWTEXT_FLAG_DEFAULT | sys_color_table.color_work_button_text);

    //define_button(X_W(WINDOW_COL2_X + 120,100), Y_H(WINDOW_ROW1_Y,30), MYBTN_CLEAR, sys_color_table.color_work_button);

    draw_text_sys("Recognition results below.", WINDOW_COL2_X, WINDOW_ROW1_Y + 40, 0, DRAWTEXT_FLAG_DEFAULT | sys_color_table.color_work_text);
    draw_text_sys("Selection & Ctrl+C available.", WINDOW_COL2_X, WINDOW_ROW1_Y + 60, 0, DRAWTEXT_FLAG_DEFAULT | sys_color_table.color_work_text);

    ted_draw(tedit1);

    if (img1 != NULL) {
        //printf("drawing...\n");
        img_draw(img1, 10, WINDOW_ROW1_Y, IMG1_DISPLAY_W, IMG1_DISPLAY_H , 0, 0);
    } else {
        draw_bar(10, WINDOW_ROW1_Y, IMG1_DISPLAY_W, IMG1_DISPLAY_H, COL_GREY);
    }

    end_draw();
}

void tedit1_print(const char *text, int text_len, int do_newline) {
    if (text_len == -1) { text_len = strlen(text); }
    ted_text_add(tedit1, (char *)text, text_len, 1);
    if (do_newline != 0 ) { ted_text_add(tedit1, "\r", 1, 1); } 
}

void create_components() {
    tedit1 = kolibri_new_editor(X_W(WINDOW_COL2_X, TEDIT1_W), Y_H(WINDOW_ROW1_Y + IMG1_DISPLAY_H - TEDIT1_H - 16, TEDIT1_H), 0/*0x11*/, TEDIT1_MAXCHAR, &tedit1_lock);  // 0x11 font 8x16 sized x2, 0 - default (8x16)
    tedit1_lock = tedit1;
    tedit1->mode_invis = 0; /* dont show invisible characters */

    op_dialog1 = kolibri_new_open_dialog(OPEN, 10, 10, 420, 320);
    op_dialog1->dir_default_path = cur_dir_path;
    op_dialog1->draw_window = redraw_window;
    OpenDialog_init(op_dialog1);
}

void recognize_qr() {
    puts("Starting QUIRC...\n");
    struct quirc *qr;

    qr = quirc_new();
    if (!qr) {
        puts("Failed to allocate memory");
        exit(-1);
    }

    if (quirc_resize(qr, img1_grayscale->Width, img1_grayscale->Height) < 0) {
        puts("Failed to allocate video memory");
        exit(-1);
    }

    void *gsbuf = img1_grayscale->Data;
    uint32_t gsbuf_size = img1_grayscale->Width*img1_grayscale->Height;

    uint8_t *buffer;
    int w, h;
    puts("quirc_begin()...\n");
    buffer = quirc_begin(qr, &w, &h);
    //printf("buffer = %x  qr = %x\n", buffer, qr);
    //printf("w = %d   h = %d\n", w, h);
    memcpy(buffer, gsbuf, gsbuf_size);
    puts("quirc_end()...\n");
    quirc_end(qr);

    ted_clear(tedit1, 1); /* cleaning output text area */

    int num_codes, i;
    puts("quirc_count()...\n");
    num_codes = quirc_count(qr);
    printf("    NUM_CODES = %d\n", num_codes);
    if (num_codes == 0) {
        tedit1_print("DECODE FAILED: NO CODES FOUND", -1, 1);
    }
    for (i = 0; i < num_codes; i++) {
        struct quirc_code code;
        struct quirc_data data;
        quirc_decode_error_t err;

        quirc_extract(qr, i, &code);

        // decoding stage
        err = quirc_decode(&code, &data);
        if (err) {
            //printf("    DECODE FAILED: %s\n", quirc_strerror(err));
            tedit1_print("DECODE FAILED: ", -1, 0);
            tedit1_print(quirc_strerror(err), -1, 1);
        } else {
            //printf("\n    RECOGNIZED DATA: %s\n", data.payload);
            tedit1_print("RECOGNIZED: ", -1, 0);
            tedit1_print(data.payload, data.payload_len, 1);
        }
    }
    
    puts("\nquirc_destroy()...\n");
    quirc_destroy(qr);
}

void on_btn_open() {
    //ted_text_add(tedit1, "Hello world!\r", strlen("Hello world!\r"), 1);
    op_dialog1->mode = OPEN;
    OpenDialog_start(op_dialog1);
    if (op_dialog1->status != 2 && op_dialog1->status != 0) {// fail or cancel
        img1_path = op_dialog1->openfile_path;
        //ted_text_add(tedit1, img1_path, strlen(img1_path), 1);
        //ted_text_add(tedit1, "\r", 1, 1); // newline 
    } else {
        return;
    }
    Image *oldimg;
    if (img1 != NULL) { img_destroy(img1); }
    if (img1_grayscale != NULL) { img_destroy(img1_grayscale); }
    uint32_t file_size;
    void *file_data = load_img(img1_path, &file_size); // Get RAW data and size 
    img1 = img_decode(file_data, file_size, 0); // Decode RAW data to Image data
    img1_grayscale = img_decode(file_data, file_size, 0); // Decode RAW data to Image data
    free(file_data); //
    printf("original: image->Width = %d, Image->Height = %d,\n original image type = %d\n\n", img1->Width, img1->Height, img1->Type);
    if (img1->Type != IMAGE_BPP24) { 
        oldimg = img1; //
        img1 = img_convert(img1, NULL, IMAGE_BPP24, 0, 0); // Convert image to RGB 24
        img_destroy(oldimg); //
        if (!img1) {
            printf("Сonvert img1 to BPP24 error!: \n");  
            exit(-1);
        }
    }
    if (img1_grayscale->Type != IMAGE_BPP24) { 
        oldimg = img1_grayscale; //
        img1_grayscale = img_convert(img1_grayscale, NULL, IMAGE_BPP24, 0, 0); // Convert image to RGB
        img_destroy(oldimg); //
        if (!img1_grayscale) {
            printf("Сonvert img1_grayscale to BPP24 error!: \n");  
            exit(-1);
        }
    }
    if (img1_grayscale->Type != IMAGE_BPP8g) { 
        oldimg = img1_grayscale; //
        img1_grayscale = img_convert(img1_grayscale, NULL, IMAGE_BPP8g, 0, 0); // Convert image to grayscale
        img_destroy(oldimg); //
        if (!img1_grayscale) {
            printf("Сonvert img1_grayscale to BPP8g error!: \n");  
            exit(-1);
        }
    }
    oldimg = img1; //
    img1 = img_scale(img1, 0, 0, img1->Width, img1->Height, NULL, LIBIMG_SCALE_STRETCH , LIBIMG_INTER_BILINEAR, IMG1_DISPLAY_W, IMG1_DISPLAY_H);
    img_destroy(oldimg); //

    recognize_qr();
}

int main(int argc, char *argv[]) {
    int gui_event; /* variable for storing event */
    uint32_t pressed_button = 0; /* code of button pressed in window */
    unsigned int keyval; /* for saving pressed key */
    oskey_t key;

    ((void)argc);
    strcpy(cur_dir_path, argv[0] + 2); char *pc = strrchr(cur_dir_path, '/'); if (pc) { *pc = '\0'; }
    printf("cur_dir_path = %s\n", cur_dir_path);
 
    kolibri_boxlib_init();
    kolibri_proclib_init();  // opensave && color dialogs
    kolibri_libimg_init();

    set_wanted_events_mask(0xC0000027);
    set_os_keyb_mode(1); // scan code mode needed for editor    
    
    kolibri_get_system_colors(&sys_color_table); // Get system colors theme

    create_components();

    do {
        gui_event = get_os_event();
        switch(gui_event) {
        case KOLIBRI_EVENT_NONE:
            break;
        case KOLIBRI_EVENT_REDRAW:
            redraw_window();
            break;
        case KOLIBRI_EVENT_MOUSE:
            ted_mouse(tedit1);
            break;        
        case KOLIBRI_EVENT_KEY:
            key = get_key();
            if (tedit1_lock == tedit1) { editor_key(tedit1, key); }
            break;
        case KOLIBRI_EVENT_BUTTON:
            pressed_button = get_os_button();
            switch (pressed_button)
            {
                case MYBTN_OPEN:
                {
                    on_btn_open();
                    redraw_window();
                    break;
                }

                /*case MYBTN_CLEAR:
                    ted_clear(tedit1, 1);
                    redraw_window();
                    break; */
 
                case MYBTN_QUIT:
                    exit(0);
                    break;
            }
        }
    } while (1);

    exit(0);
}
