/*
 * BACKGEN - Background generator
 * Author: Leency
 * Licence: GPL v2
*/

#define MEMSIZE 4096*40

#include "../lib/gui.h"
#include "../lib/obj/libimg.h"
#include "../lib/patterns/rgb.h"
#include "../lib/patterns/libimg_load_skin.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define T_TITLE "Icon Editor 0.05"

#define MAX_COLORS   32
#define TOOLBAR_H    24+8
#define PALLETE_SIZE 116

rect wrapper = { 10, TOOLBAR_H+5, NULL, NULL };
rect right_bar = { NULL, TOOLBAR_H+5, 280, NULL };

dword active_color_1 = 0x000000;
dword active_color_2 = 0xFFFfff;

enum {
	BTN_NEW = 40,
	BTN_OPEN,
	BTN_SAVE,
	BTN_MOVE_LEFT,
	BTN_MOVE_RIGHT,
	BTN_MOVE_UP,
	BTN_MOVE_DOWN,
	BTN_FLIP_HOR,
	BTN_FLIP_VER,
	BTN_ROTATE_LEFT,
	BTN_ROTATE_RIGHT,
	BTN_PICK,
	BTN_PALETTE_COLOR_MAS = 100,
};

proc_info Form;

bool pick_active = false;

more_less_box zoom = { NULL, NULL, NULL, 1, MAX_COLORS, 22, 23, "Zoom" };

dword palette_colors[] = {
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
_colors canvas;

void main()
{
	word btn;

	load_dll(libio,  #libio_init,  1);
	load_dll(libimg, #libimg_init, 1);
	Libimg_LoadImage(#skin, "/sys/icons16.png");

	canvas.set_default_values();
	zoom.value = canvas.cell_size;

	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);

	loop() switch(WaitEvent())
	{
		case evMouse:
			mouse.get();
			if (pick_active) EventPickColor(mouse.lkm, mouse.pkm);
			else {
				if (mouse.lkm) 
				&& (mouse.x>canvas.x) && (mouse.y>canvas.y) 
				&& (mouse.y<canvas.y+canvas.h) && (mouse.x<canvas.x+canvas.w)
				{
					canvas.set_color(mouse.y-canvas.y/canvas.cell_size, 
						mouse.x-canvas.x/canvas.cell_size, active_color_1);
					canvas.draw_all_cells();
				}
			}
			break;

		case evButton:
			btn = GetButtonID();
			switch(btn)
			{
				case BTN_MOVE_LEFT:
					canvas.move(DIRECTION_LEFT);
					break;
				case BTN_MOVE_RIGHT:
					canvas.move(DIRECTION_RIGHT);
					break;
				case BTN_MOVE_UP:
					canvas.move(DIRECTION_UP);
					break;
				case BTN_MOVE_DOWN:
					canvas.move(DIRECTION_DOWN);
					break;
				case CLOSE_BTN:
					ExitProcess();
				case BTN_PICK:
					EventPickActivate();
					break;
			}              
			if (btn >= BTN_PALETTE_COLOR_MAS) && (btn < BTN_PALETTE_COLOR_MAS+PALLETE_SIZE) 
			{ 
				active_color_1 = palette_colors[btn-BTN_PALETTE_COLOR_MAS]; 
				DrawActiveColor(NULL); 
			}
			if (zoom.click(btn)) EventZoom();
			break;
	  
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) ExitProcess();
			if (key_scancode == SCAN_CODE_KEY_I) EventPickActivate();
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}

void DrawToolbarButton(dword _id, _x, _icon_n)
{
	DrawWideRectangle(_x, 4, 24, 24, 4, 0xFFFfff);
	DefineHiddenButton(_x, 4, 23, 23, _id);
	img_draw stdcall(skin.image, _x+4, 8, 16, 16, 0, _icon_n*16);
}

void DrawStatusBar()
{
	zoom.y = wrapper.y + wrapper.h + 6;
	zoom.x = wrapper.x;
	zoom.draw();

	sprintf(#param,"Canvas: %ix%i", canvas.rows, canvas.columns);
	WriteText(wrapper.x+wrapper.w-calc(strlen(#param)*8), zoom.y+2, 0x90, system.color.work_text, #param);
}

void draw_window()
{
	#define TB_ICON_PADDING 26
	incn tx;
	system.color.get();
	DefineAndDrawWindow(215, 100, 700, 540, 0x33, system.color.work, T_TITLE, 0);
	GetProcessInfo(#Form, SelfInfo);
	right_bar.x = Form.cwidth - right_bar.w;

	tx.n = wrapper.x - TB_ICON_PADDING;
	DrawToolbarButton(BTN_NEW,    tx.inc(TB_ICON_PADDING), 2); //not implemented
	DrawToolbarButton(BTN_OPEN,   tx.inc(TB_ICON_PADDING), 0); //not implemented
	DrawToolbarButton(BTN_SAVE,   tx.inc(TB_ICON_PADDING), 5); //not implemented
	DrawToolbarButton(BTN_MOVE_LEFT,  tx.inc(TB_ICON_PADDING+8),   30);
	DrawToolbarButton(BTN_MOVE_RIGHT, tx.inc(TB_ICON_PADDING),   31);
	DrawToolbarButton(BTN_MOVE_UP,    tx.inc(TB_ICON_PADDING),   32);
	DrawToolbarButton(BTN_MOVE_DOWN,  tx.inc(TB_ICON_PADDING),   33);
	
	DrawToolbarButton(BTN_FLIP_HOR,   tx.inc(TB_ICON_PADDING+8), 34); //not implemented
	DrawToolbarButton(BTN_FLIP_VER,   tx.inc(TB_ICON_PADDING),   35); //not implemented
	DrawToolbarButton(BTN_ROTATE_LEFT,   tx.inc(TB_ICON_PADDING), 36); //not implemented
	DrawToolbarButton(BTN_ROTATE_RIGHT,  tx.inc(TB_ICON_PADDING), 37); //not implemented

	DrawToolbarButton(BTN_PICK,   tx.inc(TB_ICON_PADDING+8), 38);

	DrawEditArea();

	DrawDefaultColors(right_bar.x, right_bar.y);
	DrawActiveColor(Form.cheight-40);

	DrawStatusBar();
}

:void DrawEditArea()
{
	dword color1=0xC0C0C0;
	int left_padding;
	int top_padding;

	wrapper.w = Form.cwidth - right_bar.w - 30;
	wrapper.h = Form.cheight - TOOLBAR_H - 35;

	//canvas{
	canvas.x = -canvas.cell_size*canvas.columns+wrapper.w/2 + wrapper.x;
	canvas.y = -canvas.cell_size*canvas.rows+wrapper.h/2 + wrapper.y;
	DrawRectangle(canvas.x-1, canvas.y-1, canvas.w+1, canvas.h+1, 0x808080);
	canvas.draw_all_cells();	
	//}

	left_padding = canvas.x-wrapper.x-1;
	top_padding = canvas.y-wrapper.y-1;

	DrawRectangle(wrapper.x-1, wrapper.y-1, wrapper.w+1, wrapper.h+1, system.color.work_graph);

	if (left_padding>0)
	{
		DrawBar(wrapper.x, wrapper.y, wrapper.w, top_padding, color1);
		DrawBar(wrapper.x, wrapper.y+wrapper.h-top_padding, wrapper.w, top_padding, color1);		
	}
	if (top_padding>0)
	{
		DrawBar(wrapper.x, wrapper.y+left_padding, left_padding, wrapper.h-left_padding-left_padding, color1);
		DrawBar(wrapper.x+wrapper.w-left_padding, wrapper.y+left_padding, left_padding, wrapper.h-left_padding-left_padding, color1);		
	}
}

void DrawActiveColor(dword iny)
{
	static dword outy;
	if (iny != NULL) outy = iny;
	DrawBar(right_bar.x, outy, 20, 20, active_color_1);
	sprintf(#param, "%A", active_color_1);
	EDI = system.color.work;
	WriteText(right_bar.x + 30, outy + 3, 0xD0, system.color.work_text, #param+4);
}

void DrawDefaultColors(dword _x, _y)
{
	int r, c, i;
	int cellw = 20;

	i = 0;
	for (r = 0; r < 9; r++)
	{
		for (c = 0; c < 13; c++)
		{
			DrawBar(c*cellw + _x, r*cellw + _y, cellw, cellw, palette_colors[PALLETE_SIZE-i]);
			DefineHiddenButton(c*cellw + _x, r*cellw + _y, cellw-1, cellw-1, BTN_PALETTE_COLOR_MAS+PALLETE_SIZE-i);
			i++;
		}
	}
	DrawRectangle(_x-1, _y-1, c*cellw+1, r*cellw+1, system.color.work_light);
	DrawRectangle(_x-2, _y-2, c*cellw+3, r*cellw+3, system.color.work_dark);
}


//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void EventPickActivate()
{
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE);
	pick_active = true;
}

void EventPickColor(dword lkm_status, pkm_status)
{
	active_color_1 = GetPixelColorFromScreen(mouse.x + Form.left + 5, mouse.y + Form.top + skin_height);
	DrawActiveColor(NULL);
	if (mouse.down) && (mouse.key&MOUSE_LEFT) {
		pick_active = false;
		SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	}
}

void EventZoom()
{
	canvas.cell_size = zoom.value;
	canvas.w = canvas.columns * canvas.cell_size;
	canvas.h = canvas.rows * canvas.cell_size;
	if (canvas.w+2 > wrapper.w) || (canvas.h+2 > wrapper.h) { 
		zoom.click(23);
		EventZoom();
		return;
	}
	DrawEditArea();
}
