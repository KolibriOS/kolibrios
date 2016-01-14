//HTML Viewer in C--
//Copyright 2007-2013 by Veliant & Leency
//Asper, lev, Lrz, Barsuk, Nable...
//home icon - rachel fu, GPL licence

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

//libraries
#define MEMSIZE 1060000
#include "..\lib\gui.h"
#include "..\lib\draw_buf.h"
#include "..\lib\list_box.h"
#include "..\lib\cursor.h"
#include "..\lib\collection.h"
#include "..\lib\font.h"
#include "..\lib\menu.h"

//*.obj libraries
#include "..\lib\obj\box_lib.h"
#include "..\lib\obj\libio_lib.h"
#include "..\lib\obj\libimg_lib.h"
#include "..\lib\obj\http.h"
#include "..\lib\obj\iconv.h"
//useful patterns
#include "..\lib\patterns\libimg_load_skin.h"
#include "..\lib\patterns\history.h"
#include "..\lib\patterns\http_downloader.h"

char homepage[] = FROM "html\\homepage.htm";

#ifdef LANG_RUS
char version[]=" Текстовый браузер 1.48";
?define IMAGES_CACHE_CLEARED "Кэш картинок очищен"
?define T_LAST_SLIDE "Это последний слайд"
char loading[] = "Загрузка страницы...<br>";
char page_not_found[] = FROM "html\page_not_found_ru.htm";
char accept_language[]= "Accept-Language: ru\n";
char rmb_menu[] = 
"Посмотреть исходник
Редактировать исходник
История
Очистить кэш картинок
Менеджер загрузок";
#else
char version[]=" Text-based Browser 1.48";
?define IMAGES_CACHE_CLEARED "Images cache cleared"
?define T_LAST_SLIDE "This slide is the last"
char loading[] = "Loading...<br>";
char page_not_found[] = FROM "html\page_not_found_en.htm";
char accept_language[]= "Accept-Language: en\n";
char rmb_menu[] =
"View source
Edit source
History
Free image cache
Download Manager";
#endif

#define URL_SERVICE_HISTORY "WebView://history"
#define URL_SERVICE_HOME "WebView://home"
#define URL_SERVICE_SOURCE "WebView://source:"

proc_info Form;

//char search_path[]="http://nigma.ru/index.php?s=";
int redirected = 0;

char stak[4096];

int action_buf;

dword http_transfer = 0;
dword http_buffer;

dword TOOLBAR_H = 33;
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
	SANDWICH_BUTTON
};

enum {
	VIEW_SOURCE=1100,
	EDIT_SOURCE,
	VIEW_HISTORY,
	FREE_IMG_CACHE,
	DOWNLOAD_MANAGER
};

#include "..\TWB\TWB.c"
#include "history.h"
#include "show_src.h"
#include "download_manager.h"

char editURL[sizeof(URL)];
int	mouse_twb;
edit_box address_box = {250,56,34,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(URL),#editURL,#mouse_twb,2,19,19};


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
	Libimg_LoadImage(#skin, abspath("wv_skin.png"));
	SetSkinColors();
	CreateDir("/tmp0/1/downloads");
	if (param) strcpy(#URL, #param); else strcpy(#URL, URL_SERVICE_HOME);
	WB1.DrawBuf.zoom = 1;
	WB1.list.SetFont(8, 14, 10011000b);
	WB1.list.no_selection = true;
	label.init(DEFAULT_FONT);
	SetEventMask(0xa7);
	BEGIN_LOOP_APPLICATION:
		WaitEventTimeout(2);
		switch(EAX & 0xFF)
		{
			CASE evMouse:
				if (!CheckActiveProcess(Form.ID)) break;
				edit_box_mouse stdcall (#address_box);
				mouse.get();
				if (WB1.list.MouseOver(mouse.x, mouse.y))
				{
					PageLinks.Hover(mouse.x, WB1.list.first*WB1.list.item_h + mouse.y, link_color_inactive, link_color_active, bg_color);
					if (bufsize) && (mouse.pkm) && (mouse.up) {
						menu.show(Form.left+mouse.x-6,Form.top+mouse.y+skin_height+3, 180, #rmb_menu, VIEW_SOURCE);
						break;
					}
					if (WB1.list.MouseScroll(mouse.vert)) WB1.DrawPage();
				}
				scrollbar_v_mouse (#scroll_wv);
				if (WB1.list.first != scroll_wv.position)
				{
					WB1.list.first = scroll_wv.position;
					WB1.DrawPage();
					break;
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
				if (menu.list.cur_y) {
					Scan(menu.list.cur_y);
					menu.list.cur_y = 0;
				}
				DefineAndDrawWindow(GetScreenWidth()-800/2,GetScreenHeight()-600/2,800,600,0x73,col_bg,0,0);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window>2) { DrawTitle(#header); break; }
				if (Form.height<120) { MoveSize(OLD,OLD,OLD,120); break; }
				if (Form.width<280) { MoveSize(OLD,OLD,280,OLD); break; }
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
								http_find_header_field stdcall (http_transfer, "location\0");
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
								notify("Too many redirects");
								StopLoading();
								break;
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
							GetAbsoluteURL(#URL);
							history.back();
							strcpy(#editURL, #URL);
							DrawEditBox();
							OpenPage();
						}
						else
						{
							history.add(#URL);
							ESI = http_transfer;
							bufpointer = ESI.http_msg.content_ptr;
							bufsize = ESI.http_msg.content_received;
							http_free stdcall (http_transfer);
							http_transfer=0;
							SetPageDefaults();
							ShowPage();
						}
					}
				}
		}
	goto BEGIN_LOOP_APPLICATION;
}

void SetElementSizes()
{
	address_box.top = TOOLBAR_H/2-7;
	address_box.width = Form.cwidth - address_box.left - 25 - 22;
	WB1.list.SetSizes(0, TOOLBAR_H, Form.width - 10 - scroll_wv.size_x / WB1.DrawBuf.zoom, 
		Form.cheight - TOOLBAR_H - STATUSBAR_H, WB1.list.font_h + WB1.DrawBuf.zoom + WB1.DrawBuf.zoom * WB1.DrawBuf.zoom);
	WB1.list.wheel_size = 7;
	WB1.list.column_max = WB1.list.w - scroll_wv.size_x / WB1.list.font_w;
	WB1.list.visible = WB1.list.h - 5 / WB1.list.item_h;
	if (WB1.list.w!=WB1.DrawBuf.bufw) {
		WB1.DrawBuf.Init(WB1.list.x, WB1.list.y, WB1.list.w, WB1.list.h * 30);
		Scan(REFRESH_BUTTON);
	}
}



void Draw_Window()
{
	int list__w, list__h;
	DrawBar(0,0, Form.cwidth,TOOLBAR_H-2, panel_color);
	DrawBar(0,TOOLBAR_H-2, Form.cwidth,1, 0xD7D0D3);
	DrawBar(0,TOOLBAR_H-1, Form.cwidth,1, border_color);
	SetElementSizes();
	DrawRectangle(address_box.left-3, address_box.top-3, address_box.width+5, 20,border_color);
	DefineButton(address_box.left-50, address_box.top-2, 23, skin.h-2, BACK_BUTTON+BT_HIDE, 0);
	DefineButton(address_box.left-26, address_box.top-2, 23, skin.h-2, FORWARD_BUTTON+BT_HIDE, 0);
	img_draw stdcall(skin.image, address_box.left-51, address_box.top-3, 48, skin.h, 3, 0);
	DefineButton(address_box.left+address_box.width+1, address_box.top-3, 16, skin.h-1, REFRESH_BUTTON+BT_HIDE+BT_NOFRAME, 0);
	DefineButton(Form.cwidth-24, address_box.top-3, 19, skin.h-1, SANDWICH_BUTTON+BT_HIDE, 0);
	img_draw stdcall(skin.image, Form.cwidth-22, address_box.top-3, 16, skin.h, 85, 0);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,STATUSBAR_H, col_bg);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, border_color);
	if (!header) OpenPage(); else { WB1.DrawPage(); DrawEditBox(); }
	DrawRectangle(scroll_wv.start_x, scroll_wv.start_y, scroll_wv.size_x, scroll_wv.size_y-1, scroll_wv.bckg_col);
	DrawProgress();

	/*
	list__w = 200;
	list__h = 200;
	label.raw_size = 0;
	label.write_buf(10,10, list__w, list__h, 0xFFFFFF, 0, 11, "Hello World!");
	label.write_buf(10,23, list__w, list__h, 0xFFFFFF, 0xFF00FF, 12, "How are you?");
	label.write_buf(11,40, list__w, list__h, 0xFFFFFF, 0x2E74BB, 15, "Fine");
	label.apply_smooth();
	label.show_buf(0,0);
	*/
}


void Scan(dword id__)
{
	action_buf=0;
	if (WB1.list.ProcessKey(id__)) WB1.DrawPage();
	else switch (id__)
	{
		case SCAN_CODE_BS:
		case BACK_BUTTON:
			if (history.back()) {
				strcpy(#URL, history.current());
				OpenPage();
			}
			return;
		case FORWARD_BUTTON:
			if (history.forward()) {
				strcpy(#URL, history.current());
				OpenPage();
			}
			return;
		case GOTOURL_BUTTON:
		case SCAN_CODE_ENTER:
			if (!strncmp(#editURL,"http:",5)) || (editURL[0]=='/') || (!strncmp(#editURL,"WebView:",9))
			{
				strcpy(#URL, #editURL);
			}
			else
			{
				strlcpy(#URL,"http://",7);
				strcat(#URL, #editURL);
			}
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
			menu.show(Form.left+mouse.x-6,Form.top+mouse.y+skin_height+3, 180, #rmb_menu, VIEW_SOURCE);
			return;
		case VIEW_SOURCE:
			WB1.list.first = 0;
			ShowSource();
			WB1.LoadInternalPage(bufpointer, bufsize);
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
				downloader_edit = NULL;
				CreateThread(#Downloader,#downloader_stak+4092);
			}
			return;
	}
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
	history.add(#URL);
	if (!strncmp(#URL,"WebView:",8))
	{
		SetPageDefaults();
		if (!strcmp(#URL, URL_SERVICE_HOME)) WB1.LoadInternalPage(#homepage, sizeof(homepage));
		else if (!strcmp(#URL, URL_SERVICE_HISTORY)) ShowHistory();
		DrawEditBox();
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
		}
		ShowPage();
	}
}

DrawEditBox()
{
	DrawWideRectangle(address_box.left-2, address_box.top-2, address_box.width+3, 19, 2, address_box.color);
	address_box.size = address_box.pos = address_box.shift = address_box.shift_old = strlen(#editURL);
	address_box.offset = 0;
	edit_box_draw stdcall(#address_box);
	if (http_transfer > 0) EAX = 131; else EAX = 52;
	img_draw stdcall(skin.image, address_box.left+address_box.width+1, address_box.top-3, 17, skin.h, EAX, 0);
}


void ShowPage()
{
	DrawEditBox();
	if (!bufsize)
	{
		if (http_transfer) WB1.LoadInternalPage(#loading, sizeof(loading));
		else WB1.LoadInternalPage(#page_not_found, sizeof(page_not_found));
	}
	else
	{
		WB1.Prepare();
	}
	//if (!header) strcpy(#header, #version);
	if (!strcmp(#version, #header)) DrawTitle(#header);
}

byte UrlExtIs(dword ext)
{
	if (!strcmpi(#URL + strlen(#URL) - strlen(ext), ext)) return true;
	return false;
}

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
	DrawBar(address_box.left-2, address_box.top+15, btn, 2, wv_progress_bar.progress_color);
}

void ClickLink()
{
	if (http_transfer > 0) 
	{
		StopLoading();
		history.back();
	}

	strcpy(#URL, PageLinks.GetURL(PageLinks.active));	
	//#1
	if (URL[0] == '#')
	{
		strcpy(#anchor, #URL+strrchr(#URL, '#'));		
		strcpy(#URL, history.current());
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
	
	GetAbsoluteURL(#URL);
	
	if (UrlExtIs(".png")==1) || (UrlExtIs(".gif")==1) || (UrlExtIs(".jpg")==1) || (UrlExtIs(".zip")==1) || (UrlExtIs(".kex")==1)
	|| (UrlExtIs(".7z")==1) || (UrlExtIs("netcfg")==1) 
	{
		//notify(#URL);
		if (!strncmp(#URL,"http://", 7))
		{
			strcpy(#downloader_edit, #URL);
			CreateThread(#Downloader,#downloader_stak+4092);
		}
		else RunProgram("@open", #URL);
		strcpy(#editURL, history.current());
		strcpy(#URL, history.current());
		return;
	}
	if (!strncmp(#URL,"mailto:", 7))
	{
		notify(#URL);
		strcpy(#editURL, history.current());
		strcpy(#URL, history.current());
		return;
	}
	OpenPage();
	return;
}


char downloader_stak[4096];
stop: