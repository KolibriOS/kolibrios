// Notes v0.7 ALPHA 

#define MEMSIZE 0xDAE80
#include "..\lib\kolibri.h" 
#include "..\lib\mem.h" 
#include "..\lib\strings.h" 
#include "..\lib\fs.h"
#include "..\lib\dll.h"

#include "..\lib\obj\box_lib.h"
#include "..\lib\gui.h"
#include "..\lib\encoding.h"
#include "..\lib\list_box.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

#define LANG_RUS

#ifdef LANG_RUS
	?define WINDOW_CAPTION "Заметки"
	?define DELETE_TEXT "Udoli";
#else
	?define WINDOW_CAPTION "Notes and reminders"
	?define DELETE_TEXT "Delete";
#endif

#define RED_LINE_X 22
#define COL_RED_LINE 0xF3C9C9
unsigned char edge[sizeof(file "edge.raw")]= FROM "edge.raw"; //292x6
#define EDGE_H 6
#define TITLE_H 24
#define HEADER_HEIGHT TITLE_H+EDGE_H

#define DELETE_BTN 4;

#include "engine.h"

dword editbox_text;
proc_info Form;
edit_box edit_box= {0,999,0,COL_BG_ACTIVE,0x94AECE,COL_BG_ACTIVE,0xffffff,0,MAX_LINE_CHARS-1,#editbox_text,#mouse,100000000000010b};
dword lists[] = { 0xEAEAEA, 0xCDCDCD, 0xF0F0F0, 0xD8D8D8, 0 };

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

struct KDelete {
	char width;
	char active;
	void Draw();
} DeleteBtn;

void KDelete::Draw(dword x, y, h)
{
	width = strlen(DELETE_TEXT)+2*6;
	x -= width+1;
	DefineButton(x, y, width, h-1, DELETE_BTN, 0xFF0000);
	WriteText(x+6+1, h/2-4+y, 0x80, 0xFFFfff, DELETE_TEXT);
}

void main()
{   
	int btn;
	dword cur_line_offset;
	load_dll(boxlib, #box_lib_init,0);
	
	if (param) notes.OpenTxt(#param); else notes.OpenTxt(abspath("notes.txt"));
	notes.list.cur_y = -1;

	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);

	loop() switch(WaitEvent())
	{
		case evMouse:
			mouse.get();

			if (notes.list.MouseOver(mouse.x, mouse.y)) {
    			notes.list.ProcessMouse(mouse.x, mouse.y);
				if (mouse.lkm) EventSelectItem();
				if (mouse.pkm) EventDrawDeleteButton();
			}

			if (mouse.lkm) && (mouse.y<TITLE_H) && (mouse.x<Form.width-30) EventDragWindow();

			if (notes.list.cur_y>=0) edit_box_mouse stdcall (#edit_box);
			break;

		 case evButton:
			btn = GetButtonID();               
			if (CLOSE_BTN == btn) EventExitApp();
			if (DELETE_BTN == btn)
			{
				notes.DeleteCurrentNode();
				notes.DrawList();
				DeleteBtn.active = 0;
				break;
			}
			if (btn>=CHECKBOX_ID) //checkboxes
			{
				notes.lines[btn-CHECKBOX_ID].state ^= 1;
				notes.DrawList();
				break;
			}
			break;
	 
		case evKey:
			GetKeys();
			if (SCAN_CODE_ESC  == key_scancode) EventExitApp();
			if (SCAN_CODE_DOWN == key_scancode) { EventActivateLine(notes.list.cur_y+1); break; }
			if (SCAN_CODE_UP   == key_scancode) { EventActivateLine(notes.list.cur_y-1); break; }
			if (notes.list.cur_y>=0) edit_box_key stdcall (#edit_box);
			break;
		 
		 case evReDraw:
		 	draw_window();
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
	notes.list.SetSizes(1, HEADER_HEIGHT, 270, RED_LINE_X*LINES_COUNT, RED_LINE_X);
	DefineAndDrawWindow(100,100,notes.list.w+1,notes.list.h+HEADER_HEIGHT+4,0x01,0,0,0x01fffFFF);
	//DefineDragableWindow(100, 100, notes.list.w+1, notes.list.h+HEADER_HEIGHT+4);
	GetProcessInfo(#Form, SelfInfo);
	DrawRectangle3D(0,0,Form.width,TITLE_H-1,0xBB6535, 0xCD6F3B);
	DrawRectangle3D(1,1,Form.width-2,TITLE_H-3,0xEFBFA4, 0xDD8452);
	DrawBar(2,2,Form.width-3,TITLE_H-4,0xE08C5E);
	WriteText(9,TITLE_H/2-6,0x90,0xA9613A,WINDOW_CAPTION);
	WriteTextB(7,TITLE_H/2-7,0x90,0xFFFfff,WINDOW_CAPTION);
	_PutImage(1, TITLE_H, 292,EDGE_H, #edge);
	PutPixel(notes.list.x+RED_LINE_X, notes.list.y-1, COL_RED_LINE);
	ECX-=1;	$int 0x40;
	DrawCloseButton(Form.width-23,4,16,16);
	DrawRectangle(0,TITLE_H,Form.width,Form.height-HEADER_HEIGHT+EDGE_H,0xBBBBBB);
	for (i=0; lists[i]!=0; i++) DrawBar(1,Form.height-i-1, Form.width-1, 1, lists[i]);
	edit_box.width = notes.list.w-RED_LINE_X-8;
	edit_box.left = notes.list.x+RED_LINE_X+4;

	notes.DrawList();
}

void DrawEditBox_Notes()
{
	edit_box.pos = edit_box.offset = edit_box.shift = 0;
	edit_box.size = strlen(edit_box.text);
	edit_box.top = notes.list.cur_y*notes.list.item_h+4+notes.list.y;
	edit_box_draw stdcall(#edit_box);	
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//

void EventActivateLine(int line_n)
{
	int old;
	if (line_n<0) || (line_n>notes.list.count) return;
	DeleteBtn.active = 0;
	//redraw lines
	notes.list.cur_y = line_n;
	edit_box.text = notes.DrawLine(notes.list.cur_y, notes.list.item_h);
	notes.DrawList();
	DrawEditBox_Notes();
}


void EventExitApp()
{
	notes.SaveTxt();
	ExitProcess();
}

void EventDrawDeleteButton()
{
	notes.DrawLine(notes.list.cur_y, notes.list.item_h);
	DeleteBtn.Draw(notes.list.x+notes.list.w, notes.list.cur_y*notes.list.item_h+notes.list.y, notes.list.item_h);
	edit_box.top=-20;
	DeleteBtn.active = 1;
}

void EventSelectItem()
{
	int id;
	id = mouse.y-notes.list.y/notes.list.item_h;
	if (DeleteBtn.active) && (mouse.x>notes.list.x+notes.list.w-DeleteBtn.width) return;
	if (id!=notes.list.cur_y) && (id<notes.list.count) EventActivateLine(id);
	else { notes.list.cur_y=-1; notes.DrawList(); }
}

stop:
