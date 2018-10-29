#define MEMSIZE 4096*100

#include "../lib/gui.h"
#include "../lib/kfont.h"
#include "../lib/io.h"
#include "../lib/cursor.h"

#include "../lib/obj/box_lib.h"
#include "../lib/obj/libini.h"
#include "../lib/obj/iconv.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/obj/http.h"

#include "../lib/patterns/simple_open_dialog.h"
#include "../lib/patterns/history.h"
#include "../lib/patterns/http_downloader.h"
#include "../browser/download_manager.h"

llist list;

#include "link.h"
#include "canvas.h"
#include "favicon.h"

char default_dir[] = "/rd/1";
od_filter filter2 = { 16, "TXT\0HTM\0HTML\0\0" };

char accept_language[]= "Accept-Language: ru\n";

#define TOOLBAR_H 36
#define TOOLBAR_ICON_WIDTH  26
#define TOOLBAR_ICON_HEIGHT 24
#define STATUSBAR_H 15

#define DEFAULT_EDITOR "/sys/tinypad"
#define DEFAULT_PREVIEW_PATH "/tmp0/1/aelia_preview.txt"

//ATTENTION: each page must have '\0' character at the end of the file
char buidin_page_home[] = FROM "buidin_pages\\home.htm";
char buidin_page_about[] = FROM "buidin_pages\\about.htm";
char buidin_page_not_found[] = FROM "buidin_pages\\not_found.htm";

#define UML 4096*2

scroll_bar scroll = { 15,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

proc_info Form;
char title[4196];

enum {
	OPEN_FILE,
	MAGNIFY_MINUS,
	MAGNIFY_PLUS,
	CHANGE_ENCODING,
	RUN_EDIT,
	GO_BACK,
	GO_FORWARD,
	SANDWICH
};

char address[UML];
edit_box address_box = {250,56,34,0xffffff,0x94AECE,0xffffff,0xffffff,0,UML,#address,NULL,2,19,19};

bool debug_mode=false;

#include "ini.h"
#include "gui.h"
#include "prepare_page.h"
//#include "special.h"

#define SANDWICH_MENU "Refresh page\nEdit page\nHistory\nDownloader\nAbout"

void InitDlls()
{
	load_dll(boxlib,    #box_lib_init,   0);
	load_dll(libHTTP,   #http_lib_init,  1);
	load_dll(libio,     #libio_init,     1);
	load_dll(libimg,    #libimg_init,    1);
	//load_dll(libini,    #lib_init,       1);
	load_dll(iconv_lib, #iconv_open,     0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
}


void main()
{   
	InitDlls();	
	OpenDialog_init stdcall (#o_dialog);
	LoadIniSettings();
	kfont.init(DEFAULT_FONT);
	Libimg_LoadImage(#skin, abspath("toolbar.png"));
	list.no_selection = true;
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER + EVM_STACK);
	loop()
	{
		switch(WaitEvent())
		{
			case evMouse:
				HandleMouseEvent();
				break;
			case evKey:
				HandleKeyEvent();
				break;
			case evButton:
				HandleButtonEvent();
				break;
			case evNetwork:
				HandleNetworkEvent();
				break;
			case evReDraw:
				draw_window();
				if (menu.cur_y>=10) && (menu.cur_y<20) {
					encoding = menu.cur_y - 10;
					EventPageRefresh();
					menu.cur_y = 0;
				}
				if (menu.cur_y>=20) {
					menu.cur_y-=20;
					if (menu.cur_y==0) EventPageRefresh();
					if (menu.cur_y==1) EventRunEdit();
					if (menu.cur_y==2) EventShowHistory();
					if (menu.cur_y==3) EventShowDownloader();
					if (menu.cur_y==4) EventShowInfo();
					menu.cur_y = 0;
				} 
		}
	}
}


void HandleButtonEvent()
{
	byte btn = GetButtonID();
	if (btn==1) {
		SaveIniSettings();
		ExitProcess();
	}
	switch(btn-10)
	{
		case GO_BACK:
			EventGoBack();
			break;
		case GO_FORWARD:
			EventGoForward();
			break;
		case OPEN_FILE:
			EventOpenDialog();
			break;
		case MAGNIFY_PLUS:
			EventMagnifyPlus();
			break;
		case MAGNIFY_MINUS:
			EventMagnifyMinus();
			break;
		case CHANGE_ENCODING:
			EventChangeEncoding();
			break;
		case RUN_EDIT:
			EventRunEdit();
			break;
		case SANDWICH:
			EventShowSandwichMenu();
			break;
	}
}


void HandleKeyEvent()
{
	GetKeys();
	if (key_modifier & KEY_LCTRL) || (key_modifier & KEY_RCTRL) {
		switch (key_scancode)
		{
			case SCAN_CODE_UP:
				EventMagnifyPlus();
				return;
			case SCAN_CODE_DOWN:
				EventMagnifyMinus();
				return;
			case SCAN_CODE_KEY_O:
				EventOpenDialog();
				return;
			case SCAN_CODE_KEY_E:
				EventRunEdit();
				return;
			case SCAN_CODE_KEY_H:
				EventShowHistory();
				return;
			case SCAN_CODE_TAB:
				EventChangeEncoding();
				return;
		}
	}
	switch (key_scancode) 
	{
		case SCAN_CODE_F1:
			EventShowInfo();
			return;
		case SCAN_CODE_F12:
			EventChangeDebugMode();
			return;
		case SCAN_CODE_ENTER:
			EventOpenAddress(#address);
			return;
		case SCAN_CODE_BS:
			if (! address_box.flags & ed_focus) {
				EventGoBack();
				return;
			}
	}
	if (list.ProcessKey(key_scancode)) && (! address_box.flags & ed_focus) {
		DrawPage();
		return;
	}
	if (key_ascii != ASCII_KEY_ENTER)
	&& (key_ascii != ASCII_KEY_PGDN) 
	&& (key_ascii != ASCII_KEY_PGUP) {
		EAX = key_editbox;
		edit_box_key stdcall(#address_box);
	}
}


void HandleMouseEvent()
{
	edit_box_mouse stdcall (#address_box);
	mouse.get();
	list.wheel_size = 7;

	if (link.hover(mouse.x, mouse.y)) {
		if (-1 == link.active) {
			DrawStatusBar( " " ); //just clean status bar	
		}
		else {
			DrawStatusBar( link.get_active_url() );
		}
	}

	if (mouse.key&MOUSE_LEFT) && (mouse.up) {
		if (-1 != link.active) EventOpenAddress( link.get_active_url() );
	}

	if (list.MouseScroll(mouse.vert)) {
		DrawPage(); 
		return; 
	}

	scrollbar_v_mouse (#scroll);
	if (list.first != scroll.position) {
		list.first = scroll.position;
		DrawPage(); 
	}
}

void HandleNetworkEvent()
{
	char favicon_address[UML];

	if (downloader.state == STATE_IN_PROGRESS) {
		downloader.MonitorProgress();

		if (downloader.httpd.content_length>0)
			DrawProgress(STEP_2_COUNT_PAGE_HEIGHT-STEP_1_DOWNLOAD_PAGE*
				downloader.httpd.content_received/downloader.httpd.content_length); 
		else
			DrawProgress(STEP_2_COUNT_PAGE_HEIGHT-STEP_1_DOWNLOAD_PAGE/2);		
	} 
	
	if (downloader.state == STATE_COMPLETED) 
	{
		if (!strncmp(downloader.url,"http://gate.aspero.pro/",22)) {
			strcpy(#address,downloader.url + 29);
		}
		else {
			strcpy(#address,downloader.url);
		}
		downloader.Stop();
		DrawAddressBox();
		io.buffer_data = downloader.bufpointer;
		/*
		get_absolute_url(#favicon_address, #address, "/favicon.ico");
		favicon.get(#favicon_address);
		*/
		PostOpenPageActions();
	}
}


/* ----------------------------------------------------- */

void EventOpenDialog()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) EventOpenAddress(#openfile_path);
}

void EventOpenAddress(dword _new_address)
{
char temp[UML];
char getUrl[UML];
	if (!ESBYTE[_new_address]) return;
	debugln("====================================");
	debug("address: "); 
	debugln(_new_address);
	strlcpy(#address, _new_address, UML);
	strlwr(#address);
	DrawAddressBox();

	/*
	There could be several possible types of addresses:
	- build in page
	- local file
	- url
	So we need to detect what incoming address is
	and then halndle it in the propper way.
	*/

	io.buffer_data = 0;
	favicon.get(NULL);

	// - build in page
	if (!strncmp(#address,"aelia:",6)) {
		debugln("this is buildin page");
		if (!strcmp(#address,"aelia:home")) io.buffer_data = #buidin_page_home;
		if (!strcmp(#address,"aelia:about")) io.buffer_data = #buidin_page_about;
		if (!strcmp(#address,"aelia:history")) io.buffer_data = MakePageWithHistory();
		PostOpenPageActions();
	}
	// - local file
	else if (check_is_the_adress_local(#address)==true) {
		debugln("this is local address");
		io.read(#address);
		PostOpenPageActions();
	}
	// - url
	else {
		debugln("this is url");
		if (!strncmp(#address,"https://",8)) {
			sprintf(#getUrl, "http://gate.aspero.pro/?site=%s", #address);
		}
		else if (!strncmp(#address,"http://",7)) {
			strlcpy(#getUrl, #address, UML);
		}
		else {
			strcpy(#temp, "http://");
			strlcpy(#temp, #address, UML);
			strlcpy(#address, #temp, UML);
			DrawAddressBox();
			strlcpy(#getUrl, #address, UML);
		}
		downloader.Start(#getUrl);
	}
}

void PostOpenPageActions()
{
	if (!io.buffer_data) {
		debugln("page not found");
		io.buffer_data = #buidin_page_not_found;
	}

	history.add(#address);
	favicon.draw(address_box.left-18, address_box.top-1);

	/* 
	Great! So we have the page in our buffer.
	We don't know is it a plain text or html.
	So we need to parse it and draw.
	*/

	list.KeyHome();
	PreparePage();
}

void EventMagnifyPlus()
{
	kfont.size.pt++;
	if(!kfont.changeSIZE())
		kfont.size.pt--;
	else
		PreparePage();
}

void EventMagnifyMinus()
{
	kfont.size.pt--;
	if(!kfont.changeSIZE())
		kfont.size.pt++;
	else
		PreparePage();
}

void EventRunEdit()
{
	if (check_is_the_adress_local(history.current())==true) {
		io.run(DEFAULT_EDITOR, history.current());
	}
	else {
		//io.write(strlen(io.buffer_data), io.buffer_data, DEFAULT_PREVIEW_PATH); // <--- doesn't work, smth odd, need to check
		CreateFile(strlen(io.buffer_data), io.buffer_data, DEFAULT_PREVIEW_PATH);
		io.run(DEFAULT_EDITOR, DEFAULT_PREVIEW_PATH);
	}
}

void EventChangeEncoding()
{
	menu.selected = encoding + 1;
	menu.show(Form.left+Form.cwidth-97,Form.top+TOOLBAR_H+skin_height-6, 130, "UTF-8\nKOI8-RU\nCP1251\nCP1252\nISO8859-5\nCP866", 10);
}

void EventShowInfo() {
	EventOpenAddress("aelia:about");
}

void EventShowHistory()
{
	EventOpenAddress("aelia:history");
}

void EventGoBack()
{
	if (history.back()) EventOpenAddress(history.current());
}

void EventGoForward()
{
	if (history.forward()) EventOpenAddress(history.current());
}

void EventShowSandwichMenu()
{
	menu.selected = 0;
	menu.show(Form.left+Form.cwidth-130,Form.top+TOOLBAR_H+skin_height-10, 130, SANDWICH_MENU, 20);
}

void EventPageRefresh()
{
	EventOpenAddress(history.current());
}

void EventShowDownloader()
{
	if (!downloader_opened) {
		downloader_edit = NULL;
		CreateThread(#Downloader,#downloader_stak+4092);
	}
}

void EventChangeDebugMode()
{
	debug_mode ^= 1;
	if (debug_mode) notify("'Debug mode ON'-I");
	else notify("'Debug mode OFF'-I");
	return;
}

/* ------------------------------------------- */


void draw_window()
{
	DefineAndDrawWindow(Form.left,Form.top,Form.width,Form.height,0x73,0,#title,0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;

	if (Form.width  < 200) { MoveSize(OLD,OLD,200,OLD); return; }
	if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); return; }

	system.color.get();

	list.SetSizes(0, TOOLBAR_H, Form.cwidth-scroll.size_x-1, 
		Form.cheight-TOOLBAR_H-STATUSBAR_H, kfont.size.pt+4);
	
	DrawBar(0, 0, Form.cwidth, TOOLBAR_H - 2, 0xe1e1e1);
	DrawBar(0, TOOLBAR_H - 2, Form.cwidth, 1, 0xcecece);
	DrawBar(0, TOOLBAR_H - 1, Form.cwidth, 1, 0x7F7F7F);
	
	DrawToolbarButton(GO_BACK,         8);
	DrawToolbarButton(GO_FORWARD,      33);
	DrawToolbarButton(OPEN_FILE,       68);
	DrawToolbarButton(MAGNIFY_PLUS,    Form.cwidth - 125);
	DrawToolbarButton(MAGNIFY_MINUS,   Form.cwidth - 100);
	DrawToolbarButton(CHANGE_ENCODING, Form.cwidth - 64);
	DrawToolbarButton(SANDWICH,        Form.cwidth - 31);

	DrawAddressBox();

	if ((Form.cwidth-scroll.size_x-1 == list.w) && 
		(Form.cheight-TOOLBAR_H == list.h) && 
		(list.count) 
	) 
	{
		DrawPage();
	}
	else
	{
		if (!kfont.raw) {                           //this code need to be run
			if (param) EventOpenAddress(#param);    //only once at browser sturtup
			else EventOpenAddress("aelia:home");
		}
		else PreparePage();
	}

	DrawRectangle(scroll.start_x, scroll.start_y, scroll.size_x, scroll.size_y-1, scroll.bckg_col);
	DrawStatusBar(NULL);
}

void DrawPage()
{
	list.CheckDoesValuesOkey();
	if (list.count) {
		kfont.ShowBufferPart(list.x, list.y, list.w, list.h, list.first*list.item_h*list.w);
	}
	DrawScroller();
}

void DrawAddressBox()
{
	address_box.left = 97+19;
	address_box.top = 11;
	address_box.width = Form.cwidth - address_box.left - 138;
	DrawRectangle(address_box.left-4-19, address_box.top-5, address_box.width+6+19, 23, 0x8C8C8C);
	DrawWideRectangle(address_box.left-3-19, address_box.top-3, address_box.width+5+19, 21, 4, address_box.color);
	address_box.size = address_box.pos = address_box.shift = address_box.shift_old = strlen(#address);
	address_box.offset = 0;
	edit_box_draw stdcall(#address_box);
	favicon.draw(address_box.left-18, address_box.top-1);
	DrawBar(address_box.left-2, address_box.top+1, 2, 13, 0xFFFfff);
}

PathShow_data status_text = {0, 17,250, 6, 250, 0, 0, 0x0, 0xFFFfff, 0, NULL, 0};
void DrawStatusBar(dword _status_text)
{
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,STATUSBAR_H, system.color.work);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, 0x8C8C8C);

	if (_status_text) {
		status_text.start_x = 7;
		status_text.start_y = Form.cheight - STATUSBAR_H + 3;
		status_text.area_size_x = Form.cwidth - status_text.start_x -3;
		status_text.font_color = system.color.work_text;
		status_text.text_pointer = _status_text;
		PathShow_prepare stdcall(#status_text);
		PathShow_draw stdcall(#status_text);
	}
}
