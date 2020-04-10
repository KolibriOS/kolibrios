//Copyright 2007-2020 by Veliant & Leency
//Asper, lev, Lrz, Barsuk, Nable, hidnplayr...

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

//libraries
#define MEMSIZE 1024 * 1000
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
#include "..\lib\patterns\toolbar_button.h"

#include "texts.h"
#include "cache.h"
#include "show_src.h"
#include "download_manager.h"

bool debug_mode = false;

#include "..\TWB\TWB.c" //HTML Parser, a core component

TWebBrowser WB1;
_history history;

#include "history.h"

#define PADDING 9
#define SKIN_Y 24
#define TSZE 25
#define STATUSBAR_H 15
#define TAB_H 20
dword TOOLBAR_H = PADDING+TSZE+PADDING+2;

#define URL_SIZE 4000

int action_buf;

_http http = 0;

bool source_mode = false;

progress_bar wv_progress_bar;
char stak[4096];
proc_info Form;

enum { 
	NEW_TAB=600,
	ENCODINGS=700,
	BACK_BUTTON=800, 
	FORWARD_BUTTON, 
	REFRESH_BUTTON, 
	GOTOURL_BUTTON, 
	CHANGE_ENCODING,
	SANDWICH_BUTTON,
	VIEW_SOURCE,
	EDIT_SOURCE,
	OPEN_FILE,
	NEW_WINDOW,
	VIEW_HISTORY,
	DOWNLOAD_MANAGER,
	CLEAR_CACHE,
	UPDATE_BROWSER,
	COPY_LINK_URL,
	DOWNLOAD_LINK_CONTENTS,
	TAB_ID,
	TAB_CLOSE_ID = 900
};

#include "tabs.h"

char default_dir[] = "/rd/1";
od_filter filter2 = { 16, "TXT\0HTM\0HTML\0\0" };

char editURL[URL_SIZE+1];
edit_box address_box = {, PADDING+TSZE*2+PADDING+6, PADDING+3, 0xffffff,
	0x94AECE, 0xffffff, 0xffffff,0x10000000,URL_SIZE-2,#editURL,0,,19,19};


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
		if (!strncmp(#param, "-download_and_exit ", 19)) {
			download_and_exit = true;
			strcpy(#downloader_edit, #param+19);
			Downloader();
			ExitProcess();
		} else if (!strncmp(#param, "-download ", 10)) {
			strcpy(#downloader_edit, #param+10);
			//CreateThread(#Downloader,#downloader_stak+4092);
			Downloader();
			ExitProcess();
		} else if (!strncmp(#param, "-source ", 8)) {
			source_mode = true;
			history.add(#param + 8);
		} else {
			history.add(#param);
		}
	} else {
		history.add(URL_SERVICE_HOMEPAGE);
	}
}

void main()
{
	int i, btn, redirect_count=0;
	LoadLibraries();
	CreateDir("/tmp0/1/Downloads");
	//CreateDir("/tmp0/1/WebView_Cache");
	Libimg_LoadImage(#skin, "/sys/toolbar.png");
	HandleParam();
	skin.h = 26;
	WB1.list.SetFont(8, 14, 10011000b);
	WB1.list.no_selection = true;
	WB1.custom_encoding = -1;
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER + EVM_STACK);
	loop() switch(WaitEvent())
	{
		case evMouse:
			edit_box_mouse stdcall (#address_box);
			mouse.get();
			if (PageLinks.HoverAndProceed(mouse.x, WB1.list.first + mouse.y, WB1.list.y, WB1.list.first))
			&& (mouse.pkm) && (mouse.up) {
				if (WB1.list.MouseOver(mouse.x, mouse.y)) EventShowPageMenu();
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
			if (mouse.up) && (! address_box.flags & ed_focus) && (address_box.flags & ed_shift_bac) 
			{
				DrawOmnibox(); //reset text selection
			}
			break;

		case evButton:
			btn = GetButtonID();
			if (1==btn) ExitProcess(); else ProcessEvent(btn);
			break;

		case evKey:
			GetKeys();
			//if (key_scancode == SCAN_CODE_F1) {DebugTabs();break;}

			if (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) {
				if (key_scancode == SCAN_CODE_TAB) {EventActivatePreviousTab();break;}
			}

			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) {
				if (key_scancode == SCAN_CODE_KEY_O) EventOpenDialog();
				else if (key_scancode == SCAN_CODE_KEY_H) ProcessEvent(VIEW_HISTORY);
				else if (key_scancode == SCAN_CODE_KEY_U) EventViewSource();
				else if (key_scancode == SCAN_CODE_KEY_T) EventOpenNewTab(URL_SERVICE_HOMEPAGE); 
				else if (key_scancode == SCAN_CODE_KEY_N) RunProgram(#program_path, NULL);
				else if (key_scancode == SCAN_CODE_KEY_J) ProcessEvent(DOWNLOAD_MANAGER);
				else if (key_scancode == SCAN_CODE_KEY_R) ProcessEvent(REFRESH_BUTTON);
				else if (key_scancode == SCAN_CODE_ENTER) EventSeachWeb();
				else if (key_scancode == SCAN_CODE_LEFT)  ProcessEvent(BACK_BUTTON);
				else if (key_scancode == SCAN_CODE_RIGHT) ProcessEvent(FORWARD_BUTTON);
				else if (key_scancode == SCAN_CODE_KEY_W) EventCloseActiveTab();
				else if (key_scancode == SCAN_CODE_TAB)   EventActivateNextTab();
				break;
			}
			
			if (key_scancode == SCAN_CODE_F5) ProcessEvent(REFRESH_BUTTON);
			
			if (address_box.flags & ed_focus)  
			{
				if (key_scancode == SCAN_CODE_ENTER) {
					ProcessEvent(key_scancode); 
				} else {
					EAX = key_editbox; 
					edit_box_key stdcall(#address_box);
				}
			} else {
				#define KEY_SCROLL_N 11
				if (SCAN_CODE_UP   == key_scancode) for (i=0;i<KEY_SCROLL_N;i++) WB1.list.KeyUp();
				if (SCAN_CODE_DOWN == key_scancode) for (i=0;i<KEY_SCROLL_N;i++) WB1.list.KeyDown();
				if (key_scancode == SCAN_CODE_F6) {address_box.flags=ed_focus; DrawOmnibox();}
				if (WB1.list.ProcessKey(key_scancode)) WB1.DrawPage();
				else ProcessEvent(key_scancode);
			}
			break;

		case evReDraw:
			DefineAndDrawWindow(40, //GetScreenWidth()-800/2-random(80),
				GetScreenHeight()-700/2-random(80),800,700,0x73,0,0,0);
			GetProcessInfo(#Form, SelfInfo);
			system.color.get();
			if (Form.status_window>2) break;
			if (Form.height<120) { MoveSize(OLD,OLD,OLD,120); break; }
			if (Form.width<280) { MoveSize(OLD,OLD,280,OLD); break; }
			draw_window();
			break;
			
		case evNetwork:
			if (http.transfer <= 0) break;
			http.receive();
			EventUpdateProgressBar();
			if (http.receive_result != 0) break;
			if (http.status_code >= 300) && (http.status_code < 400)
			{
				// Handle redirects
				if (redirect_count<=5) {
					redirect_count++;
					http.handle_redirect();
					http.free();
					GetAbsoluteURL(#http.redirect_url, history.current());
					history.back();
					OpenPage(#http.redirect_url);
				} else {
					notify("'Too many redirects.' -E");
					StopLoading();
				}
			} else {
				// Loading the page is complete, free resources
				redirect_count = 0;
				http.free();
				pages_cache.add(history.current(), http.content_pointer, http.content_received);
				LoadInternalPage(http.content_pointer, http.content_received);
			}
	}
}

void SetElementSizes()
{
	address_box.width = Form.cwidth - address_box.left - 52 - 16;
	WB1.list.SetSizes(0, TOOLBAR_H+TAB_H, Form.width - 10 - scroll_wv.size_x, 
		Form.cheight - TOOLBAR_H - STATUSBAR_H - TAB_H, BASIC_LINE_H);
	WB1.list.wheel_size = 7 * BASIC_LINE_H;
	WB1.list.column_max = WB1.list.w - scroll_wv.size_x / WB1.list.font_w + 1;
	WB1.list.visible = WB1.list.h;
}


void draw_window()
{
	int i;
	if (menu.cur_y) {
		EAX = menu.cur_y;
		menu.cur_y = 0;
		ProcessEvent(EAX);
	}
	SetElementSizes();

	DrawBar(0,0, Form.cwidth,PADDING, system.color.work);
	DrawBar(0,PADDING+TSZE+1, Form.cwidth,PADDING-1, system.color.work);
	DrawBar(0,TOOLBAR_H-2, Form.cwidth,1, MixColors(system.color.work_dark, system.color.work, 180));
	DrawBar(0,TOOLBAR_H-1, Form.cwidth,1, system.color.work_graph);
	DrawBar(0, PADDING, address_box.left-2, TSZE+1, system.color.work);
	DrawBar(address_box.left+address_box.width+18, PADDING, Form.cwidth-address_box.left-address_box.width-18, TSZE+1, system.color.work);

	DrawTopPanelButton(BACK_BUTTON, PADDING-1, PADDING, 30);
	DrawTopPanelButton(FORWARD_BUTTON, PADDING+TSZE+PADDING-2, PADDING, 31);
	DrawTopPanelButton(SANDWICH_BUTTON, Form.cwidth-PADDING-TSZE-3, PADDING, -1);
	for (i=0; i<=2; i++) DrawBar(Form.cwidth-PADDING-TSZE+3, i*5+PADDING+7, 15, 3, system.color.work_graph);

	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, system.color.work_graph);

	DrawRectangle(WB1.list.x + WB1.list.w, WB1.list.y, scroll_wv.size_x, 
		WB1.list.h-1, scroll_wv.bckg_col);

	if (!BrowserWidthChanged()) { 
		WB1.DrawPage(); 
		DrawOmnibox(); 
	}
	DrawProgress();
	DrawStatusBar(NULL);
	DrawTabsBar();
}

bool BrowserWidthChanged()
{
	dword source_mode_holder;
	if (WB1.list.w!=DrawBuf.bufw) {
		DrawBuf.Init(WB1.list.x, WB1.list.y, WB1.list.w, 400*20);
		if (!strncmp(history.current(),"http",4)) {
			//nihuya ne izyashnoe reshenie, no pust' poka butet tak
			source_mode_holder = source_mode;
			LoadInternalPage(#loading_text, sizeof(loading_text));
			source_mode = source_mode_holder;
		}
		OpenPage(history.current());
		return true;
	}
	return false;
}


void EventChangeEncodingAndLoadPage(int _new_encoding)
{
	dword newbuf, newsize;
	WB1.custom_encoding = _new_encoding;
	newsize = strlen(WB1.o_bufpointer);
	newbuf = malloc(newsize);
	memmov(newbuf, WB1.o_bufpointer, newsize);
	LoadInternalPage(newbuf, newsize);
	free(newbuf);
}


void ProcessEvent(dword id__)
{
	switch (id__)
	{
		case ENCODINGS...ENCODINGS+6:
			EventChangeEncodingAndLoadPage(id__-ENCODINGS);
			return;
		case NEW_WINDOW:
			RunProgram(#program_path, NULL);
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
			if (http.transfer) {
				StopLoading();
				draw_window();
			} else {
				OpenPage(history.current());
			}
			return;
		case CHANGE_ENCODING:
			EventShowEncodingsList();
			return;
		case SANDWICH_BUTTON:
			EventShowMainMenu();
			return;
		case VIEW_SOURCE:
			EventViewSource();
			break;
		case EDIT_SOURCE:
			if (check_is_the_adress_local(history.current())) {
				RunProgram("/rd/1/tinypad", history.current());
			} else {
				CreateFile(WB1.bufsize, WB1.bufpointer, "/tmp0/1/WebView_tmp.htm");
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
		case UPDATE_BROWSER:
			EventUpdateBrowser();
			return;
		case CLEAR_CACHE:
			pages_cache.clear();
			notify(#clear_cache_ok);
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
		case OPEN_FILE:
			EventOpenDialog();
			return;
		case SCAN_CODE_F12:
			debug_mode ^= 1;
			if (debug_mode) notify("'Debug mode ON'-I");
			else notify("'Debug mode OFF'-I");
			return;
		case NEW_TAB:
			if (http.transfer) break;
			EventOpenNewTab(URL_SERVICE_HOMEPAGE);
			return;
		case TAB_ID...TAB_ID+TABS_MAX:
			if (http.transfer) break;
			if (mouse.mkm) {
				EventTabClose(id__ - TAB_ID);
			} else {
				EventTabClick(id__ - TAB_ID);
			}
			return;
		case TAB_CLOSE_ID...TAB_CLOSE_ID+TABS_MAX:
			EventTabClose(id__ - TAB_CLOSE_ID);
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

	if (open_new_tab) {
		open_new_tab = false;
		EventOpenNewTab(_open_URL);
		return;
	}

	SetOmniboxText(_open_URL);

	strncpy(#new_url, _open_URL, URL_SIZE);

	//Exclude # from the URL to the load page
	//We will bring it back when we get the buffer
	if (strrchr(#new_url, '#')) anchors.take_anchor_from(#new_url);

	history.add(#new_url);

	if (pages_cache.has(#new_url)) {
		//CACHED PAGE
		LoadInternalPage(pages_cache.current_page_buf, pages_cache.current_page_size);

	} else if (!strncmp(#new_url,"WebView:",8)) {
		//INTERNAL PAGE
		if (!strcmp(#new_url, URL_SERVICE_HOMEPAGE)) LoadInternalPage(#homepage, sizeof(homepage));
		else if (!strcmp(#new_url, URL_SERVICE_HELP)) LoadInternalPage(#help, sizeof(help));
		else if (!strcmp(#new_url, URL_SERVICE_HISTORY)) ShowHistory();
		else LoadInternalPage(#page_not_found, sizeof(page_not_found));
		
	} else if (!strncmp(#new_url,"http:",5)) || (!strncmp(#new_url,"https:",6)) {
		//WEB PAGE
		if (ReplaceSpaceInUrl(#new_url, URL_SIZE)) {
			strcpy(#editURL, #new_url);
		}

		if (!strncmp(#new_url,"http:",5)) {
			http.get(#new_url);
		} else if (!strncmp(#new_url,"https://",8)) {
			strcpy(#new_url, "http://gate.aspero.pro/?site=");
			strncat(#new_url, _open_URL, URL_SIZE);
			http.get(#new_url);
		}

		DrawOmnibox();

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

	if (open_new_window) {
		strcpy(#new_url, _click_URL);
		GetAbsoluteURL(#new_url, history.current());
		RunProgram(#program_path, #new_url);
		return;
	}

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

	if (http.transfer) {
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

void LoadInternalPage(dword _bufdata, _in_bufsize){
	if (!_bufdata) || (!_in_bufsize) {
		LoadInternalPage(#page_not_found, sizeof(page_not_found));
	} else {
		WB1.list.first = 0; //scroll page to the top
		DrawOmnibox();
		if(!strrchr(#editURL, '#')) {
			strcat(#editURL, #anchors.current);
			DrawOmnibox();
		}
		WB1.ParseHtml(_bufdata, _in_bufsize);
		DrawStatusBar(NULL);
		DrawActiveTab();
		if (source_mode) {
			source_mode = false;
			ShowSource(WB1.bufpointer, _in_bufsize);
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
	DrawBar(address_box.left-1, address_box.top+20, persent*address_box.width+16/100, 2, 0x72B7EB);
}

void EventShowPageMenu()
{
	menu.selected = 0;
	menu.show(Form.left + mouse.x+4, Form.top + skin_height + mouse.y, 220, #rmb_menu, VIEW_SOURCE);
}

void EventShowLinkMenu()
{
	menu.selected = 0;
	menu.show(Form.left + mouse.x+4, Form.top + skin_height + mouse.y, 220, #link_menu, COPY_LINK_URL);
}

void EventShowEncodingsList()
{
	menu.selected = WB1.cur_encoding + 1;
	menu.show(Form.left + Form.cwidth - 95, Form.top + skin_height + status_text.start_y - 121, 90, 
		"UTF-8\nKOI8-RU\nCP1251\nCP1252\nISO8859-5\nCP866", ENCODINGS);
}

void EventShowMainMenu()
{
	menu.selected = 0;
	menu.show(Form.left + Form.cwidth - 150, Form.top + skin_height + TOOLBAR_H-8, 140, #main_menu, OPEN_FILE);
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
	//strcpy(#source_view_param, "-source ");
	//strncat(#source_view_param, history.current(), URL_SIZE);
	//RunProgram(#program_path, #source_view_param);
	source_mode = true;
	EventOpenNewTab(history.current());
}

dword GetFileSize(dword _path)
{
	BDVK bdvk;
	if (GetFileInfo(_path, #bdvk)!=0) {
		return 0;
	} else {
		return bdvk.sizelo;
	}
}

void EventUpdateBrowser()
{
	dword downloader_id, slot_n;
	dword current_size;
	dword new_size;
	int error;

	draw_window();

	downloader_id = RunProgram(#program_path, #update_param);
	do {
		slot_n = GetProcessSlot(downloader_id);
		pause(10);
	} while (slot_n!=0);

	current_size = GetFileSize(#program_path);
	new_size = GetFileSize("/tmp0/1/Downloads/WebView.com");

	if (!new_size) || (new_size<5000) {
		notify(#update_download_error);
		return;
	}

	if (current_size == new_size) {
		notify(#update_is_current);
		return;
	}

	if (error = CopyFileAtOnce(new_size, "/tmp0/1/Downloads/WebView.com", #program_path)) {
		notify(#update_can_not_copy);
	} else {
		notify(#update_ok);
		RunProgram(#program_path, history.current());
		ExitProcess();
	}
}

void DrawStatusBar(dword _status_text)
{
	status_text.font_color = system.color.work_text;
	status_text.start_x = 10;
	status_text.start_y = Form.cheight - STATUSBAR_H + 4;
	status_text.area_size_x = Form.cwidth - status_text.start_x -3 - 70;
	//DrawBar(status_text.start_x, status_text.start_y, status_text.area_size_x, 9, system.color.work);
	DrawBar(0,Form.cheight - STATUSBAR_H+1, Form.cwidth,STATUSBAR_H-1, system.color.work);
	if (_status_text) {
		status_text.text_pointer = _status_text;
		PathShow_prepare stdcall(#status_text);
		PathShow_draw stdcall(#status_text);		
	}
	DefineHiddenButton(status_text.start_x+status_text.area_size_x+10, status_text.start_y-3,
		60, 12, CHANGE_ENCODING);
	WriteTextCenter(status_text.start_x+status_text.area_size_x+10,
		status_text.start_y, 60, system.color.work_text, WB1.cur_encoding*10+#charsets);
}

void DrawOmnibox()
{
	int skin_x_offset;
	
	DrawOvalBorder(address_box.left-2, address_box.top-3, address_box.width+18, 24, system.color.work_graph, 
		system.color.work_graph, system.color.work_graph, system.color.work_dark);
	DrawBar(address_box.left-1, address_box.top-2, address_box.width+18, 1, 0xD8DCD8);
	DrawBar(address_box.left-1, address_box.top-1, address_box.width+18, 1, address_box.color);
	DrawBar(address_box.left-1, address_box.top, 1, 22, address_box.color);

	if (address_box.flags & ed_focus) address_box.flags = ed_focus; else address_box.flags = 0;
	EditBox_UpdateText(#address_box, address_box.flags);
	edit_box_draw stdcall(#address_box);
	if (http.transfer > 0) skin_x_offset = 85; else skin_x_offset = 68;
	img_draw stdcall(skin.image, address_box.left+address_box.width+1, 
		address_box.top-1, 16, skin.h-3, skin_x_offset, SKIN_Y+2);
	DefineHiddenButton(address_box.left+address_box.width-1, address_box.top-2, 17, skin.h-3, REFRESH_BUTTON);

	DrawProgress();
}

void SetOmniboxText(dword _text)
{
	strcpy(#editURL, _text);
	address_box.flags=0;
	DrawOmnibox();
}


stop: