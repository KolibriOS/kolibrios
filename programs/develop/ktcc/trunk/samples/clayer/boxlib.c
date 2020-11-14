// BOXLIB EXAMPLE (scrollbar, progressbar)
// Writed by maxcodehack
// GCC version is in /contrib/C_Layer/EXAMPLE/boxlib
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

uint32_t wheels;
char* title = "Boxlib example";
scrollbar scroll = {15, WIN_W - 26, WIN_H - 29, 0, 0, 2, 215, 15, 0,0x707070,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
progressbar pg = {0, 10, 10, 270, 35, 1, 0, 200, 0xB4B4B4, 0x2728FF, 0xA9A9A9};

/*
// System colors
struct kolibri_system_colors sys_color;
*/

void draw_window(){
        BeginDraw(); 
        DrawWindow(215,100,WIN_W,WIN_H,title, /* sys_color.work_area */ 0x858585, 0x34);
        scrollbar_v_draw(&scroll);
        progressbar_draw(&pg);
        EndDraw();
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
	/*
	get_system_colors(&sys_color);
	*/
	set_event_mask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	while(1)
	{
		switch(GetOsEvent())
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
				
				// Wheel scrolling
				// Quite unstable
				/*
				int scroll_strong = 40;
				wheels = GetMouseWheels();
				if(wheels & 0xFFFF)
				{
					if((short)wheels > 0 && scroll.position < scroll.max_area - scroll_strong)
						scroll.position += scroll_strong;
					else if((short)wheels < 0 && scroll.position > 0)
						scroll.position -= scroll_strong;
					
					scrollbar_v_draw(&scroll);
				}
				*/
				pg.value = scroll.position;
				progressbar_draw(&pg);
				break;
		}
	}
}
