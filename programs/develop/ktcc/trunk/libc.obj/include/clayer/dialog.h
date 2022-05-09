#ifndef KOLIBRI_DIALOG_H
#define KOLIBRI_DIALOG_H

#include <stdlib.h>

#define NOT_SUCCESS 0
#define SUCCESS     1

char sz_com_area_name[] = "FFFFFFFF_open_dialog";
char sz_dir_default_path[] = "/sys";
char sz_start_path[] = "/sys/File managers/opendial";

char cd_com_area_name[] = "FFFFFFFF_color_dialog";
char cd_start_path[] = "/sys/colrdial";

enum open_dialog_mode {
    OPEN,
    SAVE,
    SELECT
};

typedef struct {
    unsigned int size;
    unsigned char end;
} od_filter __attribute__((__packed__));

typedef struct {
    unsigned int mode;
    char* procinfo;
    char* com_area_name;
    unsigned int com_area;
    char* opendir_path;
    char* dir_default_path;
    char* start_path;
    void (*draw_window)();
    unsigned int status;
    char* openfile_path;
    char* filename_area;
    od_filter* filter_area;
    unsigned short x_size;
    unsigned short x_start;
    unsigned short y_size;
    unsigned short y_start;
} open_dialog __attribute__((__packed__));

typedef struct {
    unsigned int type;
    char* procinfo;
    char* com_area_name;
    unsigned int com_area;
    char* start_path;
    void (*draw_window)(void);
    unsigned int status;
    unsigned short x_size;
    unsigned short x_start;
    unsigned short y_size;
    unsigned short y_start;
    unsigned int color_type;
    unsigned int color;
} color_dialog __attribute__((__packed__));

void fake_on_redraw(void) { }

open_dialog* kolibri_new_open_dialog(unsigned int mode, unsigned short tlx, unsigned short tly, unsigned short x_size, unsigned short y_size)
{
    open_dialog* new_opendialog = (open_dialog*)malloc(sizeof(open_dialog));
    od_filter* new_od_filter = (od_filter*)malloc(sizeof(od_filter));
    char* plugin_path = (char*)calloc(4096, sizeof(char));
    char* openfile_path = (char*)calloc(4096, sizeof(char));
    char* proc_info = (char*)calloc(1024, sizeof(char));
    char* filename_area = (char*)calloc(256, sizeof(char));

    new_od_filter->size = 0;
    new_od_filter->end = 0;

    new_opendialog->mode = mode;
    new_opendialog->procinfo = proc_info;
    new_opendialog->com_area_name = sz_com_area_name;
    new_opendialog->com_area = 0;
    new_opendialog->opendir_path = plugin_path;
    new_opendialog->dir_default_path = sz_dir_default_path;
    new_opendialog->start_path = sz_start_path;
    new_opendialog->draw_window = &fake_on_redraw;
    new_opendialog->status = 0;
    new_opendialog->openfile_path = openfile_path;
    new_opendialog->filename_area = filename_area;
    new_opendialog->filter_area = new_od_filter;
    new_opendialog->x_size = x_size;
    new_opendialog->x_start = tlx;
    new_opendialog->y_size = y_size;
    new_opendialog->y_start = tly;
    return new_opendialog;
}

void cd_fake_on_redraw(void) { }

color_dialog* kolibri_new_color_dialog(unsigned int type, unsigned short tlx, unsigned short tly, unsigned short x_size, unsigned short y_size)
{
    color_dialog* new_colordialog = (color_dialog*)malloc(sizeof(color_dialog));
    char* proc_info = (char*)calloc(1024, sizeof(char));

    new_colordialog->type = type;
    new_colordialog->procinfo = proc_info;
    new_colordialog->com_area_name = cd_com_area_name;
    new_colordialog->com_area = 0;
    new_colordialog->start_path = cd_start_path;
    new_colordialog->draw_window = &cd_fake_on_redraw;
    new_colordialog->status = 0;
    new_colordialog->x_size = x_size;
    new_colordialog->x_start = tlx;
    new_colordialog->y_size = y_size;
    new_colordialog->y_start = tly;
    new_colordialog->color_type = 0;
    new_colordialog->color = 0;
    return new_colordialog;
}

DLLAPI void __stdcall OpenDialog_init(open_dialog*);
DLLAPI void __stdcall OpenDialog_start(open_dialog*);

DLLAPI void __stdcall ColorDialog_init(color_dialog*);
DLLAPI void __stdcall ColorDialog_start(color_dialog*);

#endif
