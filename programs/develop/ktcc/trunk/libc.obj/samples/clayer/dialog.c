#include <clayer/dialog.h>
#include <stdio.h>
#include <sys/ksys.h>

int main()
{
    open_dialog* dlg_open = kolibri_new_open_dialog(OPEN, 10, 10, 420, 320); // create opendialog struct
    OpenDialog_init(dlg_open);                                               // Initializing an open dialog box.
    OpenDialog_start(dlg_open);                                              // Show open dialog box

    color_dialog* color_select = kolibri_new_color_dialog(SELECT, 10, 10, 420, 320); // create colordialog struct
    ColorDialog_init(color_select);                                                  // Initializing an color dialog box.
    ColorDialog_start(color_select);                                                 // Show color dialog

    if (dlg_open->status == SUCCESS) {
        printf("File selected '%s'\n", dlg_open->openfile_path);
    } else {
        puts("No file selected!");
    }

    if (color_select->status == SUCCESS) {
        printf("Color selected: #%06X\n", color_select->color);
        rgb_t color_rgb = (rgb_t)color_select->color;
        printf("Red:%d Green:%d Blue:%d", color_rgb.red, color_rgb.green, color_rgb.blue);
    } else {
        puts("No color selected!");
    }
    free(dlg_open);
    free(color_select);
    return 0;
}
