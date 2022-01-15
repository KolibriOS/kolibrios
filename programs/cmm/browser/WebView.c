//Copyright 2007-2021 by Veliant & Leency
//Asper, lev, Lrz, Barsuk, Nable, hidnplayr...

//BUGS
//if maximize a window on image load => crash
//issues with a long line
//add proxy settings

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#define MEMSIZE 1024 * 160
#include "..\lib\gui.h"
#include "..\lib\draw_buf.h"
#include "..\lib\list_box.h"
#include "..\lib\cursor.h"
#include "..\lib\collection.h"
#include "..\lib\random.h"
#include "..\lib\clipboard.h"

#include "..\lib\obj\box_lib.h"
#include "..\lib\obj\libimg.h"
#include "..\lib\obj\http.h"
#include "..\lib\obj\iconv.h"
#include "..\lib\obj\proc_lib.h"
#include "..\lib\obj\netcode.h"

#include "..\lib\patterns\history.h"
#include "..\lib\patterns\simple_open_dialog.h"
#include "..\lib\patterns\toolbar_button.h"
#include "..\lib\patterns\restart_process.h"

#include "const.h"
#include "cache.h"
#include "show_src.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//
bool debug_mode = false;
bool show_images = true;
bool source_mode = false;
bool application_mode = false;

_history history;

enum { TARGET_SAME_TAB, TARGET_NEW_WINDOW, TARGET_NEW_TAB };

#include "TWB\TWB.c" //HTML Parser, a core component

TWebBrowser WB1;

#include "history.h"

dword PADDING = 9;
dword TSZE = 25;
dword STATUSBAR_H = 15;
dword TAB_H = 20;
dword TOOLBAR_H = 0;

_http http = 0;

progress_bar prbar;
proc_info Form;

#include "tabs.h"

dword cur_img_url;
dword shared_url;
dword http_get_type=PAGE;
dword render_start_time;
int menu_id=NULL;

char default_dir[] = "/sys";
od_filter filter2 = { 22, "TXT\0HTM\0HTML\0DOCX\0\0" };

char editURL[URL_SIZE+1];
edit_box omnibox_edit = {, 0, 0, 0xffffff,
	0x94AECE, 0xffffff, 0xffffff,0x10000000,URL_SIZE-2,#editURL,0,,19,19};

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void LoadLibraries()
{
	load_dll(boxlib,      #box_lib_init,0);
	load_dll(libimg,      #libimg_init,1);
	load_dll(libHTTP,     #http_lib_init,1);
	load_dll(iconv_lib,   #iconv_open,0);
	load_dll(netcode_lib, #base64_encode,0);
	load_dll(Proc_lib,    #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);	
}

void HandleParam()
{
	if (!param) {
		history.add(DEFAULT_URL);
	} else {
		if (!strncmp(#param, "-source ", 8)) {
			source_mode = true;
			history.add(#param + 8);
		} else if (!strncmp(#param, "-new ", 5)) {
			history.add(#param + 5);
		} else if (!strncmp(#param, "-app ", 5)) {
          	history.add(#param + 5);
          	application_mode = true;
        } else {
			if (GetProcessesCount("WEBVIEW") == 1) {
				history.add(#param);
			} else {
				shared_url = memopen(#webview_shared, URL_SIZE+1, SHM_OPEN + SHM_WRITE);
				strncpy(shared_url, #param, URL_SIZE);
				ExitProcess();
			}
		}
	}
	shared_url = memopen(#webview_shared, URL_SIZE+1, SHM_CREATE + SHM_WRITE);
	ESDWORD[shared_url] = '\0';
}

void main()
{
	int redirect_count=0;

	TOOLBAR_H = PADDING+TSZE+PADDING+2;

	LoadLibraries();
    HandleParam();

	if (application_mode) {
	    TOOLBAR_H = 0;
	    PADDING = 0;
        TSZE = 0;
        STATUSBAR_H = 0;
        TAB_H = 0;
	}

	omnibox_edit.left = PADDING+TSZE*2+PADDING+6;
	omnibox_edit.top = PADDING+3;

	WB1.list.SetFont(8, 14, 10011000b);
	WB1.list.no_selection = true;
	WB1.custom_encoding = -1;
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER + EVM_STACK);
	loop() switch(@WaitEventTimeout(30))
	{
		case evMouse:
			edit_box_mouse stdcall (#omnibox_edit);
			mouse.get();

			if (WB1.list.MouseScroll(mouse.vert)) WB1.DrawPage();

			if (WB1.list.count > WB1.list.visible) {
				scrollbar_v_mouse (#scroll_wv);
				if (WB1.list.first != scroll_wv.position) {
					WB1.list.first = scroll_wv.position;
					WB1.DrawPage();
					break;
				}
			}

			if (WB1.list.MouseOver(mouse.x, mouse.y)) && (links.hover(WB1.list.y, WB1.list.first))
			{
				if (mouse.key&MOUSE_MIDDLE) && (mouse.up) {
					GetKeyModifier();
					if (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) {
						EventClickLink(TARGET_NEW_WINDOW);
					} else {
						EventClickLink(TARGET_NEW_TAB);
					}
				}
				if (mouse.key&MOUSE_LEFT) && (mouse.up) { 
					CursorPointer.Restore();
					EventClickLink(TARGET_SAME_TAB);
				}
				if (mouse.key&MOUSE_RIGHT) && (mouse.up) {
					CursorPointer.Restore();
					EventShowLinkMenu();
				}
			} else {
				CursorPointer.Restore();
				if (mouse.key&MOUSE_RIGHT) && (mouse.up) && (WB1.list.MouseOver(mouse.x, mouse.y)) {
					EventShowPageMenu();
				}
			}
			break;

		case evButton:
			ProcessButtonClick( @GetButtonID() );
			break;

		case evKey:
			@GetKeys();
			edit_box_key stdcall(#omnibox_edit);
			ProcessKeyEvent();
			break;

		case evReDraw:
			DefineAndDrawWindow(GetScreenWidth()-WIN_W/2-random(80),GetScreenHeight()-WIN_H/2-random(80),
			//DefineAndDrawWindow(0,0,
				WIN_W,WIN_H,0x73,0,0,0);
			GetProcessInfo(#Form, SelfInfo);
			ProcessMenuClick();
			sc.get();
			if (Form.status_window>2) break;
			if (Form.height<120) { MoveSize(OLD,OLD,OLD,120); break; }
			if (Form.width<280) { MoveSize(OLD,OLD,280,OLD); break; }
			draw_window();
			break;
			
		case evNetwork:
			if (http.transfer <= 0) break;
			http.receive();

			if (http_get_type==PAGE) {
				CheckContentType();				
				prbar.max = http.content_length;
				if (prbar.value != http.content_received) {
					prbar.value = http.content_received;	
					DrawProgress();
				}
			}

			if (http.receive_result != 0) break;
			if (debug_mode) {
				EAX = http.transfer;
				debugln(#EAX.http_msg.http_header);
			}
			if (http.status_code >= 300) && (http.status_code < 400)
			{
				// Handle redirects
				if (redirect_count<=5) {
					redirect_count++;
					HandleRedirect();
				} else {
					StopLoading();
					redirect_count = 0;
					if (http_get_type==IMG) goto _IMG_RES;
					notify("'Too many redirects.' -E");
				}
			} else {
				// Loading the page is complete, free resources
				redirect_count = 0;
				if (http_get_type==PAGE) {
					history.add(http.cur_url);
					if (!strchr(http.cur_url, '?')) {
						cache.add(http.cur_url, http.content_pointer, http.content_received, PAGE, WB1.custom_encoding);
					}
					LoadInternalPage(http.content_pointer, http.content_received);
					free(http.content_pointer);
					DrawOmnibox();
				}
				else if (http_get_type==IMG) {
					_IMG_RES:
					if (http.status_code >= 200) && (http.status_code < 300) {
						cache.add(cur_img_url, http.content_pointer, http.content_received, IMG, NULL);
					} else {
						cache.add(cur_img_url, 0, 0, IMG, NULL);
					}
					http.hfree();
					free(http.content_pointer);
					GetImg(false);
				}
			}
			break;
		default:
			if (ESDWORD[shared_url] != '\0') {
				EventOpenNewTab(shared_url);
				ESDWORD[shared_url] = '\0';
				ActivateWindow(GetProcessSlot(Form.ID));
			}
	}
}


//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void ProcessButtonClick(dword id__)
{
	switch (id__)
	{
		case 1: ExitProcess();
		case TAB_CLOSE_ID...TAB_CLOSE_ID+TABS_MAX: EventTabClose(id__ - TAB_CLOSE_ID); return;
		case TAB_ID...TAB_ID+TABS_MAX: EventAllTabsClick(id__ - TAB_ID); return;
		case ENCODINGS...ENCODINGS+6: EventManuallyChangeEncoding(id__-ENCODINGS); return;
		case NEW_WINDOW:       RunProgram(#program_path, NULL); return;
		case NEW_TAB:          if (!http.transfer) EventOpenNewTab(URL_SERVICE_HOMEPAGE); return;
		case SCAN_CODE_BS:
		case BACK_BUTTON:      if (history.back()) OpenPage(history.current()); return;
		case FORWARD_BUTTON:   if (history.forward()) OpenPage(history.current()); return;
		case GOTOURL_BUTTON:   EventSubmitOmnibox();	return;
		case REFRESH_BUTTON:   EventRefreshPage(); return;
		case CHANGE_ENCODING:  EventShowEncodingsList(); return;
		case SANDWICH_BUTTON:  EventShowMainMenu(); return;
		case VIEW_SOURCE:      EventViewSource(); return;
		case EDIT_SOURCE:      EventEditSource(); return;
		case VIEW_HISTORY:     OpenPage(URL_SERVICE_HISTORY); return;
		case DOWNLOAD_MANAGER: EventOpenDownloader(""); return;
		case UPDATE_BROWSER:   EventUpdateBrowser(); return;
		case CLEAR_CACHE:      EventClearCache(); return;
		case IN_NEW_TAB:       EventClickLink(TARGET_NEW_TAB); return;
		case IN_NEW_WINDOW:    EventClickLink(TARGET_NEW_WINDOW); return;
		case COPY_LINK_URL:    EventCopyLinkToClipboard(); return;
		case DOWNLOAD_LINK_CT: EventOpenDownloader( GetAbsoluteActiveURL() ); return;
		case OPEN_FILE:        EventOpenDialog(); return;
	}
}

void ProcessKeyEvent()
{
	if (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) 
	{
		if (key_scancode == SCAN_CODE_TAB) {EventActivatePreviousTab();return;}
		if (key_scancode == SCAN_CODE_KEY_T) {EventOpenNewTab(URL_SERVICE_TEST);return;}
	}

	if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) switch(key_scancode) 
	{
		case SCAN_CODE_KEY_O: EventOpenDialog(); return;
		case SCAN_CODE_KEY_H: ProcessButtonClick(VIEW_HISTORY); return;
		case SCAN_CODE_KEY_U: EventViewSource(); return;
		case SCAN_CODE_KEY_T: EventOpenNewTab(URL_SERVICE_HOMEPAGE); return;
		case SCAN_CODE_KEY_N: RunProgram(#program_path, NULL); return;
		case SCAN_CODE_KEY_J: ProcessButtonClick(DOWNLOAD_MANAGER); return;
		case SCAN_CODE_KEY_R: ProcessButtonClick(REFRESH_BUTTON); return;
		case SCAN_CODE_ENTER: EventSeachWeb(); return;
		case SCAN_CODE_LEFT:  ProcessButtonClick(BACK_BUTTON); return;
		case SCAN_CODE_RIGHT: ProcessButtonClick(FORWARD_BUTTON); return;
		case SCAN_CODE_KEY_W: EventCloseActiveTab(); return;
		case SCAN_CODE_TAB:   EventActivateNextTab(); return;
		case SCAN_CODE_F5:    EventClearCache(); return;
		default: return;
	}

	switch(key_scancode) 
	{
		case SCAN_CODE_UP:    EventScrollUpAndDown(SCAN_CODE_UP); return;
		case SCAN_CODE_DOWN:  EventScrollUpAndDown(SCAN_CODE_DOWN); return;
		case SCAN_CODE_F6:    {omnibox_edit.flags=ed_focus; DrawOmnibox();} return;
		case SCAN_CODE_F5:    EventRefreshPage(); return;
		case SCAN_CODE_ENTER: if (omnibox_edit.flags & ed_focus) EventSubmitOmnibox(); return;
		case SCAN_CODE_F12:   EventToggleDebugMode(); return;
		case SCAN_CODE_F11:   show_images^=1; EventClearCache(); return;
		default:              if (WB1.list.ProcessKey(key_scancode)) WB1.DrawPage(); return;
	}
}

void SetElementSizes()
{
	omnibox_edit.width = Form.cwidth - omnibox_edit.left - 52 - 16;
	if (Form.cwidth - scroll_wv.size_x != WB1.list.w) {
		//temporary fix for crash
		//related to 'cur_img_url' var read
		//http://board.kolibrios.org/viewtopic.php?f=1&t=1712&start=60#p77523
		StopLoading();
	}
	WB1.list.SetSizes(0, TOOLBAR_H+TAB_H, Form.cwidth - scroll_wv.size_x, 
		Form.cheight - TOOLBAR_H - STATUSBAR_H - TAB_H, BASIC_LINE_H);
	WB1.list.wheel_size = 7 * BASIC_LINE_H;
	WB1.list.column_max = WB1.list.w - scroll_wv.size_x / WB1.list.font_w + 1;
	WB1.list.visible = WB1.list.h;
}


void draw_window()
{
	bool burger_active = false;
	if (menu_id == OPEN_FILE) burger_active = true;

	SetElementSizes();

    if (!application_mode) {
        DrawBar(0,0, Form.cwidth,PADDING, sc.work);
        DrawBar(0,PADDING+TSZE+1, Form.cwidth,PADDING-1, sc.work);
        DrawBar(0,TOOLBAR_H-2, Form.cwidth,1, MixColors(sc.dark, sc.work, 180));
        DrawBar(0,TOOLBAR_H-1, Form.cwidth,1, sc.line);
        DrawBar(0, PADDING, omnibox_edit.left-2, TSZE+1, sc.work);
        DrawBar(omnibox_edit.left+omnibox_edit.width+18, PADDING, Form.cwidth-omnibox_edit.left-omnibox_edit.width-18, TSZE+1, sc.work);

        DrawTopPanelButton(BACK_BUTTON, PADDING-1, PADDING, 30, false);
        DrawTopPanelButton(FORWARD_BUTTON, PADDING+TSZE+PADDING-2, PADDING, 31, false);
        DrawTopPanelButton(SANDWICH_BUTTON, Form.cwidth-PADDING-TSZE-3, PADDING, -1, burger_active); //burger menu

        DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, sc.line);

        DrawRectangle(WB1.list.x + WB1.list.w, WB1.list.y, scroll_wv.size_x,
		WB1.list.h-1, scroll_wv.bckg_col);
	}

	if (!canvas.bufw) {
		EventOpenFirstPage();
	} else {
		WB1.DrawPage(); 
		DrawOmnibox();
	}
	if (!application_mode) {
	    DrawProgress();
	    DrawStatusBar(NULL);
        DrawTabsBar();
	}

}

void EventOpenFirstPage()
{
	OpenPage(history.current());
}

void EventManuallyChangeEncoding(int _new_encoding)
{
	dword newbuf, newsize;
	WB1.custom_encoding = _new_encoding;
	newsize = strlen(WB1.o_bufpointer);
	newbuf = malloc(newsize);
	memmov(newbuf, WB1.o_bufpointer, newsize);
	LoadInternalPage(newbuf, newsize);
	free(newbuf);
}


void EventScrollUpAndDown(int _direction)
{
	int i;
	for (i=0;i<WB1.list.item_h*2;i++) {
		if (_direction == SCAN_CODE_UP) WB1.list.KeyUp(); 
		if (_direction == SCAN_CODE_DOWN) WB1.list.KeyDown(); 
	} 
	WB1.DrawPage();
}

void EventToggleDebugMode()
{
	debug_mode ^= 1;
	if (debug_mode) notify("'Debug mode ON'-I");
	else notify("'Debug mode OFF'-I");
}

void EventAllTabsClick(dword _n)
{
	if (mouse.mkm) {
		StopLoading();
		EventTabClose(_n);
	} else {
		if (!http.transfer) EventTabClick(_n);
	}
}

void EventEditSource()
{
	if (check_is_the_adress_local(history.current())) {
		RunProgram("/sys/develop/cedit", history.current());
	} else {
		CreateFile(WB1.bufsize, WB1.bufpointer, "/tmp0/1/WebView_tmp.htm");
		if (!EAX) RunProgram("/sys/develop/cedit", "/tmp0/1/WebView_tmp.htm");
	}
}

void EventClearCache()
{
	cache.clear();
	notify(#clear_cache_ok);
	EventRefreshPage();
}

void EventCopyLinkToClipboard()
{
	Clipboard__CopyText(GetAbsoluteActiveURL()); 
	notify("'URL copied to clipboard'O");
}

void StopLoading()
{
	if (http.stop()) pause(10);
	prbar.value = 0;
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

bool HandleUrlFiles(dword _path, _data)
{
	dword url_from_file;
	if (!UrlExtIs(_path, "url")) return false;
	if (! url_from_file = strstri(_data, "URL=")) return false;
	replace_char(url_from_file, '\n', '\0', strlen(url_from_file));
	OpenPage(url_from_file); 	
	return true;	
}

bool GetLocalFileData(dword _path)
{
	dword data, size;
	read_file(_path, #data, #size);
	if (!HandleUrlFiles(_path, data)) {
		LoadInternalPage(data, size);
	}
	free(data);
	return true;
}

bool GetUrl(dword _http_url)
{
	char new_url_full[URL_SIZE+1];

	if (!strncmp(_http_url,"http:",5)) {
		http.get(_http_url);
		return true;
	} else if (!strncmp(_http_url,"https://",8)) {
		strcpy(#new_url_full, "http://gate.aspero.pro/?site=");
		strncat(#new_url_full, _http_url, URL_SIZE);
		http.get(#new_url_full);
		return true;
	}
	return false;
}

void OpenPage(dword _open_URL)
{
	char new_url[URL_SIZE+1];
	int unz_id;

	StopLoading();

	SetOmniboxText(_open_URL);

	strncpy(#new_url, _open_URL, URL_SIZE);

	//Exclude # from the URL to the load page
	//We will bring it back when we get the buffer
	if (strrchr(#new_url, '#')) {
		anchors.take_anchor_from(#new_url);
	}

	/*
	There could be several possible types of addresses:
	- cached page (only http/https)
	- internal page
	- web page
	- local file
	So we need to detect what incoming address is
	and then halndle it in the propper way.
	*/

	if (cache.has(#new_url)) {
		//CACHED PAGE
		if (cache.current_type==PAGE) {
			history.add(#new_url);
			WB1.custom_encoding = cache.current_charset;
			LoadInternalPage(cache.current_buf, cache.current_size);
		}
		else {
			EventDownloadAndOpenImage(#new_url);
		}

	} else if (!strncmp(#new_url,"WebView:",8)) {
		//INTERNAL PAGE
		history.add(#new_url);
		WB1.custom_encoding = -1;
		if (streq(#new_url, URL_SERVICE_HOMEPAGE)) LoadInternalPage(#buildin_page_home, sizeof(buildin_page_home));
		else if (streq(#new_url, URL_SERVICE_HELP)) LoadInternalPage(#buildin_page_help, sizeof(buildin_page_help));
		else if (streq(#new_url, URL_SERVICE_TEST)) LoadInternalPage(#buildin_page_test, sizeof(buildin_page_test));
		else if (streq(#new_url, URL_SERVICE_HISTORY)) ShowHistory();
		else LoadInternalPage(#buildin_page_error, sizeof(buildin_page_error));

	} else if (!strncmp(#new_url,"http:",5)) || (!strncmp(#new_url,"https:",6)) {
		//WEB PAGE
		if (ReplaceSpaceInUrl(#new_url, URL_SIZE)) {
			strcpy(#editURL, #new_url);
		}

		http_get_type = PAGE;
		GetUrl(#new_url);

		DrawOmnibox();

		if (!http.transfer) {
			history.add(#new_url);
			LoadInternalPage(#buildin_page_error, sizeof(buildin_page_error));
		}
	} else {
		//LOCAL PAGE
		history.add(#new_url);
		if (UrlExtIs(#new_url,".docx")) {
			DeleteFile("/tmp0/1/temp/word/document.xml");
			CreateDir("/tmp0/1/temp");
			unz_id = RunProgram("/sys/unz", sprintf(#param, "-o \"/tmp0/1/temp\" -h \"%s\"", #new_url));
			while (GetProcessSlot(unz_id)) pause(2);
			strcpy(#new_url, "/tmp0/1/temp/word/document.xml");
		} 
		if (!GetLocalFileData(#new_url)) {
			LoadInternalPage(#buildin_page_error, sizeof(buildin_page_error));
		}
	}
}

dword EventOpenDownloader(dword _url)
{
	//char download_params[URL_SIZE+50];
	return RunProgram("/sys/network/dl", _url);
}

bool EventClickAnchor()
{
	dword aURL = links.active_url;

	if (anchors.get_pos_by_name(aURL+1)!=-1) {
		WB1.list.first = anchors.get_pos_by_name(aURL+1);
		WB1.list.CheckDoesValuesOkey();
		strcpy(#editURL, history.current());
		strcat(#editURL, aURL);
		DrawOmnibox();
		WB1.DrawPage();
		return true;
	}
	return false;
}

void EventClickLink(dword _target)
{
	char new_url[URL_SIZE+1];
	char new_url_full[URL_SIZE+1];
	dword aURL = GetAbsoluteActiveURL();
	if (!aURL) return;

	strcpy(#new_url, aURL);

	if (ESBYTE[aURL]=='#') {
		if (_target == TARGET_SAME_TAB) {
			EventClickAnchor(); 
			return;
		} else {
			strcpy(#new_url, history.current());
			strcat(#new_url, aURL);
		}
	}

	if (_target == TARGET_NEW_TAB) {
		EventOpenNewTab(#new_url);
		return;
	}

	if (_target == TARGET_NEW_WINDOW) {
		strcpy(#new_url_full, "-new ");
		strncat(#new_url_full, #new_url, URL_SIZE);
		RunProgram(#program_path, #new_url_full);
		return;
	}

	if (!strncmp(#new_url,"mailto:", 7)) || (!strncmp(#new_url,"tel:", 4)) {
		notify(#new_url);
		return;
	}

	if (http.transfer) {
		StopLoading();
	}

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
		strncat(#new_url, #editURL, URL_SIZE-1);
		OpenPage(#new_url);
	}
}

void LoadInternalPage(dword _bufdata, _in_bufsize){
	if (!_bufdata) || (!_in_bufsize) {
		LoadInternalPage(#buildin_page_error, sizeof(buildin_page_error));
	} else {
		WB1.list.first = 0; //scroll page to the top
		DrawOmnibox();
		if(!strrchr(#editURL, '#')) {
			strcat(#editURL, #anchors.current);
			DrawOmnibox();
		}
		render_start_time = GetStartTime();
		WB1.ParseHtml(_bufdata, _in_bufsize);
		// REJECTED. Reason: infinite redirect at Google Results.
		/*
		if (WB1.redirect) { //<meta http-equiv="refresh" content="0; url=http://site.com">
			get_absolute_url(#WB1.redirect, history.current());
			history.back();
			OpenPage(#WB1.redirect);
		}
		*/
		DrawStatusBar(NULL);
		DrawActiveTab();
		if (source_mode) {
			source_mode = false;
			WB1.custom_encoding = CH_CP866;
			ShowSource(WB1.bufpointer, _in_bufsize);
		} else {
			WB1.DrawPage();
		}
		http.hfree();
		if (WB1.img_url.count) { GetImg(true); DrawOmnibox(); }
	}
}

bool UrlExtIs(dword base, ext)
{
	if (!strcmpi(base + strlen(base) - strlen(ext), ext)) return true;
	return false;
}

void DrawProgress()
{
	dword pct;
	if (application_mode) return;
	if (!http.transfer) return;
	if (http_get_type==PAGE) && (prbar.max) pct = prbar.value*30/prbar.max; else pct = 10;
	if (http_get_type==IMG) pct = prbar.value * 70 / prbar.max + 30;
	DrawBar(omnibox_edit.left-1, omnibox_edit.top+20, pct*omnibox_edit.width+16/100, 2, 0x72B7EB);
}

void EventShowPageMenu()
{
    if (application_mode) return;
	open_lmenu(mouse.x, mouse.y, MENU_TOP_LEFT, NULL, #rmb_menu);
	menu_id = VIEW_SOURCE;
}

void EventShowLinkMenu()
{
    if (application_mode) return;
	open_lmenu(mouse.x, mouse.y, MENU_TOP_LEFT, NULL, #link_menu);
	menu_id = IN_NEW_TAB;
}

void EventShowMainMenu()
{
    if (application_mode) return;
	open_lmenu(Form.cwidth - PADDING -4, PADDING + TSZE + 3, 
		MENU_TOP_RIGHT, NULL, #main_menu);
	menu_id = OPEN_FILE;
}

void EventShowEncodingsList()
{
    if (application_mode) return;
	open_lmenu(Form.cwidth-4, Form.cheight - STATUSBAR_H + 12, 
		MENU_BOT_RIGHT, WB1.cur_encoding + 1, 
		"UTF-8\nKOI8-RU\nCP1251\nCP1252\nISO8859-5\nCP866");
	menu_id = ENCODINGS;
}

void ProcessMenuClick()
{
	int click_id;
	if (menu_id) {
		if (click_id = get_menu_click()) {
			click_id += menu_id - 1;
			ProcessButtonClick(click_id);
		}
		if (!menu_process_id) menu_id = NULL;
	}
}

void EventSeachWeb()
{
	char new_url[URL_SIZE+1];
	replace_char(#editURL, ' ', '+', URL_SIZE);
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
	source_mode = true;
	EventOpenNewTab(history.current());
}

void EventRefreshPage()
{
	if (http.transfer) {
		StopLoading();
		draw_window();
	} else {
		OpenPage(history.current());
	}
}

void EventUpdateBrowser()
{
	dword downloader_id, slot_n;
	dword current_size;
	dword new_size;

	draw_window();

	downloader_id = EventOpenDownloader(#update_param);
	do {
		slot_n = GetProcessSlot(downloader_id);
		pause(10);
	} while (slot_n!=0);

	current_size = get_file_size(#program_path);
	new_size = get_file_size("/tmp0/1/Downloads/WebView.com");

	if (!new_size) || (new_size<5000) { notify(#update_download_error); return; }
	if (current_size == new_size) { notify(#update_is_current);	return; }

	if (CopyFileAtOnce(new_size, "/tmp0/1/Downloads/WebView.com", #program_path)) {
		notify(#update_can_not_copy);
	} else {
		notify(#update_ok);
		RunProgram(#program_path, history.current());
		ExitProcess();
	}
}

void DrawStatusBar(dword _msg)
{
	dword status_y = Form.cheight - STATUSBAR_H + 4;
	dword status_w = Form.cwidth - 90;
	if (application_mode) return;
	if (Form.status_window>2) return;
	DrawBar(0,Form.cheight - STATUSBAR_H+1, Form.cwidth,STATUSBAR_H-1, sc.work);
	if (_msg) {
		ESI = math.min(status_w/6, strlen(_msg));
		WriteText(10, status_y, 0, sc.work_text, _msg);
	}
	DefineHiddenButton(status_w+20, status_y-3, 60, 12, CHANGE_ENCODING);
	WriteTextCenter(status_w+20, status_y, 60, sc.work_text, WB1.cur_encoding*10+#charsets);
}

void DrawOmnibox()
{
	int imgxoff;
	if (application_mode) return;
	DrawOvalBorder(omnibox_edit.left-2, omnibox_edit.top-3, omnibox_edit.width+18, 24, sc.line, 
		sc.line, sc.line, sc.dark);
	DrawBar(omnibox_edit.left-1, omnibox_edit.top-2, omnibox_edit.width+18, 1, 0xD8DCD8);
	DrawBar(omnibox_edit.left-1, omnibox_edit.top-1, omnibox_edit.width+18, 1, omnibox_edit.bg_color);
	DrawBar(omnibox_edit.left-1, omnibox_edit.top, 1, 22, omnibox_edit.bg_color);

	if (omnibox_edit.flags & ed_focus) omnibox_edit.flags = ed_focus; else omnibox_edit.flags = 0;
	EditBox_UpdateText(#omnibox_edit, omnibox_edit.flags);
	edit_box_draw stdcall(#omnibox_edit);
	if (http.transfer) imgxoff = 16*23*3; else imgxoff = 0;
	PutImage(omnibox_edit.left+omnibox_edit.width+1, omnibox_edit.top-1, 16, 23, imgxoff + #editbox_icons);
	DefineHiddenButton(omnibox_edit.left+omnibox_edit.width-1, omnibox_edit.top-2, 17, 23, REFRESH_BUTTON);

	DrawProgress();
}

void SetOmniboxText(dword _text)
{
    if (application_mode) return;
    edit_box_set_text stdcall (#omnibox_edit, _text);
    omnibox_edit.pos = omnibox_edit.flags = 0;
    DrawOmnibox();
}

dword GetAbsoluteActiveURL()
{
	char abs_url[URL_SIZE];
	if (links.active_url) {
		strncpy(#abs_url, links.active_url, URL_SIZE);
		get_absolute_url(#abs_url, history.current());		
		return #abs_url;
	}
	return 0;
}

void CheckContentType()
{
	char content_type[64];
	if (http.header_field("content-type", #content_type, sizeof(content_type))) // application || image

	if (content_type[0] == 'i') {
		EventDownloadAndOpenImage(http.cur_url);
		StopLoading();
	}
	else if (content_type[0] == 'a') { 
		EventOpenDownloader(http.cur_url);
		StopLoading();
	}
	else {
		WB1.custom_encoding = -1;
		if (EAX = strchr(#content_type, '=')) {
			WB1.custom_encoding = get_encoding_type_by_name(EAX+1);	
		}
	}
}

void EventDownloadAndOpenImage(dword _url)
{
	char image_download_url[URL_SIZE];
	strcpy(#image_download_url, "-eo ");
	strncat(#image_download_url, _url, URL_SIZE);
	EventOpenDownloader(#image_download_url);
}

void HandleRedirect()
{
	dword redirect_url = malloc(URL_SIZE);
	http.header_field("location", redirect_url, URL_SIZE);
	get_absolute_url(redirect_url, http.cur_url);
	http.hfree();
	if (http_get_type==PAGE) OpenPage(redirect_url);
	else if (http_get_type==IMG) GetUrl(redirect_url);
	free(redirect_url);
}

dword GetImg(bool _new)
{
	int i;
	if (!show_images) return;
	http_get_type = IMG;

	for (i = 0; i < WB1.img_url.count; i++)
	{
		cur_img_url = WB1.img_url.get(i);
		if (debug_mode) 
			{debug("get img: ");debugln(cur_img_url);}
		if (cache.has(cur_img_url)==false) {
			prbar.max = WB1.img_url.count;
			prbar.value = i;
			if (GetUrl(cur_img_url)) {DrawStatusBar(cur_img_url); DrawProgress(); return;}
		}
	}
	if (_new) return;
	DrawOmnibox();
	DrawStatusBar(T_RENDERING);
	WB1.Reparse();
	WB1.DrawPage();
	debugln(sprintf(#param, T_DONE_IN_SEC, GetStartTime()-render_start_time/100));
	DrawStatusBar(NULL);
}

stop:
