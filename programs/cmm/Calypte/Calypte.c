//Calypte 0.35 - Leency
//Calypte 0.15 - Punk Joker

/* 
TODO
- word wrap
- draw by line
- text selection
*/

#define MEMSIZE 1024*200

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/kolibri.h" 
#include "../lib/fs.h"
#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/collection.h"

#include "../lib/obj/iconv.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/libio.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/obj/librasterworks.h"

#include "../lib/patterns/simple_open_dialog.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

char default_dir[] = "/rd/1";
od_filter filter2 = { 8, "TXT\0\0" };

/*=========  MENU  ==========*/
?define MENU1 "File"
?define MENU2 "Encoding"
?define MENU3 "Reopen"

char menu_file_list[] =
"Open
Close
Properties
Exit";

char menu_encoding_list[] =
"UTF-8
KOI8-RU
CP1251
CP1252
ISO8859-5
CP866";

char menu_reopen_list[] =
"Tinypad
TextEdit
TextRead
WebView
FB2Read
HexView";

enum
{
	MENU_ID_FILE=10,
	FILE_SUBMENU_ID_OPEN=10,
	FILE_SUBMENU_ID_CLOSE,
	FILE_SUBMENU_ID_PROPERTIES,
	FILE_SUBMENU_ID_EXIT,

	MENU_ID_ENCODING=20,

	MENU_ID_REOPEN=30,
	FILE_SUBMENU_ID_TINYPAD=30,
	FILE_SUBMENU_ID_TEXTEDIT,
	FILE_SUBMENU_ID_TEXTREAD,
	FILE_SUBMENU_ID_WEBVIEW,
	FILE_SUBMENU_ID_FB2READ,
	FILE_SUBMENU_ID_HEXVIEW
};

int menu_file_x = 6;
int menu_encoding_x = NULL;
int menu_reopen_x = NULL;
/*======== MENU END ==========*/

#define TITLE "Calypte v0.38 dev"
char win_title[4096] = TITLE;

#define TOPPANELH 23
#define BOTPANELH 10
#define WIN_W 750
#define WIN_H 550
#define SCROLL_SIZE 15

proc_info Form;
llist rows;

int encoding;
	
dword old_width,old_height;

dword bufpointer;
dword bufsize;

scroll_bar scroll_v = { SCROLL_SIZE,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
scroll_bar scroll_h = { SCROLL_SIZE,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

collection s;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{   
	int id;

	load_dll(boxlib,    #box_lib_init,   0);
	load_dll(libio,     #libio_init,     1);
	//load_dll(libini,    #lib_init,       1);
	load_dll(iconv_lib, #iconv_open,     0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
	load_dll(librasterworks,  #rasterworks_drawText,0);
	OpenDialog_init stdcall (#o_dialog);

	if (param)
	{
		draw_window();
		OpenFile(#param);
		Prepare();
		DrawText();
	}
	
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);	

	loop()
	{
	  switch(WaitEvent())
	  {
		case evMouse:
			mouse.get();
			rows.wheel_size = 3;
			if (rows.MouseScroll(mouse.vert))
			{
				DrawText();
			}
			break;
		
		case evButton:
			id=GetButtonID();               
			if (id==1)
			{
				ExitProcess();
			}
			if (id==MENU_ID_FILE) 
			{
				EventShowMenu(menu_file_x, #menu_file_list, MENU_ID_FILE, NULL);
			}
			if (id==MENU_ID_ENCODING) 
			{
				EventShowMenu(menu_encoding_x, #menu_encoding_list, MENU_ID_ENCODING, encoding+1);
			}
			if (id==MENU_ID_REOPEN) 
			{
				EventShowMenu(menu_reopen_x, #menu_reopen_list, MENU_ID_REOPEN, NULL);
			}
			break;
		
		case evKey:
			GetKeys();
			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL)
			{
				if (key_scancode == SCAN_CODE_KEY_O)
				{
					EventOpenFile();
				}
				break;
			}
			if (rows.ProcessKey(key_scancode))
			{
				DrawText();
			}
			break;
		 
		 case evReDraw:
			if (menu.cur_y)
			{
				EventMenuClick();
			}
			draw_window();
			break;
	  }
   }
}

int DrawMenuButton(dword x,y,id,text)
{
	int textlen = strlen(text)*8;
	int padding = 12;
	DefineHiddenButton(x, y, textlen+padding+padding, TOPPANELH-2, id);
	WriteText(x+padding,y+4, 0x90, MixColors(system.color.work, system.color.work_text, 70), text);
	return textlen+padding+padding;
}

void draw_window()
{
	system.color.get();
	DefineAndDrawWindow(screen.width-WIN_W/2,screen.height-WIN_H/2,WIN_W,WIN_H,0x73,0xFFFFFF,#win_title,0);
	GetProcessInfo(#Form, SelfInfo);
	DrawBar(0, 0, Form.cwidth, TOPPANELH-1, system.color.work);
	DrawBar(0, TOPPANELH-1, Form.cwidth, 1, system.color.work_dark);
	DrawBar(0, Form.cheight-BOTPANELH, Form.cwidth, BOTPANELH, system.color.work);
	
	menu_encoding_x = menu_file_x + DrawMenuButton(menu_file_x, 0, MENU_ID_FILE, MENU1);
	menu_reopen_x = menu_encoding_x + DrawMenuButton(menu_encoding_x, 0, MENU_ID_ENCODING, MENU2);
	DrawMenuButton(menu_reopen_x, 0, MENU_ID_REOPEN, MENU3);

	if (old_width!=Form.width) || (old_height!=Form.height)
	{
		old_width = Form.width;
		old_height = Form.height;
		
		rows.no_selection = true;
		rows.SetFont(8, 14, 0x90);
		rows.SetSizes(0, TOPPANELH, Form.cwidth - SCROLL_SIZE, Form.cheight - TOPPANELH - BOTPANELH, 20);
		rows.column_max = rows.w / rows.font_w;

		if (bufpointer)
		{
			Prepare();
		}
		rows.CheckDoesValuesOkey();
	}
	DrawRectangle(rows.x+rows.w-1, rows.y, SCROLL_SIZE, rows.h-1, 0xEEEeee);
	DrawText();
}

void OpenFile(dword _path)
{
	strcpy(#param, _path);
	sprintf(#win_title, "%s - %s", TITLE, #param);
	rows.KeyHome();
	file_size stdcall (#param);
	bufsize = EBX;
	if (bufsize)
	{
		bufpointer = mem_Free(bufpointer);
		bufpointer = mem_Alloc(bufsize);
		if (ReadFile(0, bufsize, bufpointer, #param) != 0)
		{
			bufpointer = 0;
			notify("'Error opening file'-E");
			return;
		}
	}
	if (encoding!=CH_CP866)
	{
		ChangeCharset(charsets[encoding], "CP866", bufpointer);
	}
}

enum
{
	PARSE_CALCULATE_ROWS_COUNT,
	PARSE_DRAW_PREPARE,
};

void Parse(int mode)
{
	int pos=0;
	int sub_pos=0;
	int len_str = 0;
	bool do_eof = false;
	word bukva[2];

	while(1)
	{
		while(1)
		{
			bukva = DSBYTE[bufpointer+pos+len_str];
			if (bukva=='\0')
			{
				do_eof = true;
				break;
			}
			if (bukva==0x0a)
			{
				break;
			}
			else
			{
				len_str++;
			}
		}
		if (len_str<=rows.column_max) 
		{
			if (mode==PARSE_DRAW_PREPARE)
			{
				s.addn(bufpointer+pos, len_str);
			}
			pos += len_str+1;
		}
		else
		{
			if (mode==PARSE_DRAW_PREPARE)
			{
				s.addn(bufpointer+pos, rows.column_max);
			}
			pos += rows.column_max;
		}
		sub_pos++;
		if (mode==PARSE_CALCULATE_ROWS_COUNT) && (do_eof)
		{
			break;
		}
		if (mode==PARSE_DRAW_PREPARE) && (pos>=bufsize-1)
		{
			break;
		}
		len_str = 0;
	}
	if (mode == PARSE_CALCULATE_ROWS_COUNT)
	{
		rows.count = sub_pos;
		Parse(PARSE_DRAW_PREPARE);
	}
}

void Prepare()
{
	Parse(PARSE_CALCULATE_ROWS_COUNT);
}

void DrawText()
{
	int i=0, top;

	if (rows.count<rows.visible)
	{
		top = rows.count;
	}
	else
	{
		if (rows.count-rows.first<=rows.visible)
		{
			top = rows.count-rows.first-1;
		}
		else
		{
			top = rows.visible;
		}
	}

	if (bufpointer)
	{
		for (i=0; i<top; i++)
		{
			DrawBar(0, i*rows.item_h+TOPPANELH, rows.w, rows.item_h, 0xFFFFFF);
			WriteText(2, i*rows.item_h+TOPPANELH, 0x90, 0x000000, s.get(i+rows.first));
		}
	}
	DrawBar(0, i*rows.item_h+rows.y, rows.w, -i*rows.item_h + rows.h, 0xFFFfff);
	DrawVerticalScroll();
}

void DrawVerticalScroll()
{
	scroll_v.max_area = rows.count;
	scroll_v.cur_area = rows.visible;
	scroll_v.position = rows.first;
	scroll_v.start_y = rows.y;
	scroll_v.size_y = rows.h;
	scroll_v.start_x = rows.w + rows.x -1;
	scroll_v.all_redraw = 0;
	scrollbar_v_draw(#scroll_v);
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void EventMenuClick()
{
	switch(menu.cur_y)
	{
		//File
		case FILE_SUBMENU_ID_OPEN:
			EventOpenFile();
			break;
		case FILE_SUBMENU_ID_CLOSE:
			EventCloseFile();
			break;
		case FILE_SUBMENU_ID_PROPERTIES:
			EventShowFileProperties();
			break;
		case FILE_SUBMENU_ID_EXIT:
			ExitProcess();
			break;
		//Encoding
		case MENU_ID_ENCODING...MENU_ID_ENCODING+9:
			EventChangeEncoding(menu.cur_y-MENU_ID_ENCODING);
			break;
		//Reopen
		case FILE_SUBMENU_ID_TINYPAD:
			EventOpenFileInAnotherProgram("/sys/tinypad");
			break;
		case FILE_SUBMENU_ID_TEXTEDIT:
			EventOpenFileInAnotherProgram("/sys/develop/t_edit");
			break;
		case FILE_SUBMENU_ID_TEXTREAD:
			EventOpenFileInAnotherProgram("/sys/txtread");
			break;
		case FILE_SUBMENU_ID_WEBVIEW:
			EventOpenFileInAnotherProgram("/sys/network/webview");
			break;
		case FILE_SUBMENU_ID_FB2READ:
			EventOpenFileInAnotherProgram("/sys/fb2read");
			break;
		case FILE_SUBMENU_ID_HEXVIEW:
			EventOpenFileInAnotherProgram("/sys/develop/heed");
			break;
	}
	menu.cur_y = 0;
}

void EventShowMenu(dword _menu_item_x, _menu_list, _id, _selected)
{
	menu.selected = _selected;
	menu.show(Form.left+5 + _menu_item_x, Form.top+skin_height + TOPPANELH, 140, _menu_list, _id);
}

void EventOpenFile()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status)
	{
		OpenFile(#openfile_path);
		Prepare();
		draw_window();
	}
}

void EventCloseFile()
{
	if (bufpointer)
	{
		s.drop();
		bufpointer = mem_Free(bufpointer);
		strcpy(#win_title, TITLE);
		draw_window();
	}
}

void EventShowFileProperties()
{
	char ss_param[4096];
	if (bufpointer)
	{
		sprintf(#ss_param, "-p %s", #param);
		RunProgram("/sys/File managers/Eolite", #ss_param);
	}
}

void EventChangeEncoding(dword id)
{
	encoding = id;
	OpenFile(#openfile_path);
	Prepare();
	draw_window();
}

void EventOpenFileInAnotherProgram(dword _app)
{
	RunProgram(_app, #param);
}



stop:
