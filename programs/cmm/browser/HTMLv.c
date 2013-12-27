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
#include "..\lib\list_box.h"
//*.obj libraries
#include "..\lib\lib.obj\box_lib.h"
#include "..\lib\lib.obj\libio_lib.h"
#include "..\lib\lib.obj\libimg_lib.h"
//images
#include "img\toolbar_icons.c"
#include "img\URLgoto.txt";

#ifdef LANG_RUS
	char version[]=" Текстовый браузер 0.99.31";
	?define IMAGES_CACHE_CLEARED "Кэш картинок очищен"
#else
	char version[]=" Text-based Browser 0.99.31";
	?define IMAGES_CACHE_CLEARED "Images cache cleared"
#endif

proc_info Form;
#define WIN_W 640
#define WIN_H 480

char search_path[]="http://nigma.ru/index.php?s=";

char stak[4096];
mouse m;
int action_buf;



#include "..\TWB\TWB.c"
#include "menu_rmb.h"


void main()
{
	int key, btn;
	int half_scroll_size;
	int scroll_used=0, show_menu;
	
	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0) {notify("System Error: library doesn't exists /rd/1/lib/box_lib.obj"); ExitProcess();}
	if (load_dll2(libio, #libio_init,1)!=0) debug("Error: library doesn't exists - libio");
	if (load_dll2(libimg, #libimg_init,1)!=0) debug("Error: library doesn't exists - libimg");
	
	if (!URL) strcpy(#URL, "/sys/index.htm");
	strcpy(#editURL, #URL);
	
	Form.width=WIN_W;
	Form.height=WIN_H;
	SetElementSizes();
	OpenPage();

	SetEventMask(0x27);
	loop()
	{
		WaitEventTimeout(2);
		switch(EAX & 0xFF)
		{
			CASE evMouse:
				/*
				//not work well, so we are use custom way of processing scroll
				scrollbar_v_mouse (#scroll_wv);
				if (WB1.list.first <> scroll_wv.position)
				{
					WB1.list.first = scroll_wv.position;
					WB1.ParseHTML(buf, filesize);
				};
				*/
				
				if (!CheckActiveProcess(Form.ID)) break;

				edit_box_mouse stdcall (#address_box);

				m.get();
				
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

		case 002: //free img cache
			FreeImgCache();
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
			if (strcmp(get_URL_part(5),"http:")<>0) RunProgram("/rd/1/tinypad", #URL);
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
	GetURLfromPageLinks(id);
	
	//#1
	if (URL[0] == '#')
	{
		strcpy(#anchor, #URL+strrchr(#URL, '#'));
		
		strcpy(#URL, BrowserHistory.CurrentUrl());
		
		WB1.list.first=WB1.list.count-WB1.list.visible;
		WB1.ShowPage();
		return;
	}
	//liner.ru#1
	if (strrchr(#URL, '#')<>-1)
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
	if (!strcmp(get_URL_part(5),"http:")))
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
	WB1.ShowPage();
}



stop:
