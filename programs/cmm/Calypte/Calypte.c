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
#include "..\lib\obj\iconv.h"
#include "..\lib\obj\box_lib.h"
#include "..\lib\obj\proc_lib.h"
#include "..\lib\obj\libio_lib.h"
//#include "..\lib\obj\wword.h"

#ifdef LANG_RUS
	?define T_FILE "Файл"
	?define T_TYPE "Тип"
	?define T_SIZE "Размер"

	?define MENU1 "Файл"
	?define MENU1_SUBMENU1 "Открыть"
	?define MENU1_SUBMENU2 "Закрыть"
	?define MENU1_SUBMENU3 "Свойства"
	?define MENU1_SUBMENU4 "Выход"

	?define MENU2 "Кодировка"
	?define MENU2_SUBMENU1 "UTF-8"
	?define MENU2_SUBMENU2 "KOI8-RU"
	?define MENU2_SUBMENU3 "CP1251"
	?define MENU2_SUBMENU4 "CP1252"
	?define MENU2_SUBMENU5 "ISO8859-5"
	?define MENU2_SUBMENU6 "CP866"

	?define ERROR_LOAD_BOX_LIB "Ошибка при загрузке библиотеки - box_lib.obj"
	?define ERROR_LOAD_LIBIO "Ошибка при загрузке библиотеки - libio.obj"
	?define ERROR_LOAD_PROC_LIB "Ошибка при загрузке библиотеки - proc_lib.obj"
#else
	?define T_FILE "File"
	?define T_TYPE "Type"
	?define T_SIZE "Size"
	?define MENU1 "File"

	?define MENU1_SUBMENU1 "Open"
	?define MENU1_SUBMENU2 "Close"
	?define MENU1_SUBMENU3 "Properties"
	?define MENU1_SUBMENU4 "Exit"

	?define MENU2 "Encoding"
	?define MENU2_SUBMENU1 "UTF-8"
	?define MENU2_SUBMENU2 "KOI8-RU"
	?define MENU2_SUBMENU3 "CP1251"
	?define MENU2_SUBMENU4 "CP1252"
	?define MENU2_SUBMENU5 "ISO8859-5"
	?define MENU2_SUBMENU6 "CP866"

	?define ERROR_LOAD_BOX_LIB "Error while loading library - box_lib.obj"
	?define ERROR_LOAD_LIBIO "Error while loading library - libio.obj"
	?define ERROR_LOAD_PROC_LIB "Error while loading library - proc_lib.obj"
#endif

#ifdef LANG_RUS
struct menu1_text_struct
{
	char menu[5];
	char sub_menu1[8];
	char sub_menu2[8];
	//char sub_menu3[9];
	char sub_menu4[6];
	byte end;
};
struct menu2_text_struct
{
	char menu[10];
	char sub_menu1[6];
	char sub_menu2[8];
	char sub_menu3[7];
	char sub_menu4[7];
	char sub_menu5[10];
	char sub_menu6[6];
	byte end;
};
#else
struct menu1_text_struct
{
	char menu[5];
	char sub_menu1[5];
	char sub_menu2[6];
	//char sub_menu3[11];
	char sub_menu4[5];
	byte end;
};
struct menu2_text_struct
{
	char menu[9];
	char sub_menu1[6];
	char sub_menu2[8];
	char sub_menu3[7];
	char sub_menu4[7];
	char sub_menu5[10];
	char sub_menu6[6];
	byte end;
};
#endif

#define TITLE "Calypte v0.15"

#define TOPPANELH 19
#define BOTPANELH 10
#define WIN_W 600
#define WIN_H 400

proc_info Form;

byte active_properties = 0;
int encoding;
dword properties_window;

#include "include\gui.h"
#include "include\properties.h"

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
menu1_text_struct menu1_text_area1;
menu2_text_struct menu2_text_area2;

char win_title[4096] = "Calypte v0.15";

int
	cur_row=0,
	read=0,
	pos=0,
	row_num=0,
	col_count=0,
	row_count=0;	
	
dword old_width,old_height;

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

menu_data menudata1 = {0, 40, 2, 15, 2, #menu1_text_area1.menu, #menu1_text_area1.sub_menu1, #menu1_text_area1.end, 0, 0, 80, 2, 100, 18, 0xEEEEEE, 0xFF, 0xEEEEEE, 0, 0, 0, #Form, 0, 0, 0, 16, 0, 0, 0x00CC00, 0, 0xFFFFFF, 0, 8, 0, 0};
menu_data menudata2 = {0, 70, 44, 15, 2, #menu2_text_area2.menu, #menu2_text_area2.sub_menu1, #menu2_text_area2.end, 0, 0, 80, 44, 100, 18, 0xEEEEEE, 0xFF, 0xEEEEEE, 0, 0, 0, #Form, 0, 0, 0, 16, 0, 0, 0x00CC00, 0, 0xFFFFFF, 0, 8, 0, 0};

void main()
{   
	int id;
	
	strcpy(#filter2.ext1, "TXT");
	//strcpy(#filter2.ext2, "ASM");
	//strcpy(#filter2.ext3, "INC\0");
	//strcpy(#filter2.ext4, "\0");
	filter2.size = 8;
	filter2.end = 0;

	strcpy(#menu1_text_area1.menu, MENU1);
	strcpy(#menu1_text_area1.sub_menu1, MENU1_SUBMENU1);
	strcpy(#menu1_text_area1.sub_menu2, MENU1_SUBMENU2);
	//strcpy(#menu1_text_area1.sub_menu3, MENU1_SUBMENU3);
	strcpy(#menu1_text_area1.sub_menu4, MENU1_SUBMENU4);
	menu1_text_area1.end = 0;
	
	strcpy(#menu2_text_area2.menu, MENU2);
	strcpy(#menu2_text_area2.sub_menu1, MENU2_SUBMENU1);
	strcpy(#menu2_text_area2.sub_menu2, MENU2_SUBMENU2);
	strcpy(#menu2_text_area2.sub_menu3, MENU2_SUBMENU3);
	strcpy(#menu2_text_area2.sub_menu4, MENU2_SUBMENU4);
	strcpy(#menu2_text_area2.sub_menu5, MENU2_SUBMENU5);
	strcpy(#menu2_text_area2.sub_menu6, MENU2_SUBMENU6);
	menu2_text_area2.end = 0;
	
	//mem_Init();
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libio, #libio_init,1);
	load_dll(iconv_lib, #iconv_open,0);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
	
	SetEventMask(0x27);
	loop()
	{
      switch(WaitEvent())
      {
		case evMouse:
			menu_bar_mouse stdcall (#menudata1);
			menu_bar_mouse stdcall (#menudata2);
			if (menudata1.click==1)
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
			if (menudata2.click==1)
			{
				encoding = menudata2.cursor_out - 1;
				OpenFile(#openfile_path);
				Prepare();
				draw_window();
			}
			break;
		
        case evButton:
			id=GetButtonID();               
            if (id==1) || (id==10) ExitProcess();
			break;
		
        case evKey:
            GetKeys();
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
				if (Form.status_window>2) break;
				case SCAN_CODE_HOME:
					cur_row = 0;
					DrawText();
					break;
				case SCAN_CODE_END:
					cur_row = row_num - row_count - 1;
					DrawText();
					break;
				case SCAN_CODE_UP:
					if (!cur_row) break;
					else cur_row = cur_row-1;
					DrawText();
					break;
				case SCAN_CODE_DOWN:
					if (cur_row+row_count>=row_num) break;
					cur_row = cur_row+1;
					DrawText();
					break;
				case SCAN_CODE_PGUP:
					if (!cur_row) break;
					if (cur_row<row_count) cur_row = 0;
					else cur_row = cur_row-row_count;
					DrawText();
					break;
				case SCAN_CODE_PGDN:
					if (cur_row+row_count>row_num) break;
					cur_row = cur_row+row_count;
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
	DrawBar(0, 0, Form.cwidth, TOPPANELH, system.color.work);
	DrawBar(0, Form.cheight-BOTPANELH, Form.cwidth, BOTPANELH, system.color.work);
	
	menudata1.bckg_col = system.color.work;
	menudata2.bckg_col = system.color.work;
	menu_bar_draw stdcall (#menudata1);
	menu_bar_draw stdcall (#menudata2);
	
	if (old_width!=Form.width) || (old_height!=Form.height)
	{
		old_width = Form.width;
		old_height = Form.height;
		
		col_count = Form.cwidth/6;
		row_count = Form.cheight-BOTPANELH-TOPPANELH-2;
		row_count = row_count/10;
	
		if (read==1) Prepare();
	}
	if (read==1) 
	{
		DrawText();
	}
	else DrawBar(0, TOPPANELH, Form.cwidth, Form.cheight-BOTPANELH-TOPPANELH, 0xFFFFFF);
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
	if (encoding!=CH_CP866) ChangeCharset(charsets[encoding], "CP866", bufpointer);
}

void FreeBuf()
{
	int i;
	for (i=0; i<row_num; i++)
	{
		mem_Free(DSDWORD[i*4+draw_sruct]);
	}
	mem_Free(draw_sruct);
	mem_Free(bufpointer);
}

void Prepare()
{
	int i, sub_pos;
	int len_str = 0;
	byte do_eof = 0;
	word bukva[2];
	dword address;
	row_num = 0;
	while(1)
	{
		while(1)
		{
			bukva = DSBYTE[bufpointer+pos+len_str];
			if (bukva=='\0')
			{
				do_eof = 1;
				break;
			}
			if (bukva==0x0a) break;
			else len_str++;
		}
		if (len_str<=col_count)
		{
			pos=pos+len_str+1;
			row_num++;
		}
		else
		{
			pos=pos+col_count;
			row_num++;
		}
		len_str = 0;
		if (do_eof) break;
	}
	mem_Free(draw_sruct);
	draw_sruct = mem_Alloc(row_num*4);
	pos=0;
	sub_pos=0;
	len_str = 0;
	do_eof = 0;
	while(1)
	{
		while(1)
		{
			bukva = DSBYTE[bufpointer+pos+len_str];
			if (bukva=='\0')
			{
				do_eof = 1;
				break;
			}
			if (bukva==0x0a) break;
			else len_str++;
		}
		if (len_str<=col_count)
		{
			address = mem_Alloc(len_str+1);
			ESDWORD[sub_pos*4+draw_sruct] = address;
			strlcpy(DSDWORD[sub_pos*4+draw_sruct], bufpointer+pos, len_str);
			pos=pos+len_str+1;
			sub_pos++;
		}
		else
		{
			address = mem_Alloc(len_str+1);
			ESDWORD[sub_pos*4+draw_sruct] = address;
			strlcpy(DSDWORD[sub_pos*4+draw_sruct], bufpointer+pos, col_count);
			pos=pos+col_count;
			sub_pos++;
		}
		len_str = 0;
		if (pos>=bufsize-1) break;
	}
	pos=0;
}

void DrawText()
{
	int i, top, num_line;
	if (row_num<row_count) top = row_num;
	else
	{
		if (row_num-cur_row<=row_count) top = row_num-cur_row-1;
		else top = row_count;
	}
	DrawBar(0, TOPPANELH, Form.cwidth, 3, 0xFFFFFF);
	for (i=0, num_line = cur_row; i<top; i++, num_line++)
	{
		DrawBar(0, i*10+TOPPANELH+3, Form.cwidth, 10, 0xFFFFFF);
		WriteText(2, i*10+TOPPANELH+3, 0x80, 0x000000, DSDWORD[num_line*4+draw_sruct]);
	}
	DrawBar(0, i*10+TOPPANELH+3, Form.cwidth, -i*10-TOPPANELH-BOTPANELH+Form.cheight, 0xFFFFFF);
}

stop:
char properties_stak[4096];