#include <kos32sys.h>
#include <string.h>
#include <stdlib.h>

/* 
 EVENTS LOOK LIKE IN C--
*/

#define evReDraw 1
#define evKey 2
#define evButton 3



#define _size 4

#define bs 44

#define FIRSTC 0x137F00
#define SECONDC 0x0013FF

int field[_size][_size] = {
	{0, 0, 0, 1},
	{0, 0, 1, 0},
	{0, 0, 0, 1},
	{1, 0, 0, 0}
};

char* title = "Fridge v0.1";

void draw_window(){
        BeginDraw(); 
        DrawWindow(215,100,225, 225,title,0xEEEeee,0x34);
        for (int j = 10, yy = 0; yy<_size; j+=bs, yy++)
			for (int i = 10, xx = 0; xx<_size; i+=bs, xx++)
			{
				if (field[yy][xx]) define_button(65536 * i + bs, 65536 * j + bs, ((xx+1)*10)+yy+1, FIRSTC);
				else define_button(65536 * i + bs, 65536 * j + bs, ((xx+1)*10)+yy+1, SECONDC);
			}
        EndDraw();
}

void Button() {
	int id = get_os_button();
	if (id == 1) exit(0); else
	{
		int x = (id/10)-1;
		int y = (id%10)-1;
		
		for (int i = 0; i<_size; i++)
			if (field[i][x]) field[i][x] = 0; else field[i][x] = 1;
		
		for (int i = 0; i<_size; i++)
			if (field[y][i]) field[y][i] = 0; else field[y][i] = 1;
			
		if (field[y][x]) field[y][x] = 0; else field[y][x] = 1;
		
		draw_window();
	}
}

int main()
{
    draw_window();
    while(1)
    {
		switch(get_os_event())
		{
			case evButton:
				Button();
				break;
		  
			case evKey:
				get_key();
				break;
			 
			case evReDraw:
				draw_window();
				break;
		}
    }
}
