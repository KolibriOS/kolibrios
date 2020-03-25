//HTML Viewer in C--
//Copyright 2007-2020 by Veliant & Leency
//Asper, lev, Lrz, Barsuk, Nable, hidnplayr...

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

//libraries
#define MEMSIZE 1024 * 800
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
#include "..\lib\obj\proc_lib.h"
//useful patterns
#include "..\lib\patterns\history.h"
#include "..\lib\patterns\http_downloader.h"
#include "..\lib\patterns\simple_open_dialog.h"

#ifdef LANG_RUS
char version[]="Текстовый браузер 1.94";
#define T_LOADING "Загрузка страницы..."
#define T_RENDERING "Рендеринг..."
char page_not_found[] = FROM "html\\page_not_found_ru.htm""\0";
char homepage[] = FROM "html\\homepage_ru.htm""\0";
char help[] = FROM "html\\help_ru.htm""\0";
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
char version[]="Text-based Browser 1.94";
#define T_LOADING "Loading..."
#define T_RENDERING "Rendering..."
char page_not_found[] = FROM "html\\page_not_found_en.htm""\0";
char homepage[] = FROM "html\\homepage_en.htm""\0";
char help[] = FROM "html\\help_en.htm""\0";
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

dword col_bg = 0xE3E2E2;
dword panel_color  = 0xE3E2E2;
dword border_color = 0x8C8C8C;

bool debug_mode = false;
bool open_in_a_new_window = false;

_http http = {0, 0, 0, 0, 0, 0, 0};

#include "..\TWB\TWB.c"
#include "history.h"
#include "show_src.h"
#include "download_manager.h"

#define URL_SERVICE_HISTORY "WebView:history"
#define URL_SERVICE_HOMEPAGE "WebView:home"
#define URL_SERVICE_HELP "WebView:help"

dword TOOLBAR_H = 40;
dword STATUSBAR_H = 15;

int action_buf;

bool source_mode = false;

progress_bar wv_progress_bar;
char stak[4096];
proc_info Form;

enum { 
	BACK_BUTTON=1000, 
	FORWARD_BUTTON, 
	REFRESH_BUTTON, 
	GOTOURL_BUTTON, 
	SANDWICH_BUTTON,
	VIEW_SOURCE,
	EDIT_SOURCE,
	VIEW_HISTORY,
	DOWNLOAD_MANAGER,
	COPY_LINK_URL,
	DOWNLOAD_LINK_CONTENTS,
};

char default_dir[] = "/rd/1";
od_filter filter2 = { 16, "TXT\0HTM\0HTML\0\0" };

char editURL[sizeof(URL)];
edit_box address_box = {250,60,30,0xffffff,0x94AECE,0xffffff,0xffffff,0x10000000,sizeof(URL)-2,#editURL,0,NULL,19,19};

#define SKIN_Y 24

void LoadLibraries()
{
	load_dll(boxlib,    #box_lib_init,0);
	load_dll(libio,     #libio_init,1);
	load_dll(libimg,    #libimg_init,1);
	load_dll(libHTTP,   #http_lib_init,1);
	load_dll(iconv_lib, #iconv_open,0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);	
}

void HandleParam()
{
	if (param) {
		if (!strncmp(#param, "-d ", 3)) {
			strcpy(#downloader_edit, #param+3);
			CreateThread(#Downloader,#downloader_stak+4092);
			ExitProcess();
		} else if (!strncmp(#param, "-s ", 3)) {
			source_mode = true;
			strcpy(#URL, #param + 3);
		} else {
			strcpy(#URL, #param); 
		}
	} else {
		strcpy(#URL, URL_SERVICE_HOMEPAGE);
	}
}

void main()
{
	int redirect_count = 0;
	int i;
	LoadLibraries();
	CreateDir("/tmp0/1/downloads");
	Libimg_LoadImage(#skin, "/sys/toolbar.png");
	HandleParam();
	skin.h = 26;
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
			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) {
				if (key_scancode == SCAN_CODE_KEY_O) EventOpenDialog();
				if (key_scancode == SCAN_CODE_KEY_H) ProcessEvent(VIEW_HISTORY);
				if (key_scancode == SCAN_CODE_KEY_U) EventViewSource();
				if (key_scancode == SCAN_CODE_KEY_T) 
				|| (key_scancode == SCAN_CODE_KEY_N) RunProgram(#program_path, NULL);
				if (key_scancode == SCAN_CODE_KEY_W) ExitProcess();
				if (key_scancode == SCAN_CODE_KEY_J) ProcessEvent(DOWNLOAD_MANAGER);
				if (key_scancode == SCAN_CODE_KEY_R) ProcessEvent(REFRESH_BUTTON);
				if (key_scancode == SCAN_CODE_ENTER) EventSeachWeb();
				if (key_scancode == SCAN_CODE_LEFT)  ProcessEvent(BACK_BUTTON);
				if (key_scancode == SCAN_CODE_RIGHT) ProcessEvent(FORWARD_BUTTON);
			}
			
			if (key_scancode == SCAN_CODE_F5) ProcessEvent(REFRESH_BUTTON);
			
			if (address_box.flags & ed_focus)  
			{
				if (key_scancode == SCAN_CODE_ENTER) {
					ProcessEvent(key_scancode); 
				}
				else {
					EAX = key_editbox; 
					edit_box_key stdcall(#address_box);
				}
			}
			else 
			{
				#define KEY_SCROLL_N 11
				if (SCAN_CODE_UP   == key_scancode) for (i=0;i<KEY_SCROLL_N;i++) WB1.list.KeyUp();
				if (SCAN_CODE_DOWN == key_scancode) for (i=0;i<KEY_SCROLL_N;i++) WB1.list.KeyDown();
				if (key_scancode == SCAN_CODE_F6) {address_box.flags=ed_focus; DrawOmnibox();}
				if (WB1.list.ProcessKey(key_scancode)) WB1.DrawPage();
				else ProcessEvent(key_scancode);
			}
			break;

		case evReDraw:
			if (menu.cur_y) {
				ProcessEvent(menu.cur_y);
				menu.cur_y = 0;
			}
			DefineAndDrawWindow(GetScreenWidth()-800/2-random(80),GetScreenHeight()-700/2-random(80),800,700,0x73,0,0,0);
			GetProcessInfo(#Form, SelfInfo);
			system.color.get();
			col_bg = system.color.work;
			if (Form.status_window>2) { DrawTitle(#header); break; }
			if (Form.height<120) { MoveSize(OLD,OLD,OLD,120); break; }
			if (Form.width<280) { MoveSize(OLD,OLD,280,OLD); break; }
			draw_window();
			break;
			
		case evNetwork:
			if (http.transfer > 0) {
				http.receive();
				EventUpdateProgressBar();
				DrawStatusBar(T_LOADING);
				if (http.receive_result == 0) {
					// Handle redirects
					if (http.status_code >= 300) && (http.status_code < 400)
					{
						redirect_count++;
						if (redirect_count>5)
						{
							notify("'Too many redirects.' -E");
							StopLoading();
						}
						else
						{
							http.handle_redirect();
							http.free();
							GetAbsoluteURL(#http.redirect_url);
							debug("Redirect: "); debugln(#http.redirect_url);
							history.back();
							strcpy(#URL, #http.redirect_url);
							strcpy(#editURL, #URL);
							DrawOmnibox();
							OpenPage();
							//ProcessLink(history.current());
						}
						break;
					} 
					redirect_count = 0;
					// Loading the page is complete, free resources
					history.add(#URL);
					bufpointer = http.content_pointer;
					bufsize = http.content_received;
					http.free();
					SetPageDefaults();
					DrawStatusBar(T_RENDERING);
					ShowPage();
					DrawStatusBar(NULL);
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
	WB1.list.column_max = WB1.list.w - scroll_wv.size_x / WB1.list.font_w + 1;
	WB1.list.visible = WB1.list.h;
	if (WB1.list.w!=WB1.DrawBuf.bufw) {
		WB1.DrawBuf.Init(WB1.list.x, WB1.list.y, WB1.list.w, 800*20);
		ProcessEvent(REFRESH_BUTTON);
	}
}



void draw_window()
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
		DrawOmnibox(); 
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
			EventSubmitOmnibox();
			return;
		case REFRESH_BUTTON:
			if (http.transfer > 0) 
			{
				StopLoading();
				draw_window();
			}
			else OpenPage();
			return;
		case SANDWICH_BUTTON:
			EventShowPageMenu(Form.cwidth - 215, TOOLBAR_H-6);
			return;
		case VIEW_SOURCE:
			EventViewSource();
			break;
		case EDIT_SOURCE:
			if (check_is_the_adress_local(#URL)) {
				RunProgram("/rd/1/tinypad", #URL);
			} else {
				CreateFile(bufsize, bufpointer, "/tmp0/1/WebView_tmp.htm");
				if (!EAX) RunProgram("/rd/1/tinypad", "/tmp0/1/WebView_tmp.htm");
			}
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
		pause(10);
	}
	wv_progress_bar.value = 0;
	DrawOmnibox();
}

void SetPageDefaults()
{
	strcpy(#header, #version);
	WB1.list.count = WB1.list.first = 0;
	cur_encoding = CH_NULL;
	if (o_bufpointer) o_bufpointer = free(o_bufpointer);
}

void ReplaceSpaceInUrl() {
	int i;
	strcpy(#editURL, #URL);
	while (i = strchr(#URL, ' '))
	{
		i -= #URL;
		strlcpy(#URL+i+3, #editURL+i+1, sizeof(URL)-i-4);
		URL[i] = '%';
		URL[i+1] = '2';
		URL[i+2] = '0';
	}
	strcpy(#editURL, #URL);
}

void OpenPage()
{
	char getUrl[sizeof(URL)];
	StopLoading();
	strcpy(#editURL, #URL);
	history.add(#URL);
	if (!strncmp(#URL,"WebView:",8))
	{
		SetPageDefaults();
		if (!strcmp(#URL, URL_SERVICE_HOMEPAGE)) LoadInternalPage(#homepage, sizeof(homepage)-1);
		else if (!strcmp(#URL, URL_SERVICE_HELP)) LoadInternalPage(#help, sizeof(help)-1);
		else if (!strcmp(#URL, URL_SERVICE_HISTORY)) ShowHistory();
		else {bufsize=0; ShowPage();} //page not found
		DrawOmnibox();
		return;
	}
	if (!strncmp(#URL,"http:",5)) || (!strncmp(#URL,"https://",8)) 
	{
		ReplaceSpaceInUrl();
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
		if (EBX)
		{
			bufsize = EBX;
			free(bufpointer);
			bufpointer = malloc(bufsize);
			SetPageDefaults();
			ReadFile(0, bufsize, bufpointer, #URL);
		}
		ShowPage();
	}
}

void ProcessAnchor()
{
	char anchor[256];
	int anchor_pos;
	
	anchor_pos = strrchr(#URL, '#')-1;
	strlcpy(#anchor, #URL+anchor_pos, sizeof(anchor)-1);
	URL[anchor_pos] = 0x00;

	//#1
	if (URL[0] == NULL)
	{
		if (anchor[1] == NULL) {
			WB1.list.first = 0;
		}
		else {
			if (anchors.get_anchor_pos(#anchor+1)!=-1) WB1.list.first = anchors.get_anchor_pos(#anchor+1);
		}
		strcpy(#URL, history.current());
	}
	//liner.ru#1
	else
	{
		GetAbsoluteURL(#URL);
		OpenPage();
		if (anchors.get_anchor_pos(#anchor+1)!=-1) WB1.list.first = anchors.get_anchor_pos(#anchor+1);
	}

	WB1.DrawPage();
	strcpy(#editURL, #URL);
	strcat(#editURL, #anchor);
	DrawOmnibox();
}

void EventSubmitOmnibox()
{
	if (!editURL[0]) return;
	if (!strncmp(#editURL,"http:",5)) || (editURL[0]=='/') 
	|| (!strncmp(#editURL,"https:",6)) || (!strncmp(#editURL,"WebView:",8))
	{
		strcpy(#URL, #editURL);
	}
	else
	{
		strlcpy(#URL,"http://",7);
		strcat(#URL, #editURL);
	}
	ProcessLink();
}

void EventClickLink()
{
	strcpy(#URL, PageLinks.GetURL(PageLinks.active));
	GetAbsoluteURL(#URL);
	ProcessLink();
}

void ProcessLink()
{
	if (http.transfer > 0) 
	{
		StopLoading();
		history.back();
	}
	
	if (strrchr(#URL, '#')!=0) {
		ProcessAnchor();
		return;
	}

	if (!strncmp(#URL,"mailto:", 7)) || (!strncmp(#URL,"tel:", 4)) 
	{
		notify(#URL);
		strcpy(#editURL, history.current());
		strcpy(#URL, history.current());
		return;
	}

	if (!strncmp(#URL,"WebView:",8)) {
		OpenPage();
		return;
	}

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

void DrawOmnibox()
{
	int skin_x_offset;
	DrawBar(address_box.left-2, address_box.top-2, address_box.width+3, 2, address_box.color);
	DrawBar(address_box.left-2, address_box.top, 2, 22, address_box.color);
	if (address_box.flags & ed_focus) address_box.flags = ed_focus; else address_box.flags = 0;
	EditBox_UpdateText(#address_box, address_box.flags);
	edit_box_draw stdcall(#address_box);
	if (http.transfer > 0) skin_x_offset = 68; else skin_x_offset = 51;
	img_draw stdcall(skin.image, address_box.left+address_box.width+1, address_box.top-3, 17, skin.h, skin_x_offset, SKIN_Y);
}

void LoadInternalPage(dword bufpos, in_filesize){
	bufsize = in_filesize;
	bufpointer = bufpos;
	ShowPage();
}

void ShowPage()
{
	DrawOmnibox();
	if (!bufsize) || (!bufpointer) {
		LoadInternalPage(#page_not_found, sizeof(page_not_found)-1);
	}
	WB1.Prepare();
	if (source_mode) {
		source_mode = false;
		ShowSource();
		LoadInternalPage(bufpointer, bufsize);
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
	DrawBar(address_box.left-2, address_box.top+20, persent*address_box.width/100, 2, 0x72B7EB);
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

void EventSeachWeb()
{
	sprintf(#URL, "https://www.google.com/search?q=%s", #editURL);
	replace_char(#URL, ' ', '_', sizeof(URL));
	ProcessLink();
}

void EventOpenDialog()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		strcpy(#URL, #openfile_path);
		OpenPage();
	}
}

void EventViewSource()
{
	char source_view_param[sizeof(URL)+4];
	strcpy(#source_view_param, "-s ");
	strcat(#source_view_param, #URL);
	RunProgram(#program_path, #source_view_param);
}

void DrawStatusBar(dword _status_text)
{
	status_text.start_x = 10;
	status_text.start_y = Form.cheight - STATUSBAR_H + 3;
	status_text.area_size_x = Form.cwidth - status_text.start_x -3;
	DrawBar(status_text.start_x, status_text.start_y, status_text.area_size_x, 9, col_bg);
	status_text.text_pointer = _status_text;
	PathShow_prepare stdcall(#status_text);
	PathShow_draw stdcall(#status_text);
}

stop: