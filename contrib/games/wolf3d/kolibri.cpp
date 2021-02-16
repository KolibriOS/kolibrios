#include <SDL.h>
#include <kos32sys.h>
#include <stdlib.h>
 
void kolibri_set_win_center()
{
    struct proc_info *info = (struct proc_info*)malloc(sizeof(struct proc_info));
    get_proc_info((char*)info);
    
    pos_t screen_size= max_screen_size();
    int new_x = screen_size.x/2-info->width/2;
    int new_y = screen_size.y/2-info->height/2;
    sys_change_window(new_x,new_y, -1, -1); 
    free(info);
}
