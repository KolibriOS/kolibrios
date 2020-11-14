// BOXLIB example (scrollbar, progressbar)
// ! without kolibri_gui !
// Writed by maxcodehack
// TCC version is in /programs/develop/ktcc/trunk/samples
#include <kos32sys.h>
#include <stdlib.h>

/// BOXLIB
// Modified from C_Layer
// C_Layer variant I don't like
extern int kolibri_boxlib_init(void);
typedef struct __attribute__ ((__packed__)) {
	uint16_t xsize;
    uint16_t xpos;
    uint16_t ysize;
    uint16_t ypos;
    uint32_t btn_height;
    uint32_t type;
    uint32_t max_area;
    uint32_t cur_area;
    uint32_t position;
    uint32_t back_color;
    uint32_t front_color;
    uint32_t line_color;
    uint32_t redraw;
    uint16_t delta;
    uint16_t delta2;
    uint16_t r_size_x;
    uint16_t r_start_x;
    uint16_t r_size_y;
    uint16_t r_start_y;
    uint32_t m_pos;
    uint32_t m_pos2;
    uint32_t m_keys;
    uint32_t run_size;
    uint32_t position2;
    uint32_t work_size;
    uint32_t all_redraw;
    uint32_t ar_offset;
} scrollbar;

extern void (*scrollbar_h_draw)(scrollbar*) __attribute__((__stdcall__));
extern void (*scrollbar_h_mouse)(scrollbar*) __attribute__((__stdcall__));
extern void (*scrollbar_v_draw)(scrollbar*) __attribute__((__stdcall__));
extern void (*scrollbar_v_mouse)(scrollbar*) __attribute__((__stdcall__));

typedef struct {
	unsigned int value;
    unsigned int left;
    unsigned int top;
    unsigned int width;
    unsigned int height;
    unsigned int style;
    unsigned int min;
    unsigned int max;
    unsigned int back_color;
    unsigned int progress_color;
    unsigned int frame_color;
} progressbar;

extern void (*progressbar_draw)(progressbar *) __attribute__((__stdcall__));
extern void (*progressbar_progress)(progressbar *) __attribute__((__stdcall__));
/// BOXLIB


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
int win_bg_color = 0x858585;
scrollbar scroll = {15, WIN_W - 26, WIN_H - 29, 0, 0, 2, 215, 15, 0,0x707070,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
progressbar pg = {0, 10, 10, 270, 35, 1, 0, 200, 0xB4B4B4, 0x2728FF, 0xA9A9A9};

void draw_window(){
        BeginDraw(); 
        DrawWindow(215,100,WIN_W,WIN_H,title,win_bg_color,0x34);
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
	
	set_wanted_events_mask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
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
