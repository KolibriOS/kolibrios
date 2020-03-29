//HTML Viewer in C--
//Copyright 2007-2020 by Veliant & Leency
//Asper, lev, Lrz, Barsuk, Nable, hidnplayr...

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

//libraries
#define MEMSIZE 1024 * 850
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

#include "show_src.h"
_http http = {0, 0, 0, 0, 0, 0, 0};
#include "download_manager.h"
_history history;
#include "history.h"

bool debug_mode = false;
dword col_bg = 0xE3E2E2;
dword panel_color  = 0xE3E2E2;
dword border_color = 0x787878;
#include "..\TWB\TWB.c"

#ifdef LANG_RUS
char version[]="Текстовый браузер 2.0 beta4";
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
char version[]="Text-based Browser 2.0 beta4";
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

#define URL_SIZE 4000

#define URL_SERVICE_HISTORY "WebView:history"
#define URL_SERVICE_HOMEPAGE "WebView:home"
#define URL_SERVICE_HELP "WebView:help"

#define TOOLBAR_GAPS 10
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

char editURL[URL_SIZE+1];
edit_box address_box = {NULL,TOOLBAR_GAPS+TOOLBAR_GAPS+51,10,0xffffff,0x94AECE,0xffffff,
	0xffffff,0x10000000,URL_SIZE-2,#editURL,0,NULL,19,19};

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
			history.add(#param + 3);
		} else {
			history.add(#param);
		}
	} else {
		history.add(URL_SERVICE_HOMEPAGE);
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
			if (PageLinks.HoverAndProceed(mouse.x, WB1.list.first + mouse.y, WB1.list.y, WB1.list.first))
			&& (mouse.pkm) && (mouse.up) {
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
				if (key_scancode == SCAN_CODE_KEY_O) {EventOpenDialog();break;}
				if (key_scancode == SCAN_CODE_KEY_H) {ProcessEvent(VIEW_HISTORY);break;}
				if (key_scancode == SCAN_CODE_KEY_U) {EventViewSource();break;}
				if (key_scancode == SCAN_CODE_KEY_T) 
				|| (key_scancode == SCAN_CODE_KEY_N) {RunProgram(#program_path, NULL);break;}
				if (key_scancode == SCAN_CODE_KEY_J) {ProcessEvent(DOWNLOAD_MANAGER);break;}
				if (key_scancode == SCAN_CODE_KEY_R) {ProcessEvent(REFRESH_BUTTON);break;}
				if (key_scancode == SCAN_CODE_ENTER) {EventSeachWeb();break;}
				if (key_scancode == SCAN_CODE_LEFT)  {ProcessEvent(BACK_BUTTON);break;}
				if (key_scancode == SCAN_CODE_RIGHT) {ProcessEvent(FORWARD_BUTTON);break;}
				if (key_scancode == SCAN_CODE_KEY_W) {ExitProcess();break;}
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
			DefineAndDrawWindow(GetScreenWidth()-800/2-random(80),
				GetScreenHeight()-700/2-random(80),800,700,0x73,0,0,0);
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
							GetAbsoluteURL(#http.redirect_url, history.current());
							debug("Redirect: "); debugln(#http.redirect_url);
							history.back();
							OpenPage(#http.redirect_url);
						}
						break;
					} 
					redirect_count = 0;
					// Loading the page is complete, free resources
					http.free();
					LoadInternalPage(http.content_pointer, http.content_received);
				}
			}
	}
}

void SetElementSizes()
{
	address_box.top = TOOLBAR_H/2-10;
	basic_line_h = calc(WB1.list.font_h * 130) / 100;
	address_box.width = Form.cwidth - address_box.left - 55;
	WB1.list.SetSizes(0, TOOLBAR_H, Form.width - 10 - scroll_wv.size_x, 
		Form.cheight - TOOLBAR_H - STATUSBAR_H, basic_line_h);
	WB1.list.wheel_size = 7 * basic_line_h;
	WB1.list.column_max = WB1.list.w - scroll_wv.size_x / WB1.list.font_w + 1;
	WB1.list.visible = WB1.list.h;
	if (WB1.list.w!=WB1.DrawBuf.bufw) {
		WB1.DrawBuf.Init(WB1.list.x, WB1.list.y, WB1.list.w, 400*20);
		OpenPage(history.current());
	}
}



void draw_window()
{
	DrawBar(0,0, Form.cwidth,TOOLBAR_H-2, panel_color);
	DrawBar(0,TOOLBAR_H-2, Form.cwidth,1, 0xD7D0D3);
	DrawBar(0,TOOLBAR_H-1, Form.cwidth,1, border_color);
	SetElementSizes();
	DefineHiddenButton(TOOLBAR_GAPS, address_box.top-2, 24, skin.h-2, BACK_BUTTON);
	DefineHiddenButton(TOOLBAR_GAPS+25, address_box.top-2, 24, skin.h-2, FORWARD_BUTTON);
	img_draw stdcall(skin.image, TOOLBAR_GAPS-1, address_box.top-3, 51, skin.h, 0, SKIN_Y);
	DefineHiddenButton(address_box.left+address_box.width-4, address_box.top-2, 20, skin.h-3, REFRESH_BUTTON);
	DefineHiddenButton(Form.cwidth-31, address_box.top-3, 24, skin.h-1, SANDWICH_BUTTON);
	img_draw stdcall(skin.image, Form.cwidth-27, address_box.top+1, 17, 18, 51, SKIN_Y);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,STATUSBAR_H, col_bg);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, border_color);
	if (!header) {
		OpenPage(history.current()); 
		WB1.DrawScroller();
	} else { 
		WB1.DrawPage(); 
		DrawOmnibox(); 
		DrawRectangle(scroll_wv.start_x, scroll_wv.start_y, scroll_wv.size_x, 
			scroll_wv.size_y-1, scroll_wv.bckg_col);
	}
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
				OpenPage(history.current());
			}
			return;
		case FORWARD_BUTTON:
			if (history.forward()) {
				OpenPage(history.current());
			}
			return;
		case GOTOURL_BUTTON:
		case SCAN_CODE_ENTER:
			EventSubmitOmnibox();
			return;
		case REFRESH_BUTTON:
			if (http.transfer > 0) {
				StopLoading();
				draw_window();
			} else {
				OpenPage(history.current());
			}
			return;
		case SANDWICH_BUTTON:
			EventShowPageMenu(Form.cwidth - 215, TOOLBAR_H-6);
			return;
		case VIEW_SOURCE:
			EventViewSource();
			break;
		case EDIT_SOURCE:
			if (check_is_the_adress_local(history.current())) {
				RunProgram("/rd/1/tinypad", history.current());
			} else {
				CreateFile(bufsize, bufpointer, "/tmp0/1/WebView_tmp.htm");
				if (!EAX) RunProgram("/rd/1/tinypad", "/tmp0/1/WebView_tmp.htm");
			}
			return;
		case VIEW_HISTORY:
			OpenPage(URL_SERVICE_HISTORY);
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
		pause(10);
	}
	wv_progress_bar.value = 0;
	DrawOmnibox();
}

//rewrite into 
//bool strrpl(dword dst, from, into, dst_len);
bool ReplaceSpaceInUrl(dword url, size) {
	unsigned int i, j;
	bool was_changed=false;
	for (i=url+size-3; i>url; i--)
	{
		if (ESBYTE[i]!=' ') continue;
		for (j=url+size-3; j>=i; j--) {
			ESBYTE[j+3]=ESBYTE[j+2];
			ESBYTE[j+2]=ESBYTE[j+1];
			ESBYTE[j+1]=ESBYTE[j];
		}
		ESBYTE[i] = '%';
		ESBYTE[i+1] = '2';
		ESBYTE[i+2] = '0';
		was_changed = true;
	}
	return was_changed;
}

bool GetLocalFileData(dword _path)
{
	dword data, size;
	file_size stdcall (_path);
	if (!EBX) {
		return false;
	} else {
		size = EBX;
		data = malloc(size);
		ReadFile(0, size, data, _path);
		LoadInternalPage(data, size);
		free(data);
		return true;
	}
}

void OpenPage(dword _open_URL)
{
	char new_url[URL_SIZE+1];

	StopLoading();

	strcpy(#editURL, _open_URL);
	DrawOmnibox();

	strncpy(#new_url, _open_URL, URL_SIZE);

	//Exclude # from the URL to the load page
	//We will bring it back when we get the buffer
	if (strrchr(#new_url, '#')) anchors.take_anchor_from(#new_url);

	history.add(#new_url);

	if (!strncmp(#new_url,"WebView:",8)) {
		//INTERNAL PAGE
		if (!strcmp(#new_url, URL_SERVICE_HOMEPAGE)) LoadInternalPage(#homepage, sizeof(homepage));
		else if (!strcmp(#new_url, URL_SERVICE_HELP)) LoadInternalPage(#help, sizeof(help));
		else if (!strcmp(#new_url, URL_SERVICE_HISTORY)) ShowHistory();
		else LoadInternalPage(#page_not_found, sizeof(page_not_found));
	} else if (!strncmp(#new_url,"http:",5)) || (!strncmp(#new_url,"https:",6)) {
		//WEB PAGE
		img_draw stdcall(skin.image, address_box.left+address_box.width+1, 
			address_box.top-3, 17, skin.h, 85, SKIN_Y);

		if (ReplaceSpaceInUrl(#new_url, URL_SIZE)) {
			strcpy(#editURL, #new_url);
			DrawOmnibox();
		}

		if (!strncmp(#new_url,"http:",5)) {
			http.get(#new_url);
		} else if (!strncmp(#new_url,"https://",8)) {
			strcpy(#new_url, "http://gate.aspero.pro/?site=");
			strncat(#new_url, _open_URL, URL_SIZE);
			http.get(#new_url);
		}
		if (!http.transfer) {
			StopLoading();
			LoadInternalPage(#page_not_found, sizeof(page_not_found));
		}
	} else {
		//LOCAL PAGE
		if (!GetLocalFileData(#new_url)) {
			LoadInternalPage(#page_not_found, sizeof(page_not_found));
		}
	}
}

void EventClickLink(dword _click_URL)
{
	char new_url[URL_SIZE+1];

	if (ESBYTE[_click_URL]=='#') {
		if (anchors.get_pos_by_name(_click_URL+1)!=-1) {
			WB1.list.first = anchors.get_pos_by_name(_click_URL+1);
			WB1.list.CheckDoesValuesOkey();
		}
		strcpy(#editURL, history.current());
		strcat(#editURL, _click_URL);
		DrawOmnibox();
		WB1.DrawPage();
		return;
	}

	if (!strncmp(_click_URL,"mailto:", 7)) || (!strncmp(_click_URL,"tel:", 4)) {
		notify(_click_URL);
		return;
	}

	if (http.transfer > 0) {
		StopLoading();
		history.back();
	}

	strcpy(#new_url, _click_URL);
	GetAbsoluteURL(#new_url, history.current());

	if (strrchr(#new_url, '#')!=0) {
		anchors.take_anchor_from(#new_url);
		OpenPage(#new_url);
		return;
	}

	if (!strncmp(#new_url,"WebView:",8)) {
		OpenPage(#new_url);
		return;
	}

	if (strncmp(#new_url,"http://",7)!=0) && (strncmp(#new_url,"https://",8)!=0)
	{
		if (UrlExtIs(#new_url,".htm")!=true) && (UrlExtIs(#new_url,".html")!=true)
		{	
			if (strchr(#new_url, '|')) {
				ESBYTE[strchr(#new_url, '|')] = NULL;
				RunProgram(#new_url, strlen(#new_url)+1+#new_url);
			} else {
				RunProgram("/sys/@open", #new_url);
			}
			return;
		}
	} else {
		if (UrlExtIs(#new_url,".png")==true) || (UrlExtIs(#new_url,".jpg")==true) 
		|| (UrlExtIs(#new_url,".zip")==true) || (UrlExtIs(#new_url,".kex")==true) || (UrlExtIs(#new_url,".pdf")==true)
		|| (UrlExtIs(#new_url,".7z")==true) {
			if (!downloader_opened) {
				strcpy(#downloader_edit, #new_url);
				CreateThread(#Downloader,#downloader_stak+4092);
			}
			else notify("'WebView\nPlease, start a new download only when previous ended.'Et");
			return;
		}
	}
	OpenPage(#new_url);
}

void EventSubmitOmnibox()
{
	char new_url[URL_SIZE+1];
	if (!editURL[0]) return;
	if (!strncmp(#editURL,"http:",5)) || (editURL[0]=='/') 
	|| (!strncmp(#editURL,"https:",6)) || (!strncmp(#editURL,"WebView:",8)) {
		OpenPage(#editURL);
	} else {
		strcpy(#new_url, "http://");
		strncat(#new_url, #editURL, sizeof(new_url)-1);
		OpenPage(#new_url);
	}
}

void DrawOmnibox()
{
	int skin_x_offset;
	
	DrawRectangle(address_box.left-2, address_box.top-3, address_box.width+5, 25,border_color);

	DrawBar(address_box.left-2, address_box.top-2, address_box.width+3, 1,0xD8DCD8);
	DrawBar(address_box.left-2, address_box.top-1, address_box.width+3, 1, address_box.color);
	img_draw stdcall(skin.image, address_box.left-2, address_box.top-3, 2, skin.h, 102, SKIN_Y);
	if (address_box.flags & ed_focus) address_box.flags = ed_focus; else address_box.flags = 0;
	EditBox_UpdateText(#address_box, address_box.flags);
	edit_box_draw stdcall(#address_box);
	if (http.transfer > 0) skin_x_offset = 85; else skin_x_offset = 68;
	img_draw stdcall(skin.image, address_box.left+address_box.width+1, 
		address_box.top-3, 17, skin.h, skin_x_offset, SKIN_Y);
}

void LoadInternalPage(dword _bufdata, _in_bufsize){
	if (!_bufdata) || (!_in_bufsize) {
		LoadInternalPage(#page_not_found, sizeof(page_not_found));
	} else {
		bufsize = _in_bufsize;
		if (bufpointer!=_bufdata) free(bufpointer);
		bufpointer = malloc(bufsize);
		memmov(bufpointer, _bufdata, bufsize);
		WB1.list.first = 0; //scroll page to the top
		DrawOmnibox();
		if(!strrchr(#editURL, '#')) {
			strcat(#editURL, #anchors.current);
			DrawOmnibox();
		}
		WB1.ParseHtml();
		if (source_mode) {
			source_mode = false;
			ShowSource(bufpointer, bufsize);
		} else {
			WB1.DrawPage();			
		}
	}
}

byte UrlExtIs(dword base, ext)
{
	if (!strcmpi(base + strlen(base) - strlen(ext), ext)) return true;
	return false;
}

void DrawProgress()
{
	dword persent;
	if (http.transfer == 0) return;
	if (wv_progress_bar.max) {
		persent = wv_progress_bar.value*100/wv_progress_bar.max;
	} else {
		persent = 10;
	}
	DrawBar(address_box.left-1, address_box.top+20, persent*address_box.width/100, 2, 0x72B7EB);
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
	char new_url[URL_SIZE+1];
	replace_char(#editURL, ' ', '_', URL_SIZE);
	strcpy(#new_url, "https://www.google.com/search?q=");
	strncat(#new_url, #editURL, URL_SIZE);
	OpenPage(#new_url);
}

void EventOpenDialog()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		OpenPage(#openfile_path);
	}
}

void EventViewSource()
{
	char source_view_param[URL_SIZE+1];
	strcpy(#source_view_param, "-s ");
	strncat(#source_view_param, history.current(), URL_SIZE);
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