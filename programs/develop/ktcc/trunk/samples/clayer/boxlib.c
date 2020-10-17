#include <kos32sys1.h>
#include <stdlib.h>
#include <clayer/boxlib.h>

#define evReDraw  1
#define evKey     2
#define evButton  3
#define evExit    4
#define evDesktop 5
#define evMouse   6
#define evIPC     7
#define evNetwork 8
#define evDebug   9

#define WIN_W 640
#define WIN_H 563

char* title = "Boxlib example";
int win_bg_color = 0x858585;
scrollbar scroll = {15, WIN_W - 26, WIN_H - 29, 0, 0, 2, 115, 15, 0,0x707070,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

void draw_window(){
        begin_draw(); 
        sys_create_window(215,100,WIN_W,WIN_H,title,win_bg_color,0x34);
        scrollbar_v_draw(&scroll);
        end_draw();
}

//// EVENTMASK
#define EVM_REDRAW        1
#define EVM_KEY           2
#define EVM_BUTTON        4
#define EVM_EXIT          8
#define EVM_BACKGROUND    16
#define EVM_MOUSE         32
#define EVM_IPC           64
#define EVM_STACK         128
#define EVM_DEBUG         256
#define EVM_STACK2        512
#define EVM_MOUSE_FILTER  0x80000000
#define EVM_CURSOR_FILTER 0x40000000
//// EVENTMASK


int main()
{
	kolibri_boxlib_init();
	
	set_event_mask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	while(1)
	{
		switch(get_os_event())
		{
			case evButton:
				if (get_os_button() == 1) exit(0);
				break;
		  
			case evKey:
				get_key();
				break;
			 
			case evReDraw:
				draw_window();
				break;
			case evMouse:
				scrollbar_v_mouse(&scroll);
				break;
		}
	}
}
