#define MEMSIZE 4096*25

#include "../lib/kfont.h"
#include "../lib/io.h"
#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/menu.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/libini.h"
#include "../lib/obj/iconv.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/patterns/libimg_load_skin.h"
#include "../lib/patterns/simple_open_dialog.h"

#define TOOLBAR_H 34
#define TOOLBAR_ICON_WIDTH  26
#define TOOLBAR_ICON_HEIGHT 24

#define DEFAULT_EDITOR "/sys/tinypad"

#define INTRO_TEXT "This is a plain Text Reader.\nTry to open some text file."
#define VERSION "Text Reader v1.2"
#define ABOUT "Idea: Leency, punk_joker
Code: Leency, Veliant, KolibriOS Team

Hotkeys:
Ctrl+O - open file
Ctrl+Up - bigger font
Ctrl+Down - smaller font
Ctrl+Tab - select charset
Ctrl+E - edit current document
 
Press any key..."

char default_dir[] = "/rd/1";
od_filter filter2 = {0,0};

scroll_bar scroll = { 15,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
llist list;

proc_info Form;
char title[4196];

byte help_opened = false;

enum {
	OPEN_FILE,
	MAGNIFY_MINUS,
	MAGNIFY_PLUS,
	CHANGE_ENCODING,
	RUN_EDIT,
	SHOW_INFO,
};

#include "ini.h"
#include "gui.h"
#include "prepare_page.h"


void InitDlls()
{
	load_dll(boxlib,    #box_lib_init,   0);
	load_dll(libio,     #libio_init,     1);
	load_dll(libimg,    #libimg_init,    1);
	load_dll(libini,    #lib_init,       1);
	load_dll(iconv_lib, #iconv_open,     0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
}


void main()
{   	
	InitDlls();	
	OpenDialog_init stdcall (#o_dialog);
	label.init(DEFAULT_FONT);
	Libimg_LoadImage(#skin, abspath("toolbar.png"));
	LoadIniSettings();
	OpenFile(#param);
	list.no_selection = true;
	SetEventMask(10000000000000000000000001100111b);
	loop()
	{
		switch(WaitEvent())
		{
			case evMouse:
				HandleMouseEvent();
				break;
			case evKey:
				HandleKeyEvent();
				break;
			case evButton:
				HandleButtonEvent();
				break;
			case evReDraw:
				if (menu.list.cur_y) {
					encoding = menu.list.cur_y - 10;
					OpenFile(#param); 
					PreparePage();
					menu.list.cur_y = NULL;
				};
				draw_window();
		}
	}
}


void HandleButtonEvent()
{
	
	byte btn = GetButtonID();
	if (btn==1) {
		SaveIniSettings();
		ExitProcess();
	}
	btn-=10;
	switch(btn)
	{
		case OPEN_FILE:
			EventOpenFile();
			break;
		case MAGNIFY_PLUS:
			EventMagnifyPlus();
			break;
		case MAGNIFY_MINUS:
			EventMagnifyMinus();
			break;
		case CHANGE_ENCODING:
			EventChangeEncoding();
			break;
		case RUN_EDIT:
			EventRunEdit();
			break;
		case SHOW_INFO:
			EventShowInfo();
			break;
	}
}


void HandleKeyEvent()
{
	if (help_opened) {
		help_opened = false;
		DrawPage();
		return; 
	}
	GetKeys();
	if (key_scancode==059) {
		EventShowInfo();
		return;
	}
	if (key_modifier & KEY_LCTRL) || (key_modifier & KEY_RCTRL) {
		switch (key_scancode)
		{
			case 024:
				EventOpenFile();
				break;
			case SCAN_CODE_UP:
				EventMagnifyPlus();
				break;
			case SCAN_CODE_DOWN:
				EventMagnifyMinus();
				break;
			case 018:
				EventRunEdit();
				break;
			case SCAN_CODE_TAB:
				EventChangeEncoding();
				break;
		}
		return;
	}
	if (list.ProcessKey(key_scancode))
		DrawPage();
}


void HandleMouseEvent()
{
	mouse.get();
	list.wheel_size = 7;
	if (list.MouseScroll(mouse.vert)) {
		DrawPage(); 
		return; 
	}
	scrollbar_v_mouse (#scroll);
	if (list.first != scroll.position) {
		list.first = scroll.position;
		DrawPage(); 
	}
}


/* ----------------------------------------------------- */

void EventOpenFile()
{
	OpenDialog_start stdcall (#o_dialog);
	OpenFile(#openfile_path);
	PreparePage();
}

void EventMagnifyPlus()
{
	label.size.pt++;
	if(!label.changeSIZE())
		label.size.pt--;
	else
		PreparePage();
}

void EventMagnifyMinus()
{
	label.size.pt--;
	if(!label.changeSIZE())
		label.size.pt++;
	else
		PreparePage();
}

void EventRunEdit()
{
	io.run(DEFAULT_EDITOR, #param);
}

void EventChangeEncoding()
{
	menu.selected = encoding + 1;
	menu.show(Form.left+104, Form.top+29+skin_height, 130, "UTF-8\nKOI8-RU\nCP1251\nCP1252\nISO8859-5\nCP866", 10);
}

void EventShowInfo() {
	help_opened = true;
	DrawBar(list.x, list.y, list.w, list.h, 0xFFFfff);
	WriteText(list.x + 10, list.y + 10, 10000001b, 0x555555, VERSION);
	WriteTextLines(list.x + 10, list.y+40, 10110000b, 0, ABOUT, 20);
}

/* ------------------------------------------- */


void OpenFile(dword f_path) 
{
	int tmp;
	if (ESBYTE[f_path]) {
		strcpy(#param, f_path);
		io.read(#param);
		strcpy(#title, #param);
		strcat(#title, " - Text Reader"); 
	}
	else {
		if (list.count) return;
		io.buffer_data = INTRO_TEXT;
		strcpy(#title, "Text Reader"); 
	}
	if (encoding!=CH_CP866) ChangeCharset(charsets[encoding], "CP866", io.buffer_data);
	list.KeyHome();
	list.ClearList();
}

void draw_window()
{
	DefineAndDrawWindow(Form.left,Form.top,Form.width,Form.height,0x73,0,#title,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;

	if (Form.width  < 200) { MoveSize(OLD,OLD,200,OLD); return; }
	if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); return; }
	
	DrawBar(0, 0, Form.cwidth, TOOLBAR_H - 1, 0xe1e1e1);
	DrawBar(0, TOOLBAR_H - 1, Form.cwidth, 1, 0x7F7F7F);
	
	DrawToolbarButton(OPEN_FILE,       8);
	DrawToolbarButton(MAGNIFY_PLUS,    42);
	DrawToolbarButton(MAGNIFY_MINUS,   67);
	DrawToolbarButton(CHANGE_ENCODING, 101);
	DrawToolbarButton(RUN_EDIT,        135);
	DrawToolbarButton(SHOW_INFO,       Form.cwidth - 34);
	
	if ((Form.cwidth-scroll.size_x-1 == list.w) && 
		(Form.cheight-TOOLBAR_H == list.h) && 
		(list.count) 
	)
	{
		DrawPage(); 
	} else {
		PreparePage();
	}
	DrawRectangle(scroll.start_x, scroll.start_y, scroll.size_x, scroll.size_y-1, scroll.bckg_col);
}

void DrawPage()
{
	_PutImage(list.x,list.y,list.w,list.h,list.first*list.item_h*list.w*3 + label.raw);
	DrawScroller();
}