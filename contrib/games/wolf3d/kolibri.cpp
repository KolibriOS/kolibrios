#include <stdlib.h>
#include <sys/stat.h>
#include "../../kolibri-libc/source/include/ksys.h"

void kolibri_set_win_center()
{
    ksys_proc_table_t *info = (ksys_proc_table_t*)malloc(sizeof(ksys_proc_table_t));
    _ksys_process_info(info, -1);

    ksys_pos_t screen_size= _ksys_screen_size();
    int new_x = screen_size.x/2-info->winx_size/2;
    int new_y = screen_size.y/2-info->winy_size/2;
    _ksys_change_window(new_x, new_y, -1, -1); 
    free(info);
}

int mkdir(const char * path, unsigned)
{
   return _ksys_mkdir(path);
}

