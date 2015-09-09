#define MEMSIZE 4096*20

#include "../lib/font.h"
#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/libini.h"
#include "../lib/obj/iconv.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/patterns/libimg_load_skin.h"
#include "../lib/patterns/simple_open_dialog.h"

#define TOOLBAR_H 34

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
	OPEN_FILE,
	MAGNIFY_MINUS,
	MAGNIFY_PLUS,
	CHANGE_ENCODING,
	RUN_EDIT,
	SHOW_INFO,
};

#include "ini.h"
#include "menu.h"

void main()
{   
	byte btn;
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);
	load_dll(iconv_lib, #iconv_open,0);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
	font.no_bg_copy = true; font.color = 0; font.bg_color = 0xFFFFFF;
	font.load("/sys/fonts/Tahoma.kf"); if (!font.data) { io.run("/sys/@notify","'Error: Font is not loaded.' -E"); ExitProcess(); }
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
      		mouse.get();
      		list.wheel_size = 7;
			if (list.MouseScroll(mouse.vert)) { DrawPage(); break; }
			scrollbar_v_mouse (#scroll); if (list.first != scroll.position) { list.first = scroll.position; DrawPage(); }
			break;
		case evKey:
			if (help_opened) { help_opened=false; DrawPage(); break; }
			GetKeys();
			if (key_scancode==059) goto _SHOW_INFO;
			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) {
				if (key_scancode==024) goto _OPEN_FILE;
				if (key_scancode==SCAN_CODE_UP) goto _MAGNIFY_PLUS;
				if (key_scancode==SCAN_CODE_DOWN) goto _MAGNIFY_MINUS;
				if (key_scancode==018) goto _RUN_EDIT;
				if (key_scancode==SCAN_CODE_TAB) goto _CHANGE_ENCODING;
				break;
			}
			if (list.ProcessKey(key_scancode)) DrawPage();
			break;
		case evButton:
			btn = GetButtonID();
			if (btn==1) { SaveIniSettings(); ExitProcess(); }
			btn-=10;
			if (btn==OPEN_FILE) { _OPEN_FILE: OpenDialog_start stdcall (#o_dialog); OpenFile(#openfile_path); PreparePage(); }
			else if (btn==MAGNIFY_PLUS)  { _MAGNIFY_PLUS: font.size.text++; if(!font.changeSIZE())font.size.text--; else PreparePage(); }
			else if (btn==MAGNIFY_MINUS) { _MAGNIFY_MINUS: font.size.text--; if(!font.changeSIZE())font.size.text++; else PreparePage(); }
			else if (btn==CHANGE_ENCODING) { _CHANGE_ENCODING: CreateThread(#menu_rmb,#stak+4092); break; }
			else if (btn==RUN_EDIT) { _RUN_EDIT: io.run("/sys/tinypad",#param); }
			else if (btn==SHOW_INFO) { _SHOW_INFO: ShowAbout(); }
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
	DrawToolbarButton(OPEN_FILE, 8);
	DrawToolbarButton(MAGNIFY_PLUS, 42);
	DrawToolbarButton(MAGNIFY_MINUS, 67);
	DrawToolbarButton(CHANGE_ENCODING, 101);
	DrawToolbarButton(RUN_EDIT, 135);
	DrawToolbarButton(SHOW_INFO, Form.cwidth - 34);
	if (Form.cwidth-scroll.size_x-1 == list.w) && (Form.cheight-TOOLBAR_H == list.h) && (list.count) DrawPage(); else PreparePage();
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
	DefineButton(x, 5, 26-1, 24-1, 10+image_id + BT_HIDE, 0);
	img_draw stdcall(skin.image, x, 5, 26, 24, 0, image_id*24);
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
		io.buffer_data = "This is a plain text reader.\nTry to open some text file.";
		strcpy(#title, "Text Reader"); 
	}
	if (encoding!=CH_CP866) ChangeCharset(charsets[encoding], "CP866", io.buffer_data);
	list.KeyHome();
	list.ClearList();
}



char *about[] = {
	"Text Reader v1.0",
	"Idea: Leency, punk_joker",
	"Code: Leency, KolibriOS Team",
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