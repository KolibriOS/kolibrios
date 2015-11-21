#define MEMSIZE 4096*20

#include "../lib/font.h"
#include "../lib/io.h"
#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/libini.h"
#include "../lib/obj/iconv.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/patterns/libimg_load_skin.h"
#include "../lib/patterns/simple_open_dialog.h"

#define TOOLBAR_H 34
#define TOOLBAR_ICON_WIDTH  26
#define TOOLBAR_ICON_HEIGHT 24

#define TOOLBAR_OPEN_FILE_LEFT       8
#define TOOLBAR_MAGNIFY_PLUS_LEFT    42
#define TOOLBAR_MAGNIFY_MINUS_LEFT   67
#define TOOLBAR_CHANGE_ENCODING_LEFT 101
#define TOOLBAR_RUN_EDIT_LEFT        135

#define DEFAULT_FONT   "/sys/fonts/Tahoma.kf"
#define DEFAULT_EDITOR "/sys/tinypad"

#define INTRO_TEXT "This is a plain text reader.\nTry to open some text file."

char default_dir[] = "/rd/1";
od_filter filter2 = {0,0};

scroll_bar scroll = { 15,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
llist list;

proc_info Form;
char title[4196];

byte help_opened = false;

char char_width[255];
dword line_offset;
#define DWORD 4;

enum {
	OPEN_FILE = 10,
	MAGNIFY_MINUS,
	MAGNIFY_PLUS,
	CHANGE_ENCODING,
	RUN_EDIT,
	SHOW_INFO,
};

#include "ini.h"
#include "menu.h"

void InitDlls()
{
	load_dll(boxlib,    #box_lib_init,   0);
	load_dll(libio,     #libio_init,     1);
	load_dll(libimg,    #libimg_init,    1);
	load_dll(libini,    #lib_init,       1);
	load_dll(iconv_lib, #iconv_open,     0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
}

void EventShowInfo()
{
	ShowAbout();
}

void EventOpenFile()
{
	OpenDialog_start stdcall (#o_dialog);
	OpenFile(#openfile_path);
	PreparePage();
}

void EventMagnifyPlus()
{
	font.size.text++;
	if(!font.changeSIZE())
		font.size.text--;
	else
		PreparePage();
}

void EventMagnifyMinus()
{
	font.size.text--;
	if(!font.changeSIZE())
		font.size.text++;
	else
		PreparePage();
}

void EventRunEdit()
{
	io.run(DEFAULT_EDITOR, #param);
}

void EventChangeEncoding()
{
	CreateThread(#menu_rmb,#stak+4092);
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

void main()
{   	
	InitDlls();
	
	OpenDialog_init stdcall (#o_dialog);
	
	font.no_bg_copy = true;
	font.color      = 0;
	font.bg_color   = 0xFFFFFF;
	
	font.load(DEFAULT_FONT);
	
	if (!font.data) {
		io.run("/sys/@notify","'Error: Font is not loaded.' -E");
		ExitProcess();
	}
	
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
				if (action_buf) {
					OpenFile(#param); 
					PreparePage();
					action_buf = false;
				};
				draw_window();
		}
	}
}

void draw_window()
{
	DefineAndDrawWindow(Form.left,Form.top,Form.width,Form.height,0x73,0,#title);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;

	if (Form.width  < 200) { MoveSize(OLD,OLD,200,OLD); return; }
	if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); return; }
	
	DrawBar(0, 0, Form.cwidth, TOOLBAR_H - 1, 0xe1e1e1);
	DrawBar(0, TOOLBAR_H - 1, Form.cwidth, 1, 0x7F7F7F);
	
	DrawToolbarButton(OPEN_FILE,       TOOLBAR_OPEN_FILE_LEFT);
	DrawToolbarButton(MAGNIFY_PLUS,    TOOLBAR_MAGNIFY_PLUS_LEFT);
	DrawToolbarButton(MAGNIFY_MINUS,   TOOLBAR_MAGNIFY_MINUS_LEFT);
	DrawToolbarButton(CHANGE_ENCODING, TOOLBAR_CHANGE_ENCODING_LEFT);
	DrawToolbarButton(RUN_EDIT,        TOOLBAR_RUN_EDIT_LEFT);
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
	_PutImage(list.x,list.y,list.w,list.h,list.first*list.item_h*list.w*3 + font.buffer);
	DrawScroller();
}

void PreparePage() 
{
	char line[4096]=0;
	dword line_start;
	byte ch;
	dword bufoff;
	dword line_length=30;
	dword stroka_y = 5;
	dword stroka=0;
	int i, srch_pos;
	
	font.changeSIZE();
	list.w = Form.cwidth-scroll.size_x-1;
	//get font chars width, need to increase performance
	for (i=0; i<256; i++) char_width[i] = font.symbol_size(i);
	//get font buffer height
	for (bufoff=io.buffer_data; ESBYTE[bufoff]; bufoff++)
	{
		ch = ESBYTE[bufoff];
		line_length += char_width[ch];
		if (line_length>=list.w) || (ch==10) {
			srch_pos = bufoff;
			loop()
			{
				if (__isWhite(ESBYTE[srch_pos])) { bufoff=srch_pos+1; break; } //normal word-break
				if (srch_pos == line_start) break; //no white space found in whole line
				srch_pos--;
			}
			line_start = bufoff;
			line_length = 30;
			stroka++;
		}
	}
	//draw text in buffer
	list.count = stroka+2;
	list.SetSizes(0, TOOLBAR_H, list.w, Form.cheight-TOOLBAR_H, font.size.text+1);
	if (list.count < list.visible) list.count = list.visible;

	font.size.height = list.count+1*list.item_h;
	font.buffer_size = 0;

	line_length = 30;
	line_start = io.buffer_data;
	for (bufoff=io.buffer_data; ESBYTE[bufoff]; bufoff++)
	{
		ch = ESBYTE[bufoff];
		line_length += char_width[ch];
		if (line_length>=list.w) || (ch==10)
		{
			//set word break
			srch_pos = bufoff;
			loop()
			{
				if (__isWhite(ESBYTE[srch_pos])) { bufoff=srch_pos+1; break; } //normal word-break
				if (srch_pos == line_start) break; //no white space found in whole line
				srch_pos--;
			}
			i = bufoff-line_start;
			strlcpy(#line, line_start, i);
			font.prepare_buf(8,stroka_y,list.w,font.size.height, #line);
			stroka_y += list.item_h;
			line_start = bufoff;
			line_length = 30;
		}
	}
	font.prepare_buf(8,stroka_y,list.w,font.size.height, line_start);
	SmoothFont(font.buffer, font.size.width, font.size.height);
	DrawPage();
}

void DrawToolbarButton(char image_id, int x)
{
	DefineButton(x, 5, TOOLBAR_ICON_WIDTH-1, TOOLBAR_ICON_HEIGHT-1, 10+image_id + BT_HIDE, 0);
	img_draw stdcall(skin.image, x, 5, TOOLBAR_ICON_WIDTH, TOOLBAR_ICON_HEIGHT, 0, image_id*TOOLBAR_ICON_HEIGHT);
}

void DrawScroller()
{
	scroll.max_area = list.count;
	scroll.cur_area = list.visible;
	scroll.position = list.first;
	scroll.all_redraw = 0;
	scroll.start_x = list.x + list.w;
	scroll.start_y = list.y;
	scroll.size_y = list.h;
	scroll.start_x = list.x + list.w;
	scrollbar_v_draw(#scroll);
}

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



char *about[] = {
	"Text Reader v1.01",
	"Idea: Leency, punk_joker",
	"Code: Leency, Veliant, KolibriOS Team",
	" ",
	"Hotkeys:",
	"Ctrl+O - open file",
	"Ctrl+Up - bigger font",
	"Ctrl+Down - smaller font",
	"Ctrl+Tab - select charset",
	"Ctrl+E - edit current document",
	" ",
	"Press any key...",
	0
};

ShowAbout() {
	int i;
	help_opened = true;
	DrawBar(list.x, list.y, list.w, list.h, 0xFFFfff);
	WriteText(list.x + 10, list.y + 10, 10000001b, 0x555555, about[0]);
	for (i=1; about[i]; i++) WriteText(list.x + 10, i+1*20 + list.y, 10110000b, 0, about[i]);
}
