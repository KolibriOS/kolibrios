//HTML Viewer in C--
//Copyright 2007-2013 by Veliant & Leency
//Asper, lev, Lrz, Barsuk, Nable...
//home icon - rachel fu, GPL licence

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

//libraries
#define MEMSIZE 0x100000
#include "..\lib\strings.h"
#include "..\lib\gui.h"
#include "..\lib\file_system.h"
#include "..\lib\mem.h"
#include "..\lib\draw_buf.h"
#include "..\lib\list_box.h"
#include "..\lib\cursor.h"

//*.obj libraries
#include "..\lib\obj\box_lib.h"
#include "..\lib\obj\libio_lib.h"
#include "..\lib\obj\libimg_lib.h"
#include "..\lib\obj\http.h"
#include "..\lib\obj\iconv.h"

//useful patterns
#include "..\lib\patterns\libimg_load_skin.h"

char homepage[] = FROM "html\\homepage.htm";

#ifdef LANG_RUS
	char version[]=" Текстовый браузер 1.3 UNSTABLE";
	?define IMAGES_CACHE_CLEARED "Кэш картинок очищен"
	?define T_LAST_SLIDE "Это последний слайд"
	char loading[] = "Загрузка страницы...<br>";
	char page_not_found[] = FROM "html\page_not_found_ru.htm";
	char accept_language[]= "Accept-Language: ru\n";
#else
	char version[]=" Text-based Browser 1.3 UNSTABLE";
	?define IMAGES_CACHE_CLEARED "Images cache cleared"
	?define T_LAST_SLIDE "This slide is the last"
	char loading[] = "Loading...<br>";
	char page_not_found[] = FROM "html\page_not_found_en.htm";
	char accept_language[]= "Accept-Language: en\n";	
#endif



proc_info Form;
#define WIN_W 799
#define WIN_H 559

char search_path[]="http://nigma.ru/index.php?s=";
char str_location[]="location\0";
int redirected = 0;

char stak[4096];

int action_buf;

dword http_transfer = 0;
dword http_buffer;

dword TAB_H = false; //19;
dword TAB_W = 150;
dword TOOLBAR_H = 31; //50;
dword STATUSBAR_H = 15;
dword col_bg;
dword panel_color;
dword border_color;

progress_bar wv_progress_bar;
byte souce_mode = false;

enum { 
	BACK_BUTTON=1000, 
	FORWARD_BUTTON, 
	REFRESH_BUTTON, 
	GOTOURL_BUTTON, 
	SEARCHWEB_BUTTON, 
	SANDWICH_BUTTON
};

enum {
	ZOOM2x=1100,
	VIEW_SOURCE,
	EDIT_SOURCE,
	VIEW_HISTORY,
	FREE_IMG_CACHE,
	DOWNLOAD_MANAGER
};

#include "..\TWB\TWB.c"
#include "menu_rmb.h"
#include "history.h"
#include "show_src.h"
#include "network_get.h"
#include "downloader.h"

