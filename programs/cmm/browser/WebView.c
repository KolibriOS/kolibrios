//HTML Viewer in C--
//Copyright 2007-2018 by Veliant & Leency
//Asper, lev, Lrz, Barsuk, Nable, hidnplayr...

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

//libraries
#define MEMSIZE 4096 * 200
#include "..\lib\gui.h"
#include "..\lib\draw_buf.h"
#include "..\lib\list_box.h"
#include "..\lib\cursor.h"
#include "..\lib\collection.h"
#include "..\lib\random.h"
#include "..\lib\clipboard.h"

// *.obj libraries
#include "..\lib\obj\box_lib.h"
#include "..\lib\obj\libio.h"
#include "..\lib\obj\libimg.h"
#include "..\lib\obj\http.h"
#include "..\lib\obj\iconv.h"
//useful patterns
#include "..\lib\patterns\history.h"
#include "..\lib\patterns\http_downloader.h"

_http http = {0, 0, 0, 0, 0, 0, 0};

char homepage[] = FROM "html\\homepage.htm""\0";

#ifdef LANG_RUS
char version[]="Текстовый браузер 1.8c";
?define IMAGES_CACHE_CLEARED "Кэш картинок очищен"
?define T_LAST_SLIDE "Это последний слайд"
char loading[] = "Загрузка страницы...<br>";
char page_not_found[] = FROM "html\\page_not_found_ru.htm""\0";
char accept_language[]= "Accept-Language: ru\n";
char rmb_menu[] = 
"Посмотреть исходник
Редактировать исходник
История
Менеджер загрузок";
char link_menu[] =
"Копировать ссылку
Скачать содержимое ссылки";
#else
char version[]="Text-based Browser 1.8c";
?define IMAGES_CACHE_CLEARED "Images cache cleared"
?define T_LAST_SLIDE "This slide is the last"
char loading[] = "Loading...<br>";
char page_not_found[] = FROM "html\\page_not_found_en.htm""\0";
char accept_language[]= "Accept-Language: en\n";
char rmb_menu[] =
"View source
Edit source
History
Download Manager";
char link_menu[] =
"Copy link
Download link contents";
#endif


#define URL_SERVICE_HISTORY "WebView://history"
#define URL_SERVICE_HOME "WebView://home"
#define URL_SERVICE_SOURCE "WebView://source:"

proc_info Form;

//char search_path[]="http://nigma.ru/index.php?s=";
int redirected = 0;

char stak[4096];

int action_buf;

dword TOOLBAR_H = 40;
dword STATUSBAR_H = 15;

dword col_bg = 0xE3E2E2;
dword panel_color  = 0xE3E2E2;
dword border_color = 0x8C8C8C;

bool debug_mode = false;
bool old_tag_parser_mode = false;

progress_bar wv_progress_bar;
bool souce_mode = false;
bool open_in_a_new_window = false;

enum { 
	BACK_BUTTON=1000, 
	FORWARD_BUTTON, 
	REFRESH_BUTTON, 
	GOTOURL_BUTTON, 
	SANDWICH_BUTTON,
	VIEW_SOURCE=1100,
	EDIT_SOURCE,
	VIEW_HISTORY,
	//FREE_IMG_CACHE,
	DOWNLOAD_MANAGER,
	COPY_LINK_URL=1200,
	DOWNLOAD_LINK_CONTENTS,
};

#include "..\TWB\TWB.c"
#include "history.h"
#include "show_src.h"
#include "download_manager.h"

char editURL[sizeof(URL)];
edit_box address_box = {250,60,30,0xffffff,0x94AECE,0xffffff,0xffffff,0x10000000,sizeof(URL)-2,#editURL,0,2,19,19};

#define SKIN_Y 24

void main()
{
	int i;
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libHTTP, #http_lib_init,1);
	load_dll(iconv_lib, #iconv_open,0);
	Libimg_LoadImage(#skin, "/sys/toolbar.png");
	skin.h = 26;
	wv_progress_bar.progress_color = 0x72B7EB;
	CreateDir("/tmp0/1/downloads");
	if (param) && (param[0]=='-') && (param[1]=='d') {
		strcpy(#downloader_edit, #param+3);
		CreateThread(#Downloader,#downloader_stak+4092);
		ExitProcess();
	}
	else if (param) strcpy(#URL, #param); else strcpy(#URL, URL_SERVICE_HOME);
	WB1.list.SetFont(8, 14, 10011000b);
	WB1.list.no_selection = true;
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER + EVM_STACK);
	loop() switch(WaitEvent())
	{
		case evMouse:
			edit_box_mouse stdcall (#address_box);
			mouse.get();
			if (PageLinks.HoverAndProceed(mouse.x, WB1.list.first + mouse.y))
			&& (bufsize) && (mouse.pkm) && (mouse.up) {
				if (WB1.list.MouseOver(mouse.x, mouse.y)) EventShowPageMenu(mouse.x, mouse.y);
				break;
			}
			if (WB1.list.MouseScroll(mouse.vert)) WB1.DrawPage();
			scrollbar_v_mouse (#scroll_wv);
			if (WB1.list.first != scroll_wv.position)
			{
				WB1.list.first = scroll_wv.position;
				WB1.DrawPage();
				break;
			}
			break;

		case evButton:
			ProcessEvent(GetButtonID());
			break;

		case evKey:
			GetKeys();
			if (address_box.flags & ed_focus)  
			{
				if (key_ascii == ASCII_KEY_ENTER) ProcessEvent(key_scancode); else {
					EAX = key_editbox; 
					edit_box_key stdcall(#address_box);
				}
			}
			else 
			{
				#define KEY_SCROLL_N 11
				if (SCAN_CODE_UP   == key_scancode) for (i=0;i<KEY_SCROLL_N;i++) WB1.list.KeyUp();
				if (SCAN_CODE_DOWN == key_scancode) for (i=0;i<KEY_SCROLL_N;i++) WB1.list.KeyDown();
				if (WB1.list.ProcessKey(key_scancode)) WB1.DrawPage();
				else ProcessEvent(key_scancode);
			}
			break;

		case evReDraw:
			if (menu.cur_y) {
				ProcessEvent(menu.cur_y);
				menu.cur_y = 0;
			}
			DefineAndDrawWindow(GetScreenWidth()-800/2-random(80),GetScreenHeight()-600/2-random(80),800,600,0x73,col_bg,0,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) { DrawTitle(#header); break; }
			if (Form.height<120) { MoveSize(OLD,OLD,OLD,120); break; }
			if (Form.width<280) { MoveSize(OLD,OLD,280,OLD); break; }
			Draw_Window();
			break;
			
		case evNetwork:
			if (http.transfer > 0) {
				http.receive();
				EventUpdateProgressBar();
				if (http.receive_result == 0) {
					// Handle redirects
					if (http.status_code >= 300) && (http.status_code < 400)
					{
						redirected++;
						if (redirected>5)
						{
							notify("'Too many redirects.' -E");
							StopLoading();
						}
						else
						{
							http.handle_redirect();
							http.free();
							GetAbsoluteURL(#http.redirect_url);
							history.back();
							strcpy(#editURL, #URL);
							DrawEditBoxWebView();
							OpenPage();
						}
						break;
					} 
					redirected = 0;
					// Loading the page is complete, free resources
					history.add(#URL);
					bufpointer = http.content_pointer;
					bufsize = http.content_received;
					http.free();
					SetPageDefaults();
					ShowPage();
				}
			}
	}
}

void SetElementSizes()
{
	address_box.top = TOOLBAR_H/2-10;
	basic_line_h = calc(WB1.list.font_h * 130) / 100;
	address_box.width = Form.cwidth - address_box.left - 50;
	WB1.list.SetSizes(0, TOOLBAR_H, Form.width - 10 - scroll_wv.size_x, 
		Form.cheight - TOOLBAR_H - STATUSBAR_H, basic_line_h);
	WB1.list.wheel_size = 7 * basic_line_h;
	WB1.list.column_max = WB1.list.w - scroll_wv.size_x / WB1.list.font_w;
	WB1.list.visible = WB1.list.h;
	if (WB1.list.w!=WB1.DrawBuf.bufw) {
		WB1.DrawBuf.Init(WB1.list.x, WB1.list.y, WB1.list.w, 32700);
		ProcessEvent(REFRESH_BUTTON);
	}
}



void Draw_Window()
{
	DrawBar(0,0, Form.cwidth,TOOLBAR_H-2, panel_color);
	DrawBar(0,TOOLBAR_H-2, Form.cwidth,1, 0xD7D0D3);
	DrawBar(0,TOOLBAR_H-1, Form.cwidth,1, border_color);
	SetElementSizes();
	DrawRectangle(address_box.left-3, address_box.top-3, address_box.width+5, 25,border_color);
	DefineButton(address_box.left-52, address_box.top-2, 24, skin.h-2, BACK_BUTTON+BT_HIDE, 0);
	DefineButton(address_box.left-27, address_box.top-2, 24, skin.h-2, FORWARD_BUTTON+BT_HIDE, 0);
	img_draw stdcall(skin.image, address_box.left-53, address_box.top-3, 51, skin.h, 0, SKIN_Y);
	DefineButton(address_box.left+address_box.width+1, address_box.top-3, 16, skin.h-1, REFRESH_BUTTON+BT_HIDE+BT_NOFRAME, 0);
	DefineButton(Form.cwidth-27, address_box.top-3, 23, skin.h-1, SANDWICH_BUTTON+BT_HIDE, 0);
	img_draw stdcall(skin.image, Form.cwidth-24, address_box.top-3, 17, skin.h, 102, SKIN_Y);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,STATUSBAR_H, col_bg);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, border_color);
	if (!header) 
		OpenPage(); 
	else { 
		WB1.DrawPage(); 
		DrawEditBoxWebView(); 
	}
	DrawRectangle(scroll_wv.start_x, scroll_wv.start_y, scroll_wv.size_x, scroll_wv.size_y-1, scroll_wv.bckg_col);
	DrawProgress();
}


void ProcessEvent(dword id__)
{
	switch (id__)
	{
		case 1:
			ExitProcess();
			return;
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
			if (!editURL[0]) {
				strcpy(#URL, URL_SERVICE_HOME);
			}
			else if (!strncmp(#editURL,"http:",5)) || (editURL[0]=='/') 
			|| (!strncmp(#editURL,"https:",6)) || (!strncmp(#editURL,"WebView:",8))
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
		case REFRESH_BUTTON:
			if (http.transfer > 0) 
			{
				StopLoading();
				Draw_Window();
			}
			else OpenPage();
			return;
		case SANDWICH_BUTTON:
			EventShowPageMenu(Form.cwidth - 215, TOOLBAR_H-6);
			return;
		case VIEW_SOURCE:
			WB1.list.first = 0;
			ShowSource();
			WB1.LoadInternalPage(bufpointer, bufsize);
			break;
		case EDIT_SOURCE:
			if (!strncmp(#URL,"http",4)) 
			{
				CreateFile(bufsize, bufpointer, "/tmp0/1/WebView_tmp.htm");
				if (!EAX) RunProgram("/rd/1/tinypad", "/tmp0/1/WebView_tmp.htm");
			}
			else RunProgram("/rd/1/tinypad", #URL);
			return;
		// case FREE_IMG_CACHE:
		// 	ImgCache.Free();
		// 	notify(IMAGES_CACHE_CLEARED);
		// 	WB1.DrawPage();
		// 	return;
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
		case COPY_LINK_URL:
			Clipboard__CopyText(PageLinks.GetURL(PageLinks.active));
			notify("'URL copied to clipboard'O");
			return;
		case DOWNLOAD_LINK_CONTENTS:
			if (!downloader_opened) {
				strcpy(#downloader_edit, PageLinks.GetURL(PageLinks.active));
				CreateThread(#Downloader,#downloader_stak+4092);
			}
			return;
		case SCAN_CODE_F12:
			debug_mode ^= 1;
			if (debug_mode) notify("'Debug mode ON'-I");
			else notify("'Debug mode OFF'-I");
			return;
		case SCAN_CODE_F11:
			old_tag_parser_mode ^= 1;
			if (old_tag_parser_mode) notify("'Old tag parser ON'-I");
			else notify("'Old tag parser OFF'-I");
			return;
	}
}

void StopLoading()
{
	if (http.transfer)
	{
		EAX = http.transfer;
		EAX = EAX.http_msg.content_ptr;		// get pointer to data
		$push	EAX							// save it on the stack
		http_free stdcall (http.transfer);	// abort connection
		$pop	EAX							
		free(EAX);						// free data
		http.transfer=0;
		bufsize = 0;
		bufpointer = free(bufpointer);
	}
	wv_progress_bar.value = 0;
	DrawEditBoxWebView();
}

void SetPageDefaults()
{
	strcpy(#header, #version);
	WB1.list.count = WB1.list.first = 0;
	cur_encoding = CH_NULL;
	if (o_bufpointer) o_bufpointer = free(o_bufpointer);
}

void OpenPage()
{
	char getUrl[sizeof(URL)];
	StopLoading();
	souce_mode = false;
	strcpy(#editURL, #URL);
	history.add(#URL);
	if (!strncmp(#URL,"WebView:",8))
	{
		SetPageDefaults();
		if (!strcmp(#URL, URL_SERVICE_HOME)) WB1.LoadInternalPage(#homepage, sizeof(homepage));
		else if (!strcmp(#URL, URL_SERVICE_HISTORY)) ShowHistory();
		DrawEditBoxWebView();
		return;
	}
	if (!strncmp(#URL,"http:",5)) || (!strncmp(#URL,"https://",8)) 
	{
		img_draw stdcall(skin.image, address_box.left+address_box.width+1, address_box.top-3, 17, skin.h, 68, SKIN_Y);

		if (!strncmp(#URL,"http:",5)) {
			http.get(#URL);
		}
		if (!strncmp(#URL,"https://",8)) {
			sprintf(#getUrl, "http://gate.aspero.pro/?site=%s", #URL);
			http.get(#getUrl);
		}
		//http.get(#URL);
		if (!http.transfer)
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

DrawEditBoxWebView()
{
	int skin_x_offset;
	DrawBar(address_box.left-2, address_box.top-2, address_box.width+3, 2, address_box.color);
	DrawBar(address_box.left-2, address_box.top, 2, 22, address_box.color);
	//address_box.size = address_box.pos = address_box.shift = address_box.shift_old = strlen(#editURL);
	//address_box.offset = 0;
	EditBox_UpdateText(#address_box, address_box.flags);
	edit_box_draw stdcall(#address_box);
	if (http.transfer > 0) skin_x_offset = 68; else skin_x_offset = 51;
	img_draw stdcall(skin.image, address_box.left+address_box.width+1, address_box.top-3, 17, skin.h, skin_x_offset, SKIN_Y);
}


void ShowPage()
{
	DrawEditBoxWebView();
	if (!bufsize)
	{
		if (http.transfer) WB1.LoadInternalPage(#loading, sizeof(loading));
		else WB1.LoadInternalPage(#page_not_found, sizeof(page_not_found));
	}
	else
	{
		WB1.Prepare();
	}
}

byte UrlExtIs(dword ext)
{
	if (!strcmpi(#URL + strlen(#URL) - strlen(ext), ext)) return true;
	return false;
}

void DrawProgress()
{
	dword persent;
	if (http.transfer == 0) return;
	if (wv_progress_bar.max) persent = wv_progress_bar.value*100/wv_progress_bar.max; else persent = 10;
	DrawBar(address_box.left-2, address_box.top+20, persent*address_box.width/100, 2, wv_progress_bar.progress_color);
}


char anchor[256];
void ClickLink()
{
	if (http.transfer > 0) 
	{
		StopLoading();
		history.back();
	}
	strcpy(#URL, PageLinks.GetURL(PageLinks.active));
	//#1
	if (URL[0] == '#')
	{
		if (URL[1] == NULL) {
			WB1.list.first = 0;
			strcpy(#URL, history.current());
		}
		else {
			strlcpy(#anchor, #URL+strrchr(#URL, '#'), sizeof(anchor));
			strcpy(#URL, history.current());
		}
		ShowPage();			
		return;
	}
	//liner.ru#1
	if (strrchr(#URL, '#')!=0)
	{
		strlcpy(#anchor, #URL+strrchr(#URL, '#'), sizeof(anchor));
		URL[strrchr(#URL, '#')-1] = 0x00;
	}

	if (!strncmp(#URL,"mailto:", 7))
	{
		notify(#URL);
		strcpy(#editURL, history.current());
		strcpy(#URL, history.current());
		return;
	}

	GetAbsoluteURL(#URL);

	if (strncmp(#URL,"http://",7)!=0) && (strncmp(#URL,"https://",8)!=0)
	{
		if (UrlExtIs(".htm")!=true) && (UrlExtIs(".html")!=true)
		{	
			if (strchr(#URL, '|')) {
				ESBYTE[strchr(#URL, '|')] = NULL;
				RunProgram(#URL, strlen(#URL)+1+#URL);
			}
			else {
				RunProgram("/sys/@open", #URL);
			}
			strcpy(#editURL, history.current());
			strcpy(#URL, history.current());
			return;
		}
	}
	else	
	{
		if (UrlExtIs(".png")==true) || (UrlExtIs(".gif")==true) || (UrlExtIs(".jpg")==true) 
		|| (UrlExtIs(".zip")==true) || (UrlExtIs(".kex")==true) || (UrlExtIs(".pdf")==true)
		|| (UrlExtIs(".7z")==true) {
			if (!downloader_opened) {
				strcpy(#downloader_edit, #URL);
				CreateThread(#Downloader,#downloader_stak+4092);
				strcpy(#editURL, history.current());
				strcpy(#URL, history.current());
			}
			else notify("'WebView\nPlease, start a new download only when previous ended.'Et");
			return;
		}
	}
	if (open_in_a_new_window)
	{
		RunProgram(#program_path, #URL);
		strcpy(#editURL, history.current());
		strcpy(#URL, history.current());
	}
	else 
	{
		OpenPage();
	}
	open_in_a_new_window = false;
}

void EventShowPageMenu(dword _left, _top)
{
	menu.show(Form.left+_left-6,Form.top+_top+skin_height+3, 220, #rmb_menu, VIEW_SOURCE);
}

void EventShowLinkMenu(dword _left, _top)
{
	menu.show(Form.left+_left-6,Form.top+_top+skin_height+3, 220, #link_menu, COPY_LINK_URL);
}

void EventUpdateProgressBar()
{
	wv_progress_bar.max = http.content_length;
	if (wv_progress_bar.value != http.content_received)
	{
		wv_progress_bar.value = http.content_received;	
		DrawProgress();
	}
}

DrawStatusBar(dword _status_text)
{
	status_text.start_x = wv_progress_bar.left + wv_progress_bar.width + 10;
	status_text.start_y = Form.cheight - STATUSBAR_H + 3;
	status_text.area_size_x = Form.cwidth - status_text.start_x -3;
	DrawBar(status_text.start_x, status_text.start_y, status_text.area_size_x, 9, col_bg);
	status_text.text_pointer = _status_text;
	PathShow_prepare stdcall(#status_text);
	PathShow_draw stdcall(#status_text);
}

stop: