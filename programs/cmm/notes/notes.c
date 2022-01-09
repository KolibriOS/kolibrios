// Notes v1.1

#define MEMSIZE 1024*40
#define ENTRY_POINT #main

#include "..\lib\kolibri.h" 

#include "..\lib\obj\box_lib.h"
#include "..\lib\gui.h"
#include "..\lib\list_box.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#ifdef LANG_RUS
	?define WINDOW_CAPTION "Заметки"
	?define DELETE_TEXT "Удалить";
#else
	?define WINDOW_CAPTION "Notes and reminders"
	?define DELETE_TEXT "Delete";
#endif

#define RED_LINE_X 22
#define COL_RED_LINE 0xF3C9C9
unsigned char edge[sizeof(file "img/edge.raw")]= FROM "img/edge.raw"; //292x6
#define EDGE_H 6
#define TITLE_H 24
#define HEADER_HEIGHT TITLE_H+EDGE_H
#define LINES_COUNT 13

#define WIN_W 270
#define WIN_H RED_LINE_X*LINES_COUNT+HEADER_HEIGHT+4

#define DELETE_BTN 4;
#define DELETE_W sizeof(DELETE_TEXT)+2*6

#include "engine.h"

proc_info Form;

#include "ini.h"
	

edit_box notebox = {WIN_W-RED_LINE_X-6,RED_LINE_X+5,RED_LINE_X,
	COL_BG_ACTIVE, 0x94AECE,COL_BG_ACTIVE,0xffffff,0, 
	MAX_LINE_CHARS-1, NULL,0,ed_always_focus+ed_focus};
dword lists[] = { 0xEAEAEA, 0xCDCDCD, 0xF0F0F0, 0xD8D8D8, 0 };

bool delete_active = false;
bool window_dragable = true;
block delBtn = { WIN_W-DELETE_W-1, NULL, DELETE_W, RED_LINE_X};

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{   
	bool first_redraw=true;
	load_dll(boxlib, #box_lib_init,0);

	if (GetCpuFrequency()/1000000>=1000) window_dragable=true; else window_dragable=false;
	
	if (param) notes.OpenTxt(#param); else notes.OpenTxt("/sys/notes.txt");

	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	LoadIniSettings();
	loop() switch(@WaitEvent())
	{
		case evMouse:
			edit_box_mouse stdcall (#notebox);

			mouse.get();

			if (delete_active) && (delBtn.hovered()) break;

			if (mouse.lkm) && (mouse.y<TITLE_H) && (mouse.x<WIN_W-39) 
				&& (window_dragable) EventDragWindow();

			if (mouse.pkm) 
			&& (notes.MouseOver(mouse.x, mouse.y)) {
				if (notes.ProcessMouse(mouse.x, mouse.y)) EventListRedraw();
				EventDrawDeleteButton();
			} 

			if (mouse.key&MOUSE_LEFT)&&(mouse.up) 
			&& (notes.ProcessMouse(mouse.x, mouse.y)) {
				notebox.pos = mouse.x - notebox.left / 6;
				EventListRedraw();
				EventActivateLine(notes.cur_y);
			}

			break;

		 case evButton:
			@GetButtonID();
			switch(EAX)
			{
				case CLOSE_BTN:
					EventExitApp();
					break;
				case DELETE_BTN:
					EventDeleteCurrentNode();
					break;
				default: 
					EventCheckBoxClick(EAX-CHECKBOX_ID);
					break;
			}  
			break;
	 
		case evKey:
			@GetKeys();
			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL)
			{
				if (key_scancode == SCAN_CODE_SPACE)
				{
					EventCheckBoxClick(notes.cur_y);
				}
			}
			switch(key_scancode)
			{
				case SCAN_CODE_ESC:
					EventExitApp();
					break;
				case SCAN_CODE_DOWN:
					EventActivateLine(notes.cur_y+1);
					break;
				case SCAN_CODE_UP:
					EventActivateLine(notes.cur_y-1);
					break;
				default:
					if (notes.cur_y>=0) {
						EAX = key_editbox;
						edit_box_key stdcall (#notebox);
					}
				
			}
			break;
		 
		 case evReDraw:
		 	draw_window();
		 	if (first_redraw) {
		 		first_redraw = false;
		 		EventActivateLine(0);
		 	}
   }
}

void DrawCloseButton(dword x,y,w,h)
{
	DrawRectangle(x,y,w,h,0xC96832);
	DrawRectangle3D(x+1,y+1,w-2,h-2,0xE6A37F,0xDD8452);
	PutPixel(x+w-1, y+1, 0xE08C5E);
	DefineButton(x+1,y+1,w-1,h-1,CLOSE_BTN+BT_HIDE,0);
	WriteTextB(-6+w/2+x,h/2-4+y,0x80,0xFFFfff,"x");
}

void draw_window()
{
	int i;
	if (window_dragable) 
		DefineUnDragableWindow(Form.left,Form.top,WIN_W, WIN_H);
	else 
		DefineDragableWindow(Form.left,Form.top,WIN_W, WIN_H);
	GetProcessInfo(#Form, SelfInfo);
	notes.SetSizes(RED_LINE_X+1, HEADER_HEIGHT, WIN_W-1, RED_LINE_X*LINES_COUNT, RED_LINE_X);
	DrawRectangle3D(0,0,WIN_W,TITLE_H-1,0xBB6535, 0xCD6F3B);
	DrawRectangle3D(1,1,WIN_W-2,TITLE_H-3,0xEFBFA4, 0xDD8452);
	DrawBar(2,2,WIN_W-3,TITLE_H-4,0xE08C5E);
	WriteText(9,TITLE_H/2-6,0x90,0xA9613A,WINDOW_CAPTION);
	WriteTextB(7,TITLE_H/2-7,0x90,0xFFFfff,WINDOW_CAPTION);
	PutImage(1, TITLE_H, 292,EDGE_H, #edge);
	PutPixel(notes.x, notes.y-1, COL_RED_LINE);
	ECX-=1;	$int 0x40;
	DrawCloseButton(WIN_W-23,4,16,16);
	DrawRectangle(0,TITLE_H,WIN_W,WIN_H-HEADER_HEIGHT+EDGE_H,0xBBBBBB);
	for (i=0; lists[i]!=0; i++) DrawBar(1,WIN_H-i-1, WIN_W-1, 1, lists[i]);
	EventListRedraw();
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventActivateLine(int line_n)
{
	if (line_n<0) || (line_n>=notes.count) return;
	notes.cur_y = line_n;
	notebox.text = notes.DrawLine(notes.cur_y, notes.item_h);
	EventListRedraw();

	notebox.size = strlen(notebox.text);
	notebox.offset = notebox.shift = notebox.shift_old = 0;
	notebox.cl_curs_x = notebox.cl_curs_y = 0;
	if (notebox.pos > notebox.size) notebox.pos = notebox.size;
	notebox.top = notes.cur_y*notes.item_h+4+notes.y;
	edit_box_draw stdcall(#notebox);
}

void EventExitApp()
{
	notes.SaveTxt();
	SaveIniSettings();
	ExitProcess();
}

void EventDrawDeleteButton()
{
	notes.DrawLine(notes.cur_y, notes.item_h);
	delBtn.y = notes.cur_y*notes.item_h+notes.y;
	DefineButton(delBtn.x, delBtn.y, delBtn.w, delBtn.h, DELETE_BTN, 0xFF0000);
	WriteText(delBtn.x+10, delBtn.h/2-3 + delBtn.y, 0x80, 0xFFFfff, DELETE_TEXT);
	notebox.top=-20;
	delete_active = true;
}

void EventDeleteCurrentNode()
{
	dword t;
	t = notes.cur_y;
	notes.lines[t].Delete();
	EventListRedraw();
}

void EventListRedraw()
{
	delete_active = false;
	DeleteButton(DELETE_BTN);
	notes.DrawList();
}

void EventCheckBoxClick(int id)
{
	notes.lines[id].state ^= 1;
	EventListRedraw();
}

stop:
