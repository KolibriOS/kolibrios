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
#include "..\lib\draw_buf.h"
#include "..\lib\list_box.h"
#include "..\lib\cursor.h"

//*.obj libraries
#include "..\lib\lib.obj\box_lib.h"
#include "..\lib\lib.obj\libio_lib.h"
#include "..\lib\lib.obj\libimg_lib.h"
#include "..\lib\lib.obj\http.h"
//images
#include "img\toolbar_icons.c"
#include "img\URLgoto.txt";

#ifdef LANG_RUS
	char version[]=" Текстовый браузер 0.99.65";
	?define IMAGES_CACHE_CLEARED "Кэш картинок очищен"
	?define T_LAST_SLIDE "Это последний слайд"
	char loading[] = "Loading...";
	char page_not_found[] = "Страница не найдена. Воможно, URL содержит ошибку.";
	char page_not_found_no_internet[] = "Страница не найдена. Воможно, URL содержит ошибку.<br>Или нет доступа в Интернеты.";
#else
	char version[]=" Text-based Browser 0.99.65";
	?define IMAGES_CACHE_CLEARED "Images cache cleared"
	?define T_LAST_SLIDE "This slide is the last"
	char loading[] = "Loading...";
	char page_not_found[] = "Page not found. Maybe, URL contains some errors.";
	char page_not_found_no_internet[] = "Page not found. Maybe, URL contains some errors.<br>Or Internet unavailable for your configuration.";
#endif

proc_info Form;
#define WIN_W 640
#define WIN_H 480

char search_path[]="http://nigma.ru/index.php?s=";

char stak[4096];
mouse m;
int action_buf;

dword http_transfer = 0;
dword http_buffer;

#include "..\TWB\TWB.c"
#include "menu_rmb.h"

