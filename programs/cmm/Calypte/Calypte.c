#ifndef AUTOBUILD
	#include "lang.h--"
#endif

#define MEMSIZE 0x100000
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\file_system.h"
#include "..\lib\dll.h"
#include "..\lib\gui.h"
#include "..\lib\list_box.h"

#include "..\lib\obj\box_lib.h"
#include "..\lib\obj\proc_lib.h"
#include "..\lib\obj\libio_lib.h"

#ifdef LANG_RUS
	?define T_FILE "Файл"
	?define T_TYPE "Тип"
	?define T_SIZE "Размер"
	?define MENU1 "Файл"
	?define MENU1_SUBMENU1 "Открыть"
	?define MENU1_SUBMENU2 "Закрыть"
	?define MENU1_SUBMENU3 "Свойства"
	?define MENU1_SUBMENU4 "Выход"
#else
	?define T_FILE "File"
	?define T_TYPE "Type"
	?define T_SIZE "Size"
	?define MENU1 "File"
	?define MENU1_SUBMENU1 "Open"
	?define MENU1_SUBMENU2 "Close"
	?define MENU1_SUBMENU3 "Properties"
	?define MENU1_SUBMENU4 "Exit"
#endif

#ifdef LANG_RUS
struct menu_text_struct
{
	char menu[5];
	char sub_menu1[8];
	char sub_menu2[8];
	//char sub_menu3[9];
	char sub_menu4[6];
	byte end;
};
#else
struct menu_text_struct
{
	char menu[5];
	char sub_menu1[5];
	char sub_menu2[6];
	//char sub_menu3[11];
	char sub_menu4[5];
	byte end;
};
#endif

#define TOPPANELH 19
#define BOTPANELH 10
#define WIN_W 600
#define WIN_H 400

#define TITLE "Calypte v0.13"
char win_title[4096] = TITLE;
proc_info Form;

dword old_width,old_height;
llist tview;

byte active_properties = 0;
dword properties_window;

#include "include\gui.h"
#include "include\properties.h"
// #include "include\top_menu.h"
// #include "include\open_dial.h"

struct filter
{
	dword size;
	char ext1[4];
	//char ext2[4];
	//char ext3[4];
	//char ext4[4];
	byte end;
};

filter filter2;
menu_text_struct menu_text_area1;

int read=0;


proc_info pr_inf;
char communication_area_name[] = "FFFFFFFF_open_dialog";
byte plugin_path[4096];
char default_dir[] = "/rd/1";
char open_dialog_path[] = "/rd/1/File managers/opendial"; //opendial
byte openfile_path[2048];
byte filename_area[4096];

opendialog o_dialog = {0, #pr_inf, #communication_area_name, 0, #plugin_path, #default_dir, #open_dialog_path, #draw_window, 0, #openfile_path, #filename_area, #filter2, 420, 200, 320, 120};

dword bufpointer;
dword bufsize;
dword draw_sruct;

menu_data menudata1 = {0, 40, 2, 15, 2, #menu_text_area1.menu, #menu_text_area1.sub_menu1, #menu_text_area1.end, 0, 0, 80, 2, 100, 18, 0xEEEEEE, 0xFF, 0xEEEEEE, 0, 0, 0, #Form, 0, 0, 0, 16, 0, 0, 0x00CC00, 0, 0xFFFFFF, 0, 8, 0, 0};

void main()
{   
	int id;
	
	strcpy(#filter2.ext1, "TXT");
	//strcpy(#filter2.ext2, "ASM");
	//strcpy(#filter2.ext3, "INC\0");
	//strcpy(#filter2.ext4, "\0");
	filter2.size = 8;
	filter2.end = 0;

	strcpy(#menu_text_area1.menu, MENU1);
	strcpy(#menu_text_area1.sub_menu1, MENU1_SUBMENU1);
	strcpy(#menu_text_area1.sub_menu2, MENU1_SUBMENU2);
	//strcpy(#menu_text_area1.sub_menu3, MENU1_SUBMENU3);
	strcpy(#menu_text_area1.sub_menu4, MENU1_SUBMENU4);
	menu_text_area1.end = 0;
	
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libio, #libio_init,1);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
	SetEventMask(0x27);
	loop()
	{
      switch(WaitEvent())
      {
		case evMouse:
			mouse.get();
			if (tview.MouseScroll(mouse.vert)) DrawText();
		
			menu_bar_mouse stdcall (#menudata1);			
			if (menudata1.click)
			{
				switch(menudata1.cursor_out)
				{
					case 1:
						OpenDialog_start stdcall (#o_dialog);
						OpenFile(#openfile_path);
						Prepare();
						draw_window();
						break;
					case 2:
						read = 0;
						tview.cur_y = 0;
						strcpy(#win_title, TITLE);
						FreeBuf();
						draw_window();
						break;
					case 3:
						if (!active_properties) 
						{
							SwitchToAnotherThread();
							properties_window = CreateThread(#properties_dialog, #properties_stak+4092);
							break;
						}
						else
						{
							ActivateWindow(GetProcessSlot(properties_window));
						}
						break;
					case 4:
						ExitProcess();
				}
			}
			break;
		
        case evButton:
            id=GetButtonID();               
            if (id==1) ExitProcess();
			break;
      
        case evKey:
			if (Form.status_window>2) break;
			GetKeys();
			if (tview.ProcessKey(key_scancode)) DrawText();
			if (TestBit(key_modifier, 2))
			{
				switch(key_scancode)
				{
					case 024:  //Ctrl+O
						OpenDialog_start stdcall (#o_dialog);
						OpenFile(#openfile_path);
						Prepare();
						draw_window();
						break;
				}
				break;
			}
			switch (key_scancode)
			{
				/*case SCAN_CODE_HOME:
				case SCAN_CODE_END:
				case SCAN_CODE_UP:
				case SCAN_CODE_DOWN:
					if (tview.ProcessKey(key_scancode)) DrawText();
					break;*/
				case SCAN_CODE_PGUP:
					if (!tview.cur_y) break;
					if (tview.cur_y<tview.visible) tview.cur_y = 0;
					else tview.cur_y = tview.cur_y-tview.visible;
					DrawText();
					break;
				case SCAN_CODE_PGDN:
					if (tview.cur_y+tview.visible>tview.count) break;
					tview.cur_y = tview.cur_y+tview.visible;
					DrawText();
					break;
			}
			break;
         
         case evReDraw:
			draw_window();
			break;
      }
   }
}


void draw_window()
{
	system.color.get();
	DefineAndDrawWindow(GetScreenWidth()-WIN_W/2,GetScreenHeight()-WIN_H/2,WIN_W,WIN_H,0x73,0xFFFFFF,#win_title);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	tview.SetSizes(0, TOPPANELH, Form.cwidth, Form.cheight-BOTPANELH-TOPPANELH, 12);
	DrawBar(0, 0, Form.cwidth, TOPPANELH, system.color.work);
	DrawBar(0, Form.cheight-BOTPANELH, Form.cwidth, BOTPANELH, system.color.work);
	
	menudata1.bckg_col = system.color.work;
	menu_bar_draw stdcall (#menudata1);
	
	if (old_width!=Form.width) || (old_height!=Form.height)
	{
		old_width = Form.width;
		old_height = Form.height;
		if (read==1) Prepare();
		tview.debug_values();
	}
	if (read==1) 
	{
		DrawText();
	}
	else DrawBar(tview.x, tview.y, tview.w, tview.h, 0xFFFFFF);
}

void OpenFile(dword path)
{
	strcpy(#win_title, TITLE);
	strcat(#win_title, " - ");
	strcat(#win_title, path);
	file_size stdcall (path);
	bufsize = EBX;
	if (bufsize)
	{
		mem_Free(bufpointer);
		bufpointer = mem_Alloc(bufsize);
		ReadFile(0, bufsize, bufpointer, path);
		read=1;
	}
}

void FreeBuf()
{
	int i;
	for (i=0; i<tview.count; i++)
	{
		mem_Free(DSDWORD[i*4+draw_sruct]);
	}
	mem_Free(draw_sruct);
	mem_Free(bufpointer);
}

void Prepare()
{
	int i, sub_pos;
	static int cur_pos;
	int len_str = 0;
	byte do_eof = 0;
	word bukva[2];
	dword address;
	tview.count = 0;
	while(1)
	{
		while(1)
		{
			bukva = DSBYTE[bufpointer+cur_pos+len_str];
			if (bukva=='\0')
			{
				do_eof = 1;
				break;
			}
			if (bukva==0x0a) break;
			else len_str++;
		}
		if (len_str<=tview.column_max)
		{
			cur_pos=cur_pos+len_str+1;
			tview.count++;
		}
		else
		{
			cur_pos=cur_pos+tview.column_max;
			tview.count++;
		}
		len_str = 0;
		if (do_eof) break;
	}
	mem_Free(draw_sruct);
	draw_sruct = mem_Alloc(tview.count*4);
	cur_pos=0;
	sub_pos=0;
	len_str = 0;
	do_eof = 0;
	while(1)
	{
		while(1)
		{
			bukva = DSBYTE[bufpointer+cur_pos+len_str];
			if (bukva=='\0')
			{
				do_eof = 1;
				break;
			}
			if (bukva==0x0a) break;
			else len_str++;
		}
		if (len_str<=tview.column_max)
		{
			address = mem_Alloc(len_str+1);
			ESDWORD[sub_pos*4+draw_sruct] = address;
			strlcpy(DSDWORD[sub_pos*4+draw_sruct], bufpointer+cur_pos, len_str);
			cur_pos=cur_pos+len_str+1;
			sub_pos++;
		}
		else
		{
			address = mem_Alloc(len_str+1);
			ESDWORD[sub_pos*4+draw_sruct] = address;
			strlcpy(DSDWORD[sub_pos*4+draw_sruct], bufpointer+cur_pos, tview.column_max);
			cur_pos=cur_pos+tview.column_max;
			sub_pos++;
		}
		len_str = 0;
		if (cur_pos>=bufsize-1) break;
	}
	cur_pos=0;
}

void DrawText()
{
	int i, top, num_line;
	if (tview.count<tview.visible) top = tview.count;
	else
	{
		if (tview.count-tview.cur_y<=tview.visible) top = tview.count-tview.cur_y-1;
		else top = tview.visible;
	}
	DrawBar(tview.x, tview.y, tview.w, 3, 0xFFFFFF);
	for (i=0, num_line = tview.cur_y; i<top; i++, num_line++)
	{
		DrawBar(tview.x, i * tview.item_h + tview.y + 3, tview.w, tview.item_h, 0xFFFFFF);
		WriteText(tview.x + 2, i * tview.item_h + tview.y + 3, 0x80, 0x000000, DSDWORD[num_line*4+draw_sruct]);
	}
	DrawBar(0, i * tview.item_h + tview.y + 3, tview.w, -i* tview.item_h + tview.h, 0xFFFFFF);
}

stop:
char properties_stak[4096];