#ifndef KOLIBRI_COLORIALOG_H
#define KOLIBRI_COLORIALOG_H

char cd_com_area_name[]    = "FFFFFFFF_color_dialog";
char cd_start_path[]       = "/rd/1/colrdial";

typedef struct __attribute__ ((__packed__)) {
    unsigned int type;
    unsigned int procinfo;
    unsigned int com_area_name;
    unsigned int com_area;
    unsigned int start_path;
    unsigned int draw_window;
    unsigned int status;
    unsigned short x_size;
    unsigned short x_start;
    unsigned short y_size;
    unsigned short y_start;
    unsigned int color_type;
    unsigned int color;
}color_dialog;

void cd_fake_on_redraw(void) {}

struct open_dialog* kolibri_new_color_dialog(unsigned int type, unsigned short tlx, unsigned short tly, unsigned short x_size, unsigned short y_size)
{
    color_dialog *new_colordialog = (color_dialog *)malloc(sizeof(color_dialog));
    char *proc_info = (char *)calloc(1024, sizeof(char));

	new_colordialog -> type = type;
	new_colordialog -> procinfo = proc_info;
	new_colordialog -> com_area_name = &cd_com_area_name;
	new_colordialog -> com_area = 0;
	new_colordialog -> start_path = &cd_start_path;
	new_colordialog -> draw_window = &cd_fake_on_redraw;
	new_colordialog -> status = 0;
	new_colordialog -> x_size = x_size;
	new_colordialog -> x_start = tlx;
	new_colordialog -> y_size = y_size;
	new_colordialog -> y_start = tly;
	new_colordialog -> color_type = 0;
	new_colordialog -> color = 0;
	return new_colordialog;
}

extern void kolibri_proclib_init() __attribute__((__stdcall__));
extern void (*ColorDialog_init)(color_dialog *) __attribute__((__stdcall__));
extern void (*ColorDialog_start)(color_dialog *) __attribute__((__stdcall__));
#endif /* KOLIBRI_COLORDIALOG_H */