char editURL[sizeof(URL)];
int	mouse_twb;
edit_box address_box= {250,207,16,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(URL),#editURL,#mouse_twb,2,19,19};


enum { BACK=300, FORWARD, REFRESH, HOME, NEWTAB, GOTOURL, SEARCHWEB, INPUT_CH, INPUT_BT, BTN_UP, BTN_DOWN };


void main()
{
	int key, btn;
	int half_scroll_size;
	int scroll_used=0, show_menu;
	
	mem_Init();
	CursorPointer.Load(#CursorFile);
	if (load_dll2(boxlib, #box_lib_init,0)!=0) {notify("System Error: library doesn't exists /rd/1/lib/box_lib.obj"); ExitProcess();}
	if (load_dll2(libio, #libio_init,1)!=0) notify("Error: library doesn't exists - libio");
	if (load_dll2(libimg, #libimg_init,1)!=0) notify("Error: library doesn't exists - libimg");
	
	if (!URL) strcpy(#URL, "/sys/index.htm");
	Form.width=WIN_W;
	Form.height=WIN_H;
	SetElementSizes();
	OpenPage();

	SetEventMask(0xa7);
	loop()
	{
		WaitEventTimeout(2);
		switch(EAX & 0xFF)
		{
			CASE evMouse:
				if (!CheckActiveProcess(Form.ID)) break;

				edit_box_mouse stdcall (#address_box);

				m.get();
				PageLinks.Hover(m.x, m.y, link_color_inactive, link_color_active, bg_color);
				
				if (m.y>WB1.list.y) && (m.y<Form.height) && (filesize)
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

				if (m.vert)
				{
					if (WB1.list.MouseScroll(m.vert)) WB1.ParseHTML(buf);
				}
				
				if (!m.lkm) scroll_used=0;
				if (m.x>=scroll_wv.start_x) && (m.x<=scroll_wv.start_x+scroll_wv.size_x) 
				&& (m.y>=scroll_wv.start_y+scroll_wv.btn_height) && (-scroll_wv.btn_height+scroll_wv.start_y+scroll_wv.size_y>m.y)
				&& (WB1.list.count>WB1.list.visible) && (m.lkm)
				{
					scroll_used=1;
				}
				
				if (scroll_used)
				{
					half_scroll_size = WB1.list.h - 16 * WB1.list.visible / WB1.list.count - 3 /2;
					if (half_scroll_size+WB1.list.y>m.y) || (m.y<0) || (m.y>4000) m.y=half_scroll_size+WB1.list.y;
					btn=WB1.list.first;
					WB1.list.first = m.y -half_scroll_size -WB1.list.y * WB1.list.count / WB1.list.h;
					if (WB1.list.visible+WB1.list.first>WB1.list.count) WB1.list.first=WB1.list.count-WB1.list.visible;
					if (btn<>WB1.list.first) WB1.ParseHTML(buf);
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
					Scan(btn);
				}
				break;
			case evKey:
				key = GetKey();
				
				if (address_box.flags & 0b10) SWITCH(key)
					{ CASE 52: CASE 53: CASE 54: goto _EDIT_MARK; } 

				Scan(key);
				
				_EDIT_MARK:
				if (key<>0x0d) && (key<>183) && (key<>184) {EAX=key<<8; edit_box_key stdcall(#address_box);}
				break;
			case evReDraw:
				if (action_buf) { Scan(action_buf); action_buf=0;}
				Draw_Window();
				break;
			case evNetwork:
				return;
				//open page
				http_get stdcall (#URL, 0);	
				http_transfer = EAX;
				IF (http_transfer<0) notify("Error from HTTP lib");
				//
				if (http_transfer != 0) {
					http_process stdcall (http_transfer);
					$push EAX
					ESI = http_transfer;
					if (!ESI.http_msg.content_received) break;
					buf = ESI.http_msg.content_ptr;
					debug(buf);	
					filesize = ESI.http_msg.content_received;
					debugi(filesize);
					WB1.ParseHTML(buf);							
					$pop EAX	
					if (EAX == 0) {			
						http_free stdcall (http_transfer);
						http_transfer=0;				
					}
				}
			default:
				if (downloader_id<>0)
				{
					if (GetProcessSlot(downloader_id)<>0) break;
					downloader_id=0;
					WB1.list.first = WB1.list.count = 0;
					WB1.ReadHtml(_WIN);
					Draw_Window();
				}
		}
	}
}

void SetElementSizes()
{
	address_box.width = Form.width - 266;
	WB1.list.SetSizes(0, 44, Form.width - 10 - scroll_wv.size_x, Form.cheight - 44, 0, 10);
	WB1.list.column_max = WB1.list.w - 30 / 6;
	WB1.list.visible = WB1.list.h - 3 / WB1.list.line_h - 2;
	WB1.DrawBuf.Init(WB1.list.x, WB1.list.y, WB1.list.w, WB1.list.h, WB1.list.line_h);
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
	ShowPage();

	DefineButton(scroll_wv.start_x+1, scroll_wv.start_y+1, 16, 16, BTN_UP+BT_HIDE, 0xE4DFE1);
	DefineButton(scroll_wv.start_x+1, scroll_wv.start_y+scroll_wv.size_y-18, 16, 16, BTN_DOWN+BT_HIDE, 0xE4DFE1);
}


void Scan(int id)
{
	if (id >= 400) ProcessLinks(id);
	
	switch (id)
	{
		case 011: //Ctrk+K 
			WB1.ReadHtml(_KOI);
			WB1.ParseHTML(buf);
			return;

		case 021: //Ctrl+U
			WB1.ReadHtml(_UTF);
			WB1.ParseHTML(buf);
			return;

		case 004: //Ctrl+D
			WB1.ReadHtml(_DOS);
			WB1.ParseHTML(buf);
			return;

		case 005: //Win encoding
			WB1.ReadHtml(_WIN);
			WB1.ParseHTML(buf);
			return;

		case 009: //free img cache
			ImgCache.Free();
			notify(IMAGES_CACHE_CLEARED);
			WB1.ParseHTML(buf);
			return;

		case BACK:
			if (!BrowserHistory.GoBack()) return;
			OpenPage();
			return;
		case FORWARD:
			if (!BrowserHistory.GoForward()) return;
			OpenPage();
			return;
		case 052:  //F3
			if (strncmp(#URL,"http:",5)<>0) RunProgram("/rd/1/tinypad", #URL);
			else RunProgram("/rd/1/tinypad", #download_path);
			return;
		case 054: //F5
			IF(address_box.flags & 0b10) WB1.ParseHTML(buf);
			return;

		case REFRESH:
			if (GetProcessSlot(downloader_id)<>0)
			{
				KillProcess(downloader_id);
				pause(20);
				Draw_Window();
				return;
			}
			anchor_line_num=WB1.list.first;
			anchor[0]='|';
			OpenPage();
			return;
		case 014:
		case 020:
		case NEWTAB:
			MoveSize(190,80,OLD,OLD);
			RunProgram(#program_path, #URL);
			return;
			
		case HOME:
			strcpy(#editURL, "http://kolibrios.org/en/index.htm");
		case GOTOURL:
		case 0x0D: //enter
			if ((strstr(#editURL,"ttp://")==0) && (editURL[0]!='/')) strcpy(#URL,"http://"); else URL[0] = 0;
			strcat(#URL, #editURL);
			OpenPage();
			return;
		case SEARCHWEB:
			strcpy(#URL, #search_path);
			strcat(#URL, #editURL);
			OpenPage();
			return;

		case 183: //PgDown
			if (WB1.list.count < WB1.list.visible) return;
			IF(WB1.list.first == WB1.list.count - WB1.list.visible) return;
			WB1.list.first += WB1.list.visible + 2;
			IF(WB1.list.visible + WB1.list.first > WB1.list.count) WB1.list.first = WB1.list.count - WB1.list.visible;
			WB1.ParseHTML(buf);
			return;

		case 184: //PgUp
			if (WB1.list.count < WB1.list.visible) return;
			IF(WB1.list.first == 0) return;
			WB1.list.first -= WB1.list.visible - 2;
			IF(WB1.list.first < 0) WB1.list.first = 0;
			WB1.ParseHTML(buf);
			return;

		case 178:
		case BTN_UP: //ьюЄрхь ттхЁї
			if (WB1.list.first <= 0) return;
			WB1.list.first--;
			WB1.ParseHTML(buf);
			return;

		case 177: 
		case BTN_DOWN: //ьюЄрхь тэшч
			if (WB1.list.visible + WB1.list.first >= WB1.list.count) return;
			WB1.list.first++;
			WB1.ParseHTML(buf);
			return;

		case 180: //home
			if (WB1.list.KeyHome()) WB1.ParseHTML(buf);
			return; 

		case 181: //end
			if (WB1.list.count < WB1.list.visible) return;
			if (WB1.list.KeyEnd()) WB1.ParseHTML(buf);
			return;
	}
}



void ProcessLinks(int id)
{
	strcpy(#URL, PageLinks.GetURL(id-401));
	
	//$1 - Condition Script
	if (URL[0] == '$')
	{
		if (URL[1]=='-') && (condition_href) condition_href--;
		if (URL[1]=='+') 
		{
			if (condition_href<condition_max) condition_href++; else notify(T_LAST_SLIDE);
		}
		if (URL[1]!='-') && (URL[1]!='+') condition_href = atoi(#URL+1);
		strcpy(#URL, BrowserHistory.CurrentUrl());
		ShowPage();
		return;
	}
	//#1
	if (URL[0] == '#')
	{
		strcpy(#anchor, #URL+strrchr(#URL, '#'));		
		strcpy(#URL, BrowserHistory.CurrentUrl());
		WB1.list.first=WB1.list.count-WB1.list.visible;
		ShowPage();
		return;
	}
	//liner.ru#1
	if (strrchr(#URL, '#')!=-1)
	{
		strcpy(#anchor, #URL+strrchr(#URL, '#'));
		URL[strrchr(#URL, '#')-1] = 0x00;
	}
	
	WB1.GetNewUrl();
	
	if (!strcmp(#URL + strlen(#URL) - 4, ".gif")) || (!strcmp(#URL + strlen(#URL) - 4, ".png")) || (!strcmp(#URL + strlen(#URL) - 4, ".jpg"))
	{
		//if (strstr(#URL,"http:")) 
		RunProgram("/sys/media/kiv", #URL);
		strcpy(#editURL, BrowserHistory.CurrentUrl());
		strcpy(#URL, BrowserHistory.CurrentUrl());
		return;
	}
	if (!strcmpn(#URL,"mailto:", 7))
	{
		notify(#URL);
		strcpy(#editURL, BrowserHistory.CurrentUrl());
		strcpy(#URL, BrowserHistory.CurrentUrl());
		return;
	}

	OpenPage();
	return;
}

void OpenPage()
{
	if (GetProcessSlot(downloader_id)<>0) PutPaletteImage(#toolbar,200,42,0,0,8,#toolbar_pal);
	KillProcess(downloader_id);
	strcpy(#editURL, #URL);
	BrowserHistory.AddUrl();
	strcpy(#header, #version);
	pre_text =0;
	if (!strncmp(#URL,"http:",5))
	{
		KillProcess(downloader_id);
		DeleteFile(#download_path);
		IF (URL[strlen(#URL)-1]=='/') URL[strlen(#URL)-1]=NULL;
		downloader_id = RunProgram("/sys/network/downloader", #URL);
		IF (downloader_id<0) notify("Error running Downloader. Internet unavilable.");
		Draw_Window();
		return;
	}
	WB1.list.first = WB1.list.count =0;
	WB1.ReadHtml(_WIN);
	ShowPage();
}

void ShowPage()
{
	address_box.size = address_box.pos = strlen(#editURL);
	address_box.offset=0;
	edit_box_draw stdcall(#address_box);

	if (!filesize)
	{
		PageLinks.Clear();
		if (GetProcessSlot(downloader_id)<>0) 
		{
				filesize = sizeof(loading);
				WB1.ParseHTML(#loading);
		}
		else
		{
			if (strncmp(#URL,"http:",5)==0)
			{
				filesize = sizeof(page_not_found_no_internet);
				WB1.ParseHTML(#page_not_found_no_internet);
			}
			else
			{
				filesize = sizeof(page_not_found);
				WB1.ParseHTML(#page_not_found);
			}
		}
		//return;
	}
	else
		WB1.ParseHTML(buf);

	if (!header) strcpy(#header, #version);
	if (!strcmp(#version, #header)) DrawTitle(#header);
}


stop:
