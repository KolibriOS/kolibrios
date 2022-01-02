// BOXLIB EXAMPLE (scrollbar, progressbar, editbox and checkbox)
// Writed by maxcodehack and superturbocat2001

#include <kos32sys1.h>
#include <stdlib.h>
#include <string.h>
#include <clayer/boxlib.h>
#include <stdio.h>

#define WIN_W 640
#define WIN_H 563

#define ED_BUFF_LEN 50
#define TEXT_SIZE 0x10000000
#define SCROLL_BUTTON_SIZE 15
#define SCROLL_MAX_LEN 215
#define BLACK 0x000000
#define WHITE 0xFFFFFF
#define BLUE  0x0000FF

uint32_t wheels;
char* title = "Boxlib example";
char ed_buff[ED_BUFF_LEN];


scrollbar scroll = {15, WIN_W - 26, WIN_H - 29, 0, 0, 2, 215, SCROLL_BUTTON_SIZE, 0,0x707070,0xD2CED0,0x555555};
progressbar pg = {0, 10, 10, 270, 35, 1, 0, 200, 0xB4B4B4, 0x2728FF, 0xA9A9A9};
edit_box ed={WIN_W-140,10,60,0xFFFFFF,0x6a9480,0,0x6a9480, BLACK | TEXT_SIZE, ED_BUFF_LEN, ed_buff,NULL,ed_focus};
check_box output_off={X_W(10, 15), Y_H(120,15), 10, WHITE, BLUE, BLACK | TEXT_SIZE, "Disable duplicate output",0};

void draw_window(){
        BeginDraw(); 
        DrawWindow(215,100,WIN_W,WIN_H,title, 0x858585, 0x34);     
		edit_box_draw(&ed);
		check_box_draw2(&output_off);
		if(!output_off.flags)
		{
			draw_text_sys(ed_buff, 10, 90, strlen(ed_buff), BLACK | TEXT_SIZE);
		}
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
	init_checkbox2(&output_off);
	set_event_mask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE+EVM_MOUSE_FILTER);
	while(1)
	{
		switch(GetOsEvent())
		{
			case KOLIBRI_EVENT_BUTTON:
				if (get_os_button() == 1) exit(0);
				break;
		  
			case KOLIBRI_EVENT_KEY:
				edit_box_key(&ed, get_key().val);
                draw_window();
				break;
			 
			case KOLIBRI_EVENT_REDRAW:
				draw_window();
				break;
			case KOLIBRI_EVENT_MOUSE: 
				edit_box_mouse(&ed);
				scrollbar_v_mouse(&scroll);
				pg.value = scroll.position;
				progressbar_draw(&pg);
				check_box_mouse2(&output_off);
				unsigned int scroll_strong = 10;
                wheels = GetMouseWheels();
				if(wheels & 0xFFFF)
                {
					if((short)wheels > 0){
						scroll.position += scroll_strong;
						if(scroll.position>scroll.max_area-scroll.cur_area)
						{
							scroll.position=scroll.max_area-scroll.cur_area;
						}
					}
                    else if((short)wheels < 0 && scroll.position > 0){
            			scroll.position -= scroll_strong;
						if((int)scroll.position<0){
							scroll.position=0;
						}
					}
            		scrollbar_v_draw(&scroll);
                }
				break;
		}
	}
}
