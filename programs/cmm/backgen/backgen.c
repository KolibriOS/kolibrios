/*
 * BACKGEN - Background generator
 * Author: Leency
 * Licence: GPL v2
*/

#define MEMSIZE 4096*10

#include "../lib/gui.h"
#include "../lib/patterns/rgb.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define MAX_COLORS 10

more_less_box x_count = { 10, 220, NULL, 1, MAX_COLORS, 22, 23, "X count" };
more_less_box y_count = { 10, 250, NULL, 1, MAX_COLORS, 24, 25, "Y count" };

rect preview = { 10, 10, 200, 200 };
rect right_bar = { 230, 10, 280, 400 };

dword active_color = 0xFFFfff;
char active_color_string[11]="0x00111222\0";

enum {
	APPLY_BACKGROUND_BTN = 10
};

proc_info Form;

dword default_colors[] = {
0x330000,0x331900,0x333300,0x193300,0x003300,0x003319,0x003333,0x001933,0x000033,0x190033,0x330033,0x330019,0x000000,
0x660000,0x663300,0x666600,0x336600,0x006600,0x006633,0x006666,0x003366,0x000066,0x330066,0x660066,0x660033,0x202020,
0x990000,0x994C00,0x999900,0x4C9900,0x009900,0x00994C,0x009999,0x004C99,0x000099,0x4C0099,0x990099,0x99004C,0x404040,
0xCC0000,0xCC6600,0xCCCC00,0x66CC00,0x00CC00,0x00CC66,0x00CCCC,0x0066CC,0x0000CC,0x6600CC,0xCC00CC,0xCC0066,0x606060,
0xFF0000,0xFF8000,0xFFFF00,0x80FF00,0x00FF00,0x00FF80,0x00FFFF,0x0080FF,0x0000FF,0x7F00FF,0xFF00FF,0xFF007F,0x808080,
0xFF3333,0xFF9933,0xFFFF33,0x99FF33,0x33FF33,0x33FF99,0x33FFFF,0x3399FF,0x3333FF,0x9933FF,0xFF33FF,0xFF3399,0xA0A0A0,
0xFF6666,0xFFB266,0xFFFF66,0xB2FF66,0x66FF66,0x66FFB2,0x66FFFF,0x66B2FF,0x6666FF,0xB266FF,0xFF66FF,0xFF66B2,0xC0C0C0,
0xFF9999,0xFFCC99,0xFFFF99,0xCCFF99,0x99FF99,0x99FFCC,0x99FFFF,0x99CCFF,0x9999FF,0xCC99FF,0xFF99FF,0xFF99CC,0xE0E0E0,
0xFFCCCC,0xFFE5CC,0xFFFFCC,0xE5FFCC,0xCCFFCC,0xCCFFE5,0xCCFFFF,0xCCE5FF,0xCCCCFF,0xE5CCFF,0xFFCCFF,0xFFCCE5,0xFFFFFF	
};

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

#include "colors_mas.h"

void main()
{
	word btn;

	colors.set_default_values();
	x_count.value = colors.columns;
	y_count.value = colors.rows;

	loop() switch(WaitEvent())
	{
		case evButton:
			btn = GetButtonID();               
			if (btn == CLOSE_BTN) ExitProcess();
			if (btn == APPLY_BACKGROUND_BTN) {
				SetBackgroundImage(colors.columns, colors.rows, colors.get_image(), DRAW_DESKTOP_BG_STRETCH);
			}
			if (x_count.click(btn)) EventChangeFieldSize();
			if (y_count.click(btn)) EventChangeFieldSize();
			if (btn >= 100) && (btn < 228) 
			{ 
				active_color = default_colors[btn-100]; 
				DrawActiveColor(NULL); 
			}
			if (btn >= 300) && (btn < 401) 
			{ 
				btn-=300;
				debugval("\n\nid",btn);
				colors.set_color(btn/colors.columns, btn%colors.columns, active_color);
				DrawColorsField();
			}
			break;
	  
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) ExitProcess();
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}

void draw_window()
{
	system.color.get();
	DefineAndDrawWindow(215, 100, right_bar.x+right_bar.w+9, 
		right_bar.y+right_bar.h+skin_height+5, 
		0x34, system.color.work, "Background generator",0
		);
	GetProcessInfo(#Form, SelfInfo);

	x_count.draw();
	y_count.draw();

	DrawColorsField();

	DrawStandartCaptButton(10, 320, APPLY_BACKGROUND_BTN, "Fill background");

	DrawRightBar();
}

void DrawColorsField()
{
	DrawRectangle(preview.x, preview.y, preview.w, preview.h, 0x808080);
	DrawBar(preview.x+1, preview.y+1, preview.w-1, preview.h-1, 0xBFCAD2); //F3F3F3

	colors.x = -colors.cell_size*colors.columns+preview.w/2 + preview.x;
	colors.y = -colors.cell_size*colors.rows+preview.h/2 + preview.y;
	colors.draw_all_cells();	
}

void DrawRightBar()
{
	int i;
	incn y;
	y.n = right_bar.y;
	EDI = system.color.work;
	WriteTextB(right_bar.x, y.inc(3), 0x90, system.color.work_text, "Active color");
	DrawActiveColor(y.inc(22));
	WriteTextB(right_bar.x, y.inc(34), 0x90, system.color.work_text, "Default colors");
	DrawDefaultColors(right_bar.x, y.inc(22));
}

void DrawActiveColor(dword iny)
{
	static dword outy;
	if (iny != NULL) outy = iny;
	colors.draw_cell(right_bar.x, outy, active_color);
	sprintf(#active_color_string, "%A", active_color);
	EDI = system.color.work;
	WriteText(right_bar.x + 30, outy + 3, 0xD0, system.color.work_text, #active_color_string+4);
}

void DrawDefaultColors(dword _x, _y)
{
	int r, c, i;
	i = 0;
	for (r = 0; r < 9; r++)
	{
		for (c = 0; c < 13; c++)
		{
			colors.draw_cell(c*colors.cell_size + _x, r*colors.cell_size + _y, default_colors[i]);
			DefineHiddenButton(c*colors.cell_size + _x, r*colors.cell_size + _y, colors.cell_size, colors.cell_size, 100+i);
			i++;
		}
	}
}


//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void EventChangeFieldSize()
{
	colors.columns = x_count.value;
	colors.rows = y_count.value;
	DrawColorsField();
}