char editURL[sizeof(URL)];
int	mouse_twb;
edit_box address_box = {250,55,34,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(URL),#editURL,#mouse_twb,2,19,19};

#define URL_SERVICE_HISTORY "WebView://history"
#define URL_SERVICE_HOME "WebView://home"
#define URL_SERVICE_SOURCE "WebView://source:"


libimg_image skin;

int SetSkinColors()
{
	dword image_data;
	image_data = DSDWORD[skin.image+24];
	col_bg = DSDWORD[image_data];
	panel_color  = DSDWORD[skin.w*4*4 + image_data];
	border_color = DSDWORD[skin.w*4*7 + image_data];
	wv_progress_bar.progress_color = DSDWORD[skin.w*4*10 + image_data];
	$and col_bg, 0x00ffffff
	$and panel_color, 0x00ffffff
	$and border_color, 0x00ffffff
	$and wv_progress_bar.progress_color, 0x00ffffff
}

void DrawProgress()
{
	unsigned long btn;
	if (http_transfer == 0) return;
	if (wv_progress_bar.max) btn = address_box.width*wv_progress_bar.value/wv_progress_bar.max; else btn = 30;
	DrawBar(address_box.left-1, address_box.top+15, btn, 2, wv_progress_bar.progress_color);
}


void main()
{
	dword btn;
	int half_scroll_size;
	int scroll_used=0, show_menu;

	CursorPointer.Load(#CursorFile);
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libHTTP, #http_lib_init,1);
	load_dll(iconv_lib, #iconv_open,0);
	//load_dll(kmenu, #akmenu_init,0);
	Libimg_LoadImage(#skin, abspath("wv_skin.png"));
	SetSkinColors();
	
	WB1.DrawBuf.zoom = 1;
	WB1.list.SetFont(8, 14, 10111000b);
	Form.width=WIN_W;
	Form.height=WIN_H;
	SetElementSizes();
	if (param) strcpy(#URL, #param); else strcpy(#URL, URL_SERVICE_HOME);
	OpenPage();

	CreateDir("/tmp0/1/downloads");

	SetEventMask(0xa7);
	BEGIN_LOOP_APPLICATION:
		WaitEventTimeout(2);
		switch(EAX & 0xFF)
		{
			CASE evMouse:
				if (!CheckActiveProcess(Form.ID)) break;
				//Edit URL
				edit_box_mouse stdcall (#address_box);
				mouse.get();
				//Links hover
				if (mouse.y>WB1.list.y) PageLinks.Hover(mouse.x, mouse.y, link_color_inactive, link_color_active, bg_color);
				//Menu
				if (mouse.y>WB1.list.y) && (mouse.y<Form.height) && (bufsize)
				{
					if (mouse.pkm) && (mouse.up)
					{
						CreateThread(#menu_rmb,#stak+4092);
						break; 
					}
				}
				//Mouse scroll
				if (mouse.vert)
				{
					if (WB1.list.MouseScroll(mouse.vert)) WB1.DrawPage();
				}
				//Drag scroller
				scroll_wv.all_redraw = 0;
				if (!mouse.lkm) scroll_used=0;
				if (mouse.x>=scroll_wv.start_x) && (mouse.x<=scroll_wv.start_x+scroll_wv.size_x) 
				&& (mouse.y>=scroll_wv.start_y+scroll_wv.btn_height) && (-scroll_wv.btn_height+scroll_wv.start_y+scroll_wv.size_y>mouse.y)
				&& (WB1.list.count>WB1.list.visible) && (mouse.lkm)
				{
					scroll_used=1;
				}				
				if (scroll_used)
				{
					mouse.y = mouse.y + 5;
					half_scroll_size = WB1.list.h - 16 * WB1.list.visible / WB1.list.count - 3 /2;
					if (half_scroll_size+WB1.list.y>mouse.y) || (mouse.y<0) || (mouse.y>4000) mouse.y=half_scroll_size+WB1.list.y;
					btn=WB1.list.first;
					WB1.list.first = mouse.y -half_scroll_size -WB1.list.y * WB1.list.count / WB1.list.h;
					if (WB1.list.visible+WB1.list.first>WB1.list.count) WB1.list.first=WB1.list.count-WB1.list.visible;
					if (btn!=WB1.list.first) WB1.DrawPage();
				}
				break;

			case evButton:
				btn=GetButtonID();
				if (btn==1)	ExitProcess();
				Scan(btn);
				break;

			case evKey:
				GetKeys();
				if (address_box.flags & 0b10)  
				{
					if (key_ascii == ASCII_KEY_ENTER) Scan(key_scancode); else
					if (key_ascii != 0x0d) && (key_ascii != 183) && (key_ascii != 184) {EAX = key_ascii << 8; edit_box_key stdcall(#address_box);}
				}
				else 
				{
					Scan(key_scancode);
				}
				break;

			case evReDraw:
				if (action_buf) Scan(action_buf);
				DefineAndDrawWindow(GetScreenWidth()-WIN_W/2,GetScreenHeight()-WIN_H/2,WIN_W,WIN_H,0x73,col_bg,0,0);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window>2) { DrawTitle(#header); break; }
				if (Form.height<120) MoveSize(OLD,OLD,OLD,120);
				if (Form.width<280) MoveSize(OLD,OLD,280,OLD);
				Draw_Window();
				break;
				
			case evNetwork:
				if (http_transfer > 0) {
					http_receive stdcall (http_transfer);
					$push EAX
					ESI = http_transfer;
					wv_progress_bar.max = ESI.http_msg.content_length;
					if (wv_progress_bar.value != ESI.http_msg.content_received)
					{
						wv_progress_bar.value = ESI.http_msg.content_received;	
						DrawProgress();
					}
					$pop EAX
					if (EAX == 0) {
						ESI = http_transfer;
						// Handle redirects
						if (ESI.http_msg.status >= 300) && (ESI.http_msg.status < 400)
						{
							redirected++;
							if (redirected<=5)
							{
								http_find_header_field stdcall (http_transfer, #str_location);
								if (EAX!=0) {
									ESI = EAX;
									EDI = #URL;
									do {
										$lodsb;
										$stosb;
									} while (AL != 0) && (AL != 13) && (AL != 10));
									DSBYTE[EDI-1]='\0';
								}
							}
							else
							{
							//TODO: display error (too many redirects)
							}
						} 
						else
						{
							redirected = 0;
						}
						// Loading the page is complete, free resources
						if (redirected>0)
						{
							http_free stdcall (http_transfer);
							http_transfer=0;
							PageLinks.GetAbsoluteURL(#URL);
							BrowserHistory.current--;
							strcpy(#editURL, #URL);
							DrawEditBox();
							OpenPage();
						}
						else
						{
							BrowserHistory.AddUrl();
							ESI = http_transfer;
							bufpointer = ESI.http_msg.content_ptr;
							bufsize = ESI.http_msg.content_received;
							http_free stdcall (http_transfer);
							http_transfer=0;
							SetPageDefaults();
							Draw_Window();		// stop button => refresh button
						}
					}
				}
		}
	goto BEGIN_LOOP_APPLICATION;
}

void SetElementSizes()
{
	address_box.top = TOOLBAR_H-TAB_H/2-7+TAB_H;
	address_box.width = Form.cwidth - address_box.left - 25 - 22;
	WB1.list.SetSizes(0, TOOLBAR_H, Form.width - 10 - scroll_wv.size_x / WB1.DrawBuf.zoom, 
		Form.cheight - TOOLBAR_H - STATUSBAR_H, WB1.list.font_h + WB1.DrawBuf.zoom + WB1.DrawBuf.zoom * WB1.DrawBuf.zoom);
	WB1.list.wheel_size = 7;
	WB1.list.column_max = WB1.list.w - scroll_wv.size_x / WB1.list.font_w;
	WB1.list.visible = WB1.list.h - 5 / WB1.list.line_h;
	WB1.DrawBuf.Init(WB1.list.x, WB1.list.y, WB1.list.w, WB1.list.h * 20);
}

void Draw_Window()
{
	int img_off;
	// tab {
	/*
	if (TAB_H)
	{
		DrawBar(0, 0, TAB_W, TAB_H+1, panel_color);
		WriteText(5, 7, 0x80, 0xfdfdFd, "Index.htm");
		WriteText(4, 6, 0x80, 0, "Index.htm");		
		DrawBar(TAB_W,0, Form.cwidth-TAB_W,TAB_H, col_bg);
		DrawBar(TAB_W-1,TAB_H, Form.cwidth-TAB_W+1,1, border_color);
		img_draw stdcall(skin.image, TAB_W-13, 0, 30, skin.h, 101, 0);
	} 
	else */ DrawBar(0,0, Form.cwidth,1, col_bg);
	// }
	DrawBar(0,TAB_H+1, Form.cwidth,TOOLBAR_H-TAB_H-3, panel_color);
	DrawBar(0,TOOLBAR_H-2, Form.cwidth,1, 0xD7D0D3);
	DrawBar(0,TOOLBAR_H-1, Form.cwidth,1, border_color);
	SetElementSizes();
	DrawRectangle(address_box.left-2, address_box.top-3, address_box.width+4, 20,border_color);
	DrawRectangle(address_box.left-1, address_box.top-2, address_box.width+2, 18,address_box.color);
	DrawRectangle(address_box.left-1, address_box.top-1, address_box.width+2, 16,address_box.color);
	// < / >
	DefineButton(address_box.left-49, address_box.top-2, 23, skin.h-2, BACK_BUTTON+BT_HIDE, 0);
	DefineButton(address_box.left-25, address_box.top-2, 23, skin.h-2, FORWARD_BUTTON+BT_HIDE, 0);
	img_draw stdcall(skin.image, address_box.left-50, address_box.top-3, 48, skin.h, 3, 0);
	// refresh_BUTTON
	DefineButton(address_box.left+address_box.width+1, address_box.top-3, 16, skin.h-1, REFRESH_BUTTON+BT_HIDE+BT_NOFRAME, 0);
	if (http_transfer > 0) img_off = 131; else img_off = 52;
	img_draw stdcall(skin.image, address_box.left+address_box.width+1, address_box.top-3, 17, skin.h, img_off, 0);
	// config
	DefineButton(Form.cwidth-24, address_box.top-3, 19, skin.h-1, SANDWICH_BUTTON+BT_HIDE, 0);
	img_draw stdcall(skin.image, Form.cwidth-22, address_box.top-3, 16, skin.h, 85, 0);
	//status bar
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,STATUSBAR_H, col_bg);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, border_color);
	ShowPage();
	DrawRectangle(scroll_wv.start_x, scroll_wv.start_y, scroll_wv.size_x, scroll_wv.size_y-1, scroll_wv.bckg_col);
	DrawProgress();
}


void Scan(dword id__)
{
	action_buf=0;
	if (id__ >= 400) && (id__ < 1000)
	{
		ProcessLinks(id__);
		return;
	}
	switch (id__)
	{
		case SCAN_CODE_BS:
		case BACK_BUTTON:
			if (!BrowserHistory.GoBack()) return;
			OpenPage();
			return;

		case FORWARD_BUTTON:
			if (!BrowserHistory.GoForward()) return;
			OpenPage();
			return;

		case SCAN_CODE_HOME:
		case SCAN_CODE_END:
		case SCAN_CODE_PGUP:
		case SCAN_CODE_PGDN:
			if (WB1.list.ProcessKey(key_scancode)) WB1.DrawPage();
			return;

		case SCAN_CODE_UP:
			if (WB1.list.first <= 0) return;
			WB1.list.first--;
			WB1.DrawPage();
			return;

		case SCAN_CODE_DOWN:
			if (WB1.list.visible + WB1.list.first >= WB1.list.count) return;
			WB1.list.first++;
			WB1.DrawPage();
			return;

		case GOTOURL_BUTTON:
		case SCAN_CODE_ENTER: //enter
			if (!editURL[0]) return;
			if (strncmp(#editURL,"http:",5)) && (editURL[0]!='/') && (strncmp(#editURL,"WebView:",9)) strncpy(#URL,"http://",7);
			else
				URL[0] = 0;
			strcat(#URL, #editURL);
			OpenPage();
			return;

		case 063: //F5
			IF(address_box.flags & 0b10) return;
		case REFRESH_BUTTON:
			if (http_transfer > 0) 
			{
				StopLoading();
				Draw_Window();
			}
			else OpenPage();
			return;

		case SANDWICH_BUTTON:
			mouse.y = TOOLBAR_H-6;
			mouse.x = Form.cwidth - 167;
			CreateThread(#menu_rmb,#stak+4092);
			return;

		case ZOOM2x:
			if (WB1.DrawBuf.zoom==2)
			{
				WB1.DrawBuf.zoom=1;
				WB1.list.SetFont(8, 14, 10111000b);
			}
			else
			{
				WB1.DrawBuf.zoom=2;
				WB1.list.SetFont(8, 14, 10111001b);
			}
			Draw_Window(); 
			return;

		case VIEW_SOURCE:
			WB1.list.first = 0;
			ShowSource();
			WB1.DrawPage();
			break;

		case EDIT_SOURCE:
			if (!strncmp(#URL,"http:",5)) 
			{
				WriteFile(bufsize, bufpointer, "/tmp0/1/WebView_tmp.htm");
				if (!EAX) RunProgram("/rd/1/tinypad", "/tmp0/1/WebView_tmp.htm");
			}
			else RunProgram("/rd/1/tinypad", #URL);
			return;

		case FREE_IMG_CACHE:
			ImgCache.Free();
			notify(IMAGES_CACHE_CLEARED);
			WB1.DrawPage();
			return;

		case VIEW_HISTORY:
			strcpy(#URL, URL_SERVICE_HISTORY);
			OpenPage();
			return;

		case DOWNLOAD_MANAGER:
			if (!downloader_opened) {
				strncpy(#DL_URL, "http://",7);
				CreateThread(#Downloader,#downloader_stak+4092);
			}
			return;
/*
		case 020:
		case NEWTAB:
			MoveSize(190,80,OLD,OLD);
			RunProgram(#program_path, #URL);
			return;

		case SEARCHWEB_BUTTON:
			sprintf(#URL,"%s%s",#search_path,#editURL);
			OpenPage();
			return;
*/
	}
}



void ProcessLinks(int id)
{
	if (http_transfer > 0) 
	{
		StopLoading();
		BrowserHistory.current--;
	}

	strcpy(#URL, PageLinks.GetURL(id-401));	
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
	
	PageLinks.GetAbsoluteURL(#URL);
	
	if (UrlExtIs(".png")==1) || (UrlExtIs(".gif")==1) || (UrlExtIs(".jpg")==1) || (UrlExtIs(".zip")==1) || (UrlExtIs(".kex")==1)
	|| (UrlExtIs(".7z")==1) || (UrlExtIs("netcfg")==1) 
	{
		//notify(#URL);
		if (!strncmp(#URL,"http://", 7))
		{
			strcpy(#DL_URL, #URL);
			CreateThread(#Downloader,#downloader_stak+4092);
		}
		else RunProgram("@open", #URL);
		strcpy(#editURL, BrowserHistory.CurrentUrl());
		strcpy(#URL, BrowserHistory.CurrentUrl());
		return;
	}
	if (!strncmp(#URL,"mailto:", 7))
	{
		notify(#URL);
		strcpy(#editURL, BrowserHistory.CurrentUrl());
		strcpy(#URL, BrowserHistory.CurrentUrl());
		return;
	}
	OpenPage();
	return;
}

void StopLoading()
{
	if (http_transfer)
	{
		EAX = http_transfer;
		EAX = EAX.http_msg.content_ptr;		// get pointer to data
		$push	EAX							// save it on the stack
		http_free stdcall (http_transfer);	// abort connection
		$pop	EAX							
		free(EAX);						// free data
		http_transfer=0;
		bufsize = 0;
		bufpointer = free(bufpointer);
	}
	wv_progress_bar.value = 0;
	img_draw stdcall(skin.image, address_box.left+address_box.width+1, address_box.top-3, 17, skin.h, 52, 0);
}

void SetPageDefaults()
{
	strcpy(#header, #version);
	WB1.list.count = WB1.list.first = 0;
	stroka = 0;
	cur_encoding = CH_NULL;
	if (o_bufpointer) o_bufpointer = free(o_bufpointer);
	anchor_line_num=WB1.list.first;
	anchor[0]='|';
}

void OpenPage()
{
	StopLoading();
	souce_mode = false;
	strcpy(#editURL, #URL);
	BrowserHistory.AddUrl();
	if (!strncmp(#URL,"WebView:",8))
	{
		SetPageDefaults();
		if (!strcmp(#URL, URL_SERVICE_HOME)) WB1.LoadInternalPage(#homepage, sizeof(homepage));
		else if (!strcmp(#URL, URL_SERVICE_HISTORY)) ShowHistory();
		return;
	}
	if (!strncmp(#URL,"http:",5))
	{
		img_draw stdcall(skin.image, address_box.left+address_box.width+1, address_box.top-3, 17, skin.h, 131, 0);
		http_get stdcall (#URL, 0, 0, #accept_language);
		http_transfer = EAX;
		if (!http_transfer)
		{
			StopLoading();
			bufsize = 0;
			bufpointer = free(bufpointer);
			ShowPage();
			return;
		}
	}
	else
	{
		file_size stdcall (#URL);
		bufsize = EBX;
		if (bufsize)
		{
			free(bufpointer);
			bufpointer = malloc(bufsize);
			SetPageDefaults();
			ReadFile(0, bufsize, bufpointer, #URL);
			//ShowSource();
		}
		ShowPage();
	}
}

DrawEditBox()
{
	address_box.size = address_box.pos = address_box.shift = address_box.shift_old = strlen(#editURL);
	address_box.offset = 0;
	edit_box_draw stdcall(#address_box);
}


void ShowPage()
{
	DrawEditBox();
	if (!bufsize)
	{
		PageLinks.Clear();
		if (http_transfer)
		{
			WB1.LoadInternalPage(#loading, sizeof(loading));
		}
		else
			WB1.LoadInternalPage(#page_not_found, sizeof(page_not_found));
	}
	else
		WB1.Prepare();

	if (!header) strcpy(#header, #version);
	if (!strcmp(#version, #header)) DrawTitle(#header);
}

byte UrlExtIs(dword ext)
{
	if (!strcmpi(#URL + strlen(#URL) - strlen(ext), ext)) return true;
	return false;
}



char downloader_stak[4096];
stop: