/* Copyright (C) 2021- Rustem Gimadutdinov (rgimad), GPLv2 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ksys.h>
#include <clayer/boxlib.h>

#define DATA(type, addr, offset) *((type*)((uint8_t*)addr+offset))
#define X_W(X, W) ((X<<16)+W)
#define Y_H X_W

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 180
#define EDIT_BOX_PASSWORD_LEN_MAXLEN 16
#define EDIT_BOX_PASSWORD_GENERATED_MAXLEN 256
#define FONT_SIZE_DEFAULT 0x10000000
#define CHECKBOX_ENABLED 2

const char WINDOW_TITLE[] = "PasswordGen v0.2";

ksys_colors_table_t sys_color_table;
const color_t DRAWTEXT_FLAG_DEFAULT = 0x90000000;

edit_box *edit_box_password_len;
char edit_box_password_len_buf[EDIT_BOX_PASSWORD_LEN_MAXLEN];

edit_box *edit_box_password_generated;
char edit_box_password_generated_buf[EDIT_BOX_PASSWORD_GENERATED_MAXLEN];

check_box *check_box_az;
check_box *check_box_AZ;
check_box *check_box_09;
check_box *check_box_spec_char;

enum MYCOLORS {
    GREEN = 0x067D06,
    BLUE  = 0x0000FF,
    RED   = 0xFF0000,
    BLACK = 0x000000,
    WHITE = 0xFFFFFF,
    GREY  = 0x919191
};

enum BUTTONS {
    BTN_QUIT = 1,
    BTN_GENERATE = 10,
    BTN_COPY = 20
    //
};

const char char_set_az[] = "abcdefghijklmnopqrstuvwxyz";
const char char_set_AZ[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const char char_set_09[] = "0123456789";
const char char_set_spec_char[] = "!@#$%^&*_";

char resulting_char_set[75];
char password_generated[EDIT_BOX_PASSWORD_GENERATED_MAXLEN];

/* ---------------------------------------------------------- */

void generate_password_internal(char *dst, int length, int flag_az, int flag_AZ, int flag_09, int flag_spec_char) {
    int i, resulting_char_set_len;
    resulting_char_set[0] = '\0';
    if (flag_az == 0 && flag_AZ == 0 && flag_09 == 0 && flag_spec_char == 0) { return; }
    if (flag_az) { strcat(resulting_char_set, char_set_az); }
    if (flag_AZ) { strcat(resulting_char_set, char_set_AZ); }
    if (flag_09) { strcat(resulting_char_set, char_set_09); }
    if (flag_spec_char) { strcat(resulting_char_set, char_set_spec_char); }
    resulting_char_set_len = strlen(resulting_char_set);
    //debug_printf("resulting_char_set = %s\n", resulting_char_set
    for (i = 0; i < length; i++) {
        dst[i] = resulting_char_set[rand() % resulting_char_set_len];
    }
    dst[length] = '\0';
}

void notify_show(char *text) {
   _ksys_exec("/sys/@notify", text);
}

void* safe_malloc(size_t size) {
    void *p = malloc(size);
    if (p == NULL) {
       notify_show("'Memory allocation error!' -E");
       exit(0);
    } else {
        return p;
    }
}

void copy_to_clipboard(char *text) {
    char *temp_buffer = safe_malloc(EDIT_BOX_PASSWORD_GENERATED_MAXLEN + 12);
    memset(temp_buffer, 0, EDIT_BOX_PASSWORD_GENERATED_MAXLEN);
    DATA(char,temp_buffer,4) = KSYS_CLIP_TEXT;  /* TEXT */
    DATA(char,temp_buffer,8) = KSYS_CLIP_CP866; /* CP866 */
    strncpy(temp_buffer+12, text, EDIT_BOX_PASSWORD_GENERATED_MAXLEN - 1);
    _ksys_clip_set(strlen(text) + 12, temp_buffer);
    notify_show("'Copied to clipboard!' -I");
    free(temp_buffer);
}

edit_box* create_edit_box(unsigned int width, unsigned int left, unsigned int top,
    unsigned int color, unsigned int shift_color, unsigned int focus_border_color,
    unsigned int blur_border_color, unsigned int text_color, unsigned int max,
    char *text, void *mouse_variable, unsigned int flags)
{
    edit_box *eb = (edit_box*)safe_malloc(sizeof(edit_box));
    memset(eb, 0, sizeof(edit_box));
    eb->width = width;
    eb->left = left;
    eb->top = top;
    eb->color = color;
    eb->shift_color = shift_color;
    eb->focus_border_color = focus_border_color;
    eb->blur_border_color = blur_border_color;
    eb->text_color = text_color;
    eb->max = max;
    eb->text = text;
    eb->mouse_variable = mouse_variable;
    eb->flags = flags;
    return eb;   
}

check_box* create_check_box(unsigned int left_s, unsigned int top_s, unsigned int ch_text_margin,
    unsigned int color, unsigned int border_color, unsigned int text_color, char *text, unsigned int flags)
{
    check_box *cb = (check_box*)safe_malloc(sizeof(check_box));
    memset(cb, 0, sizeof(check_box));
    cb->left_s = left_s;
    cb->top_s = top_s;
    cb->ch_text_margin = ch_text_margin;
    cb->color = color;
    cb->border_color = border_color;
    cb->text_color = text_color;
    cb->text = text;
    cb->flags = flags;
    return cb;
}
/* ---------------------------------------------------------- */

void redraw_window() {
    ksys_pos_t win_pos = _ksys_get_mouse_pos(KSYS_MOUSE_SCREEN_POS); 
    _ksys_start_draw();
    _ksys_create_window(win_pos.x, win_pos.y, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, sys_color_table.work_area, 0x14);

    edit_box_draw(edit_box_password_len);

    _ksys_draw_text("Password length: ", 15, 34 + 5, 0, DRAWTEXT_FLAG_DEFAULT | sys_color_table.work_text);
    _ksys_draw_text("Characters: ", 15, 68, 0, DRAWTEXT_FLAG_DEFAULT | sys_color_table.work_text);
    _ksys_draw_text("Generated password: ", 15, 102 + 5, 0, DRAWTEXT_FLAG_DEFAULT | sys_color_table.work_text);

    check_box_draw2(check_box_az);
    check_box_draw2(check_box_AZ);
    check_box_draw2(check_box_09);
    check_box_draw2(check_box_spec_char);

    edit_box_draw(edit_box_password_generated);

    _ksys_define_button(170, 136, 100, 30, BTN_GENERATE, sys_color_table.work_button);
    _ksys_draw_text("Generate!", 182, 136 + 7, 0, DRAWTEXT_FLAG_DEFAULT | sys_color_table.work_button_text);

    _ksys_define_button(370-60, 136, 60, 30, BTN_COPY, sys_color_table.work_button);
    _ksys_draw_text("Copy", 370 - 60 + 15, 136 + 7, 0, DRAWTEXT_FLAG_DEFAULT | sys_color_table.work_button_text);

    _ksys_end_draw();
}

/* create and initialize components */ 
void create_components() {
    edit_box_password_len = create_edit_box(70, 150, 34, WHITE, sys_color_table.work_button, 0, GREY, FONT_SIZE_DEFAULT, EDIT_BOX_PASSWORD_LEN_MAXLEN - 2, edit_box_password_len_buf, NULL, 0/*ed_focus*/);
    edit_box_set_text(edit_box_password_len, "10");

    check_box_az = create_check_box(X_W(110, 15), Y_H(68,15), 10, WHITE, BLUE, BLACK | FONT_SIZE_DEFAULT, "a-z", CHECKBOX_ENABLED);
    init_checkbox2(check_box_az);

    check_box_AZ = create_check_box(X_W(170, 15), Y_H(68,15), 10, WHITE, BLUE, BLACK | FONT_SIZE_DEFAULT, "A-Z", CHECKBOX_ENABLED);
    init_checkbox2(check_box_AZ);

    check_box_09 = create_check_box(X_W(230, 15), Y_H(68,15), 10, WHITE, BLUE, BLACK | FONT_SIZE_DEFAULT, "0-9", CHECKBOX_ENABLED);
    init_checkbox2(check_box_09);

    check_box_spec_char = create_check_box(X_W(290, 15), Y_H(68,15), 10, WHITE, BLUE, BLACK | FONT_SIZE_DEFAULT, "!@#$%^&*_", 0);
    init_checkbox2(check_box_spec_char);

    edit_box_password_generated = create_edit_box(200, 170, 102, WHITE, sys_color_table.work_button, 0, GREY, FONT_SIZE_DEFAULT, EDIT_BOX_PASSWORD_GENERATED_MAXLEN - 2, edit_box_password_generated_buf, NULL, 0/*ed_focus*/);
}

void generate_and_show_password() {
    int f_az, f_AZ, f_09, f_spec, psw_len = atoi(edit_box_password_len_buf);
    if (psw_len == 0 || psw_len >= EDIT_BOX_PASSWORD_GENERATED_MAXLEN - 2) {
        notify_show("'Incorrect password length' -E");
    } else {
        f_az = check_box_az->flags & CHECKBOX_ENABLED;
        f_AZ = check_box_AZ->flags & CHECKBOX_ENABLED;
        f_09 = check_box_09->flags & CHECKBOX_ENABLED;
        f_spec = check_box_spec_char->flags & CHECKBOX_ENABLED;
        if (f_az == 0 && f_AZ == 0 && f_09 == 0 && f_spec == 0) {
            notify_show("'You must choose at least one character set' -E");
            return;
        }
        generate_password_internal(password_generated, psw_len, f_az, f_AZ, f_09, f_spec);
        edit_box_set_text(edit_box_password_generated, password_generated);
    }
}

int main(int argc, const char *argv[]) {
    int gui_event; /* variable for storing event */
    uint32_t pressed_button = 0; /* code of button pressed in window */
    ksys_oskey_t key; /* for saving pressed key */

    srand(time(0)*2/3); /* seeding the pseudo random number generator*/
    _ksys_get_system_colors(&sys_color_table);
    _ksys_set_event_mask(0xC0000027);
    create_components(); /* create and init some visual components */

    do
    {
        gui_event = _ksys_get_event();
        switch(gui_event)
        {
        case KSYS_EVENT_NONE:
            break;
        case KSYS_EVENT_REDRAW:
            redraw_window();
            break;
        case KSYS_EVENT_MOUSE:
            edit_box_mouse(edit_box_password_len);
            check_box_mouse2(check_box_az);
            check_box_mouse2(check_box_AZ);
            check_box_mouse2(check_box_09);
            check_box_mouse2(check_box_spec_char);
            edit_box_mouse(edit_box_password_generated);
            break;        
        case KSYS_EVENT_KEY:
            key = _ksys_get_key();
            edit_box_key_safe(edit_box_password_len, key);
            edit_box_key_safe(edit_box_password_generated, key);
            break;
        case KSYS_EVENT_BUTTON: 
            pressed_button = _ksys_get_button(); 
            switch (pressed_button) 
            {
                case BTN_GENERATE:
                    generate_and_show_password();
                    redraw_window();
                    break;

                case BTN_COPY:
                    copy_to_clipboard(edit_box_password_generated_buf);
                    redraw_window();
                    break;

                case BTN_QUIT:
                    return 0;
                    break;
            }
        }
    } while(1);
    return 0;
}
