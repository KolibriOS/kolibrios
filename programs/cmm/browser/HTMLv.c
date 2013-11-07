//HTML Viewer in C--
//Copyright 2007-2013 by Veliant & Leency
//Asper, lev, Lrz, Barsuk, Nable...
//home icon - rachel fu, GPL licence

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

//libraries
#define MEMSIZE 0x100000
#include "..\lib\kolibri.h"
#include "..\lib\strings.h"
#include "..\lib\figures.h"
#include "..\lib\encoding.h"
#include "..\lib\file_system.h"
#include "..\lib\mem.h"
#include "..\lib\dll.h"
//*.obj libraries
#include "..\lib\lib.obj\box_lib.h"
#include "..\lib\lib.obj\libio_lib.h"
#include "..\lib\lib.obj\libimg_lib.h"
#include "..\lib\list_box.h"
//images
#include "img\toolbar_icons.c"
#include "img\URLgoto.txt";

#ifdef LANG_RUS
	char version[]=" Т•™бвЃҐл© °а†гІ•а 0.99.1";
	?define IMAGES_CACHE_CLEARED "Кни ™†ав®≠Ѓ™ Ѓз®й•≠"
#else
	char version[]=" Text-based Browser 0.99.1";
	?define IMAGES_CACHE_CLEARED "Images cache cleared"
#endif


#define URL param
char fontlol[64];

char editURL[sizeof(URL)],
	page_links[12000],
	header[2048];

struct lines{
	int visible, all, first, column_max;
};

int	mouse_dd;
edit_box address_box= {250,207,16,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(editURL),#editURL,#mouse_dd,2,19,19};
scroll_bar scroll1 = { 18,200,398, 44,18,0,115,15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};


proc_info Form;
#define WIN_W 640
#define WIN_H 480


char stak[4096];
mouse m;

int action_buf;

#include "TWB.h"
#include "include\menu_rmb.h"


void main()
{
	int key, btn;
	int half_scroll_size;
	int scroll_used=0, show_menu;
	
	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0) {notify("System Error: library doesn't exists /rd/1/lib/box_lib.obj"); ExitProcess();}
	if (load_dll2(libio, #libio_init,1)!=0) debug("Error: library doesn't exists - libio"w);
	if (load_dll2(libimg, #libimg_init,1)!=0) debug("Error: library doesn't exists - libimg"w);
	
	if (!URL) strcpy(#URL, "/sys/index.htm");
	strcpy(#editURL, #URL);
	
	Form.width=WIN_W;
	Form.height=WIN_H;
	SetElementSizes();
	WB1.OpenPage();

	SetEventMask(0x27);
	loop()
	{
		WaitEventTimeout(2);
		switch(EAX & 0xFF)
		{
			CASE evMouse:
				/*scrollbar_v_mouse (#scroll1);      //конченый скролл притормажимает, идЄм "своим путЄм"
				if (lines.first <> scroll1.position)
				{
					lines.first = scroll1.position;
					WB1.ParseHTML(buf, filesize);
					//break;
				};*/
				
				if (!CheckActiveProcess(Form.ID)) break;

				edit_box_mouse stdcall (#address_box);

				m.get();
				
				if (m.y>WB1.top) && (m.y<Form.height) && (filesize)
				{
					if (m.pkm)
					{
						show_menu = 1;
					}
					if (!m.pkm) && (show_menu)
					{
						show_menu = 0;
						SwitchToAnotherThread();
						CreateThread(#menu_rmb,#stak+4092);
						break; 
					}
				}

				if (m.vert==65535)
				{
					if (lines.first==0) break;
					if (lines.first>3) lines.first-=2; ELSE lines.first=1;
					WB1.Scan(ID1);
					break;
				} 
				if (m.vert==1)
				{
					if(lines.visible+lines.first+3>=lines.all) WB1.Scan(181);
					else
					{
						lines.first+=2;
						WB1.Scan(ID2);
					}
					break;
				}
				
				if (!m.lkm) scroll_used=0;
				if (m.x>=scroll1.start_x) && (m.x<=scroll1.start_x+scroll1.size_x) 
				&& (m.y>=scroll1.start_y+scroll1.btn_height) && (-scroll1.btn_height+scroll1.start_y+scroll1.size_y>m.y)
				&& (lines.all>lines.visible) && (m.lkm)
				{
					scroll_used=1;
				}
				
				if (scroll_used)
				{
					half_scroll_size = WB1.height - 16 * lines.visible / lines.all - 3 /2;
					if (half_scroll_size+WB1.top>m.y) || (m.y<0) || (m.y>4000) m.y=half_scroll_size+WB1.top; //если курсор над окном
					btn=lines.first; //сохран€ем старое количество
					lines.first = m.y -half_scroll_size -WB1.top * lines.all / WB1.height;
					if (lines.visible+lines.first>lines.all) lines.first=lines.all-lines.visible;
					if (btn<>lines.first) WB1.ParseHTML(buf); //чтоб лишний раз не перерисовывать
				}

				break;
			case evButton:
				btn=GetButtonID();
				if (btn==1)
				{
					KillProcess(downloader_id);
					ExitProcess();
				}
				ELSE
				{
					WB1.Scan(btn);
				}
				break;
			case evKey:
				key = GetKey();
				
				if (address_box.flags & 0b10) SWITCH(key) //если активна строка адреса игнорируем некоторые кнопки
					{ CASE 52: CASE 53: CASE 54: goto _EDIT_MARK; } 

				WB1.Scan(key);
				
				_EDIT_MARK:
				if (key<>0x0d) && (key<>183) && (key<>184) {EAX=key<<8; edit_box_key stdcall(#address_box);} //адресна€ строка
				break;
			case evReDraw:
				if (action_buf) { WB1.Scan(action_buf); action_buf=0;}
				Draw_Window();
				break;
			default:
				if (downloader_id<>0)
				{
					if (GetProcessSlot(downloader_id)<>0) break;
					downloader_id=0;
					lines.first = lines.all = 0;
					WB1.ReadHtml(_WIN);
					Draw_Window();
				}
		}
	}
}

void SetElementSizes()
{
	address_box.width = Form.width-266;
	WB1.top = 44;
	WB1.width = Form.width - 10 - scroll1.size_x;
	WB1.height = Form.height - WB1.top - GetSkinHeight() - 4;
	WB1.line_h = 10;
	lines.column_max = WB1.width - 30 / 6;
	lines.visible = WB1.height - 3 / WB1.line_h - 2;
	DrawBufInit();
}


void Draw_Window()
{
	int j;
	DefineAndDrawWindow(215,100,WIN_W,WIN_H,0x73,0xE4DFE1,0,0);

	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2)
	{
		DrawTitle(#header);
		return;
	}
	if (Form.height<120) MoveSize(OLD,OLD,OLD,120);
	if (Form.width<280) MoveSize(OLD,OLD,280,OLD);
	
	PutPaletteImage(#toolbar,200,42,0,0,8,#toolbar_pal);
	if (GetProcessSlot(downloader_id)<>0) _PutImage(88,10, 24,24, #stop_btn);
	
	DrawBar(200,0,Form.cwidth-200,43,0xE4DFE1);
	DrawBar(0,42,Form.cwidth,1,0xE2DBDC);
	DrawBar(0,43,Form.cwidth,1,0xD2CED0);
	for (j=0; j<5; j++) DefineButton(j*37+11, 7, 29, 29, 300+j+BT_HIDE, 0xE4DFE1);
	_PutImage(Form.cwidth-48,14, 40,19, #URLgoto);
	DefineButton(Form.cwidth-28,15, 18, 16, GOTOURL+BT_HIDE, 0xE4DFE1);
	DefineButton(Form.cwidth-47,15, 17, 16, SEARCHWEB+BT_HIDE, 0xE4DFE1);
	DrawRectangle(205,14,Form.cwidth-205-49,18,0x94AECE); //around adress bar
	DrawRectangle(206,15,Form.cwidth-205-50,16,0xE4ECF3);

	SetElementSizes();
	WB1.ShowPage();

	DefineButton(scroll1.start_x+1, scroll1.start_y+1, 16, 16, ID1+BT_HIDE, 0xE4DFE1);
	DefineButton(scroll1.start_x+1, scroll1.start_y+scroll1.size_y-18, 16, 16, ID2+BT_HIDE, 0xE4DFE1);
}


stop:
