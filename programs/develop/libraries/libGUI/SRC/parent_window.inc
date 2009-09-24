/*
        create  parent of window
*/

#define PARENT_WINDOW_DEFAULT_SIZEX             320
#define PARENT_WINDOW_DEFAULT_SIZEY             200

#define PARENT_WINDOW_BORDER_WIDTH              5;

void gui_get_screen_parameters(void)
{
        int     value;
        
        value=(int)gui_ksys_get_screen_bits_per_pixel();
        screen.bits_per_pixel=(char)value;
        screen.bytes_per_pixel=screen.bits_per_pixel >> 3;
        
        screen.skin_height=gui_ksys_get_skin_height();

        screen.x=PARENT_WINDOW_BORDER_WIDTH;
        screen.y=screen.skin_height;
        
        value=gui_ksys_get_screen_size();
        screen.display_size_y=value & 0xffff;
        screen.display_size_y=value >> 16;
}

void    gui_draw_window(parent_t *window)
{
        DWORD   flag;
        
        flag=3;
        flag=flag<<24;
        flag +=0xaabbcc;

        gui_ksys_begin_draw_window();
        gui_ksys_draw_window(window->ctrl_x,window->ctrl_y,window->ctrl_sizex,window->ctrl_sizey,flag);
        gui_ksys_finish_draw_window();
}

//---------------------------------------------------------------------------------
//                              create window parent
//---------------------------------------------------------------------------------
void* CreateWindow(void)
{
        struct HEADERPARENT *WindowParent;

        WindowParent=malloc(sizeof(parent_t));
        WindowParent->message=malloc(sizeof(gui_message_t));
        WindowParent->control_for_callback_function=malloc(sizeof(DWORD)*MAX_CALLBACKS);
        WindowParent->callback_for_control_callback=malloc(sizeof(DWORD)*MAX_CALLBACKS);

        WindowParent->main_parent=(DWORD*)WindowParent;
        WindowParent->global_active_control_for_keys=(DWORD*)NULL;

        WindowParent->control_for_callback_function[0]=(DWORD*)NULL;
        WindowParent->number_callbacks=0;

        WindowParent->child_bk=(DWORD*)NULL;
        WindowParent->active_control_for_keys=(DWORD*)NULL;
        WindowParent->active_control_for_mouse=(DWORD*)NULL;
        WindowParent->ctrl_x=0x0;
        WindowParent->ctrl_y=0x0;
        WindowParent->ctrl_sizex=PARENT_WINDOW_DEFAULT_SIZEX;
        WindowParent->ctrl_sizey=PARENT_WINDOW_DEFAULT_SIZEY;
        WindowParent->callback=(DWORD*)NULL;//no callbacks yet
        WindowParent->timer=(DWORD*)NULL;//no timers yet
        
        WindowParent->flags=0;
        WindowParent->flags=WindowParent->flags | FLAG_SHOW_CONTROL;
        WindowParent->flags=WindowParent->flags | FLAG_FOCUSE_INPUT_SUPPOROTE;

        WindowParent->number_timers_for_controls=0;
        WindowParent->timer_bk=(DWORD*)NULL;
        WindowParent->timer_fd=(DWORD*)NULL;

        WindowParent->callback=(DWORD*)NULL;
        WindowParent->calev_bk=(DWORD*)NULL;
        WindowParent->calev_fd=(DWORD*)NULL;

        WindowParent->IDL_func=(DWORD*)NULL;

//---------------------------------------------------------------------------------
//---------------------------platform depended part of code------------------------
//---------------------------------------------------------------------------------
        //create and initialize screen buffer
        gui_get_screen_parameters();
        //by default draw output to the screen
        screen.draw_output=DRAW_OUTPUT_SCREEN;
        //calculate size of client's arrea
        screen.size_x=WindowParent->ctrl_sizex-9;
        screen.size_y=WindowParent->ctrl_sizey-screen.skin_height-4;
//----------------------------------------------------------------------------------
        ID=0;
#ifdef DEBUG
        printf("\ncreated parent window %d",(DWORD)WindowParent);
#endif
        return(WindowParent);
}

//---------------------------------------------------------------------------------
//                              create window parent
//---------------------------------------------------------------------------------
void SetWindowSizeRequest(parent_t *WindowParent,int size_x,int size_y)
{
        static int      x,y,sizex,sizey;
//---------------------------------------------------------------------------------
//---------------------------platform depended part of code------------------------
//---------------------------------------------------------------------------------
        x=WindowParent->ctrl_x;
        y=WindowParent->ctrl_y;
        sizex=size_x;
        sizey=size_y;
        gui_ksys_set_position_and_size_window(x,y,sizex,sizey);
//---------------------------------------------------------------------------------
        WindowParent->ctrl_sizex=sizex;
        WindowParent->ctrl_sizey=sizey;

        screen.size_x=WindowParent->ctrl_sizex-9;
        screen.size_y=WindowParent->ctrl_sizey-screen.skin_height-4;
#ifdef DEBUG
        printf("\nwindow resized new sizex=%d sizey=%d",
                WindowParent->ctrl_sizex,
                WindowParent->ctrl_sizey);
#endif
 
}

void GetNewWindowSizePos(parent_t *WindowParent)
{
        static  process_table_t procinfo;
        
        gui_ksys_get_current_process_information(&procinfo);
        
        WindowParent->ctrl_x=(DWORD)procinfo.winx_start;
        WindowParent->ctrl_y=(DWORD)procinfo.winy_start;
        WindowParent->ctrl_sizex=(DWORD)procinfo.winx_size;
        WindowParent->ctrl_sizey=(DWORD)procinfo.winy_size;

        //get screen parameters again
        gui_get_screen_parameters();
}
