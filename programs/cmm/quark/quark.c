/* 
	Quark Code Edit
	Author: Kiril Lipatov aka Leency
	Licence: GPLv2

	The core components of this app are:
		1. list: text grid with keyboard and mouse events
		2. lines: the mas of pointers for each line start
		3. selection
*/

#define MEMSIZE 1024*100

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/io.h"
#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/draw_buf.h"
#include "../lib/events.h"
#include "../lib/array.h"
#include "../lib/clipboard.h"
#include "../lib/math.h"

#include "../lib/obj/box_lib.h"
#include "../lib/obj/libini.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/iconv.h"
#include "../lib/obj/proc_lib.h"

#include "../lib/patterns/simple_open_dialog.h"
#include "../lib/patterns/toolbar_button.h"

//===================================================//
//                                                   //
//                 INTERNAL INCLUDES                 //
//                                                   //
//===================================================//

proc_info Form;
llist list;

#define TOOLBAR_H 38
#define TOOLBAR_ICON_WIDTH  24
#define TOOLBAR_ICON_HEIGHT 22
#define STATUSBAR_H 15
#define TAB_H 20

int user_encoding;
int real_encoding = CH_CP866;
int curcol_scheme;
int font_size;

bool enable_edit = false;
bool search_next = false;

#include "data.h"

#include "search.h"
#include "selection.h"
#include "prepare_page.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

scroll_bar scroll = { 15,200,398,44,0,2,115,15,0,0xeeeeee,
	0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

char title[4196];

int reopenin_mx,
    theme_mx,
    burger_mx,
    search_mx;

enum {
	CHANGE_CHARSET=12,
	REOPEN_IN_APP=1,
	COLOR_SCHEME=8,
	RMB_MENU,
	BTN_FIND_NEXT,
	BTN_FIND_CLOSE,
	BTN_CHANGE_CHARSET
};

dword menu_id;

EVENTS button;
EVENTS key;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void InitDlls()
{
	load_dll(boxlib,    #box_lib_init,   0);
	load_dll(libio,     #libio_init,     1);
	load_dll(libimg,    #libimg_init,    1);
	load_dll(libini,    #lib_init,       1);
	load_dll(iconv_lib, #iconv_open,     0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
}

void LoadFileFromDocPack()
{
	dword bufsize = atoi(#param + 1) + 20;
	dword bufpointer = malloc(bufsize);

	ESDWORD[bufpointer+0] = 0;
	ESDWORD[bufpointer+4] = 8;
	IpcSetArea(bufpointer, bufsize);

	SetEventMask(EVM_IPC);
	if (@WaitEventTimeout(200) != evIPC) {
		notify("'IPC FAIL'E");
		return;
	}

	io.buffer_data = malloc(ESDWORD[bufpointer+12]);
	strcpy(io.buffer_data, bufpointer + 16);
}

void main()
{   	
	InitDlls();
	LoadIniSettings();
	EventSetColorScheme(curcol_scheme);
	if (param[0] == '*') {
		LoadFileFromDocPack();
		param[0]='\0';
		sprintf(#title, "#DOCPACK - %s", #short_app_name);
	} else {
		if (streq(#param,"-new")) {Form.left+=40;Form.top+=40;}
		LoadFile(#param);
	}
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop()
	{
		switch(@WaitEventTimeout(400))
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
			case evReDraw:
				if (CheckActiveProcess(Form.ID)) EventMenuClick();
				draw_window();
				break;
			default:
				DrawStatusBar(" "); //clean DrawStatusBar text with delay
		}
	}
}

//===================================================//
//                                                   //
//                  EVENT HANDLERS                   //
//                                                   //
//===================================================//

void HandleButtonEvent()
{
	int btn = GetButtonID();
	if (btn==1) {
		SaveIniSettings();
		ExitProcess();
	}
	button.press(btn);
	switch(btn-10)
	{
		case BTN_FIND_NEXT:
			EventSearchNext();
			break;
		case BTN_FIND_CLOSE:
			search.hide();
			break;
		case BTN_CHANGE_CHARSET:
			EventShowCharsetsList();
			break;
	}
}

void HandleKeyEvent()
{
	GetKeys();

	switch (key_scancode)
	{
		case SCAN_CODE_F1:
			EventShowInfo();
			return;
		case SCAN_CODE_ESC:
			search.hide();
			return;
		case SCAN_CODE_ENTER:
			if (! search_box.flags & ed_focus) return;
		case SCAN_CODE_F3:
			EventSearchNext();
			return;
	}

	if (search.edit_key()) return;

	if (key_modifier & KEY_LCTRL) || (key_modifier & KEY_RCTRL) {
		if (key.press(ECTRL + key_scancode)) return;
		switch (key_scancode)
		{
			case SCAN_CODE_KEY_A:
				selection.select_all();
				DrawPage();
				return;
			case SCAN_CODE_KEY_X:
				EventCut();
				return;
			case SCAN_CODE_KEY_C:
				EventCopy();
				return;
			case SCAN_CODE_KEY_V:
				EventPaste();
				return;
			case SCAN_CODE_UP:
				EventMagnifyPlus();
				return;
			case SCAN_CODE_DOWN:
				EventMagnifyMinus();
				return;
			case SCAN_CODE_TAB:
				EventShowCharsetsList();
				return;
			case SCAN_CODE_KEY_F:
				search.show();
				return;
		}
	}

	if (key_modifier & KEY_LSHIFT) || (key_modifier & KEY_RSHIFT) {
		selection.set_start();
	} else {
		selection.cancel();
	}

	if (list.ProcessKey(key_scancode)) {
		if (key_modifier & KEY_LSHIFT) || (key_modifier & KEY_RSHIFT) selection.set_end();
		DrawPage();
		return;
	}
	if(enable_edit) EventInsertCharIntoText();
}

void HandleMouseEvent()
{
	mouse.get();
	list.wheel_size = 7;
	if (list.MouseScroll(mouse.vert)) {
		DrawPage();
		return; 
	}
	if (!scroll.delta2) && (list.MouseOver(mouse.x, mouse.y)) {
		if (mouse.key&MOUSE_LEFT) {

			GetKeyModifier();
			if (key_modifier & KEY_LSHIFT) || (key_modifier & KEY_RSHIFT) {
				if (mouse.down) selection.set_start();
				list.ProcessMouse(mouse.x, mouse.y);
				if (mouse.up) selection.set_end();
				DrawPage();
				return;
			}

			//as we have lines of variable width, we need to recalculate column_max
			list.column_max = lines.len(mouse.y - list.y / list.item_h + list.first);

			list.ProcessMouse(mouse.x, mouse.y);

			if (mouse.down) {
				selection.cancel();
				selection.set_start();
			} 
			selection.set_end();
			DrawPage();
		}
		if (mouse.key&MOUSE_RIGHT) && (mouse.up) {
			EventShowRmbMenu();
		}
		return;
	} 
	scrollbar_v_mouse (#scroll);
	if (list.first != scroll.position) {
		list.first = scroll.position;
		DrawPage(); 
	}
	search.edit_mouse();
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

bool EventSearchNext()
{
	int new_y = search.find_next(list.first+1);
	if (new_y) {
		list.first = new_y;
		list.CheckDoesValuesOkey();
		search_next = true;
		DrawPage();	
	}
}

void EventNewFile()
{
	RunProgram(#program_path, "-new");
}

void EventOpenDialog()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		LoadFile(#openfile_path);
		ParseAndPaint();
	}
}

void EventSave()
{
	int res;
	char backy_param[4096];
	if (io.buffer_data) {
		io.dir.make("/tmp0/1/quark_backups");
		sprintf(#backy_param, "%s -o /tmp0/1/quark_backups", #param);
		io.run("/sys/develop/backy", #backy_param);
		if (! io.write(#param, io.buffer_data) ) {
			notify(FILE_SAVED_WELL);
		} else {
			notify(FILE_NOT_SAVED);
		}
	}
}

void EventShowFileInfo()
{
	char ss_param[4096];
	if (!param) return;
	strcpy(#ss_param, "-p ");
	strcpy(#ss_param+3, #param);
	RunProgram("/sys/File managers/Eolite", #ss_param);
}

void EventMagnifyMinus()
{
	SetSizes('S');
	ParseAndPaint();
}

void EventMagnifyPlus()
{
	SetSizes('M');
	ParseAndPaint();
}

void EventShowCharsetsList()
{
	menu_id = CHANGE_CHARSET;
	open_lmenu(Form.left + Form.cwidth, Form.top + skin_height
		+ Form.cheight - 6, MENU_ALIGN_BOT_RIGHT, user_encoding+1, 
		"UTF-8\nKOI8-RU\nCP1251\nCP1252\nISO8859-5\nCP866\nAUTO");
}

void EventShowReopenMenu()
{
	menu_id = REOPEN_IN_APP;
	open_lmenu(Form.left+5 + reopenin_mx + 23, Form.top+29+skin_height, 
		MENU_ALIGN_TOP_RIGHT, NULL,
		"Tinypad\nTextEdit\nWebView\nFB2Read\nHexView\nOther");
}

void EventShowThemesList()
{
	menu_id = COLOR_SCHEME;
	open_lmenu(Form.left+5 + theme_mx + 23, 
		Form.top+29+skin_height, MENU_ALIGN_TOP_RIGHT, 
		curcol_scheme+1, #color_scheme_names);
}

void EventShowRmbMenu()
{
	menu_id = RMB_MENU;
	open_lmenu(Form.left + mouse.x+4, Form.top + skin_height + mouse.y,
		MENU_ALIGN_TOP_LEFT, NULL, #rmb_menu);
}


void EventSetColorScheme(dword _setn)
{
	curcol_scheme = _setn;
	theme.bg      = color_schemes[curcol_scheme*6];
	theme.text    = color_schemes[curcol_scheme*6+1];
	scroll.bckg_col = theme.bg;
	scroll.frnt_col = scroll.line_col = color_schemes[curcol_scheme*6+2];
	selection.color = color_schemes[curcol_scheme*6+3];
	theme.cursor    = color_schemes[curcol_scheme*6+4];
	theme.found     = color_schemes[curcol_scheme*6+5];
	if (list.count) ParseAndPaint();
}


void EventShowInfo() {
	static dword shared_about;
	if (!shared_about) {
		shared_about = memopen("QUARK_ABOUT", sizeof(about)+1, SHM_OPEN_ALWAYS + SHM_READ);
		strcpy(shared_about, #about);
	}
	RunProgram("/sys/dialog", "-info 122 *QUARK_ABOUT");
}

void EventChangeCharset(dword id)
{
	if (param[0]=='\0') return;
	user_encoding = id;
	LoadFile(#param);
	ParseAndPaint();
	draw_window();
}

void EventOpenFileInOtherApp(dword _id)
{
	dword app;
	byte open_param[4096];
	switch(_id) {
		case 0: app = "/sys/tinypad"; break;
		case 1: app = "/sys/develop/t_edit"; break;
		case 2: app = "/sys/network/webview"; break;
		case 3: app = "/sys/fb2read"; break;
		case 4: app = "/sys/develop/heed"; break;
		case 5: open_param[0]='~';
			strcpy(#open_param+1,#param);
			RunProgram("/sys/@open", #open_param);
			return;
	}
	RunProgram(app, #param);
}

void EventMenuClick()
{
	dword click_id = get_menu_click();

	if (click_id) && (menu_id) switch(menu_id)
	{
		case CHANGE_CHARSET: EventChangeCharset(click_id-1); break;
		case REOPEN_IN_APP:  EventOpenFileInOtherApp(click_id-1); break;
		case COLOR_SCHEME:   EventSetColorScheme(click_id-1); break;
		case RMB_MENU:       EventRbmMenuClick(click_id-1); break;
		default: notify("'Error: wrong menu number'E");
	}
	menu_id = NULL;
}

void EventClickSearch()
{
	if (search.visible) {
		search.hide();
	} else {
		search.show();
	}
}

void EventInsertCharIntoText()
{
	dword cursor_pos = lines.get(list.cur_y) + list.cur_x;

	switch(key_scancode)
	{
		case SCAN_CODE_DOWN:
		case SCAN_CODE_UP:
		case SCAN_CODE_HOME:
		case SCAN_CODE_END:
		case SCAN_CODE_PGUP:
		case SCAN_CODE_PGDN:
		return;
		case SCAN_CODE_BS:
		case SCAN_CODE_DEL:
		notify("'Not supported yet'A");
		return;
	}

	if (list.cur_x >= list.column_max) return;

	ESBYTE[cursor_pos] = key_ascii;
	list.KeyRight();
	PaintVisible();
}

void EventOpenSysfuncs()
{
	if (io.run("/sys/docpack", "f") <= 0) {
		notify("'Can not open SysFunctions because\n/rd/1/docpack is not found!'E");
	}
}

void EventOpenPipet()
{
	io.run("/sys/develop/pipet", NULL);
}

void EventRbmMenuClick(dword id)
{
	switch(id) {
		case 0: EventCut(); break;
		case 1: EventCopy(); break;
		case 2: EventPaste(); break;
		case 3: EventRevealInFolder(); break;
		case 4: EventCopyFilePath(); break;
	}
}

void EventCut()
{
	//selection.copy();
}

void EventCopy()
{
	char copy_status_text[32];

	dword copy_buf;
	dword copy_len;
	dword copy_start;
	dword copy_end;

	if (selection.is_active()) {
		copy_start = selection.start_offset;
		copy_end = selection.end_offset;
		if (copy_start > copy_end) copy_start >< copy_end;
	} else {
		copy_start = lines.get(list.cur_y);
		copy_end = lines.get(list.cur_y+1);
	}
	copy_len = copy_end - copy_start;
	copy_buf = malloc(copy_len + 2);
	strncpy(copy_buf, copy_start, copy_len);
	ESBYTE[copy_buf+copy_len] = '\0';
	Clipboard__CopyText(copy_buf);
	free(copy_buf);

	sprintf(#copy_status_text, #copied_chars, copy_len);
	DrawStatusBar(#copy_status_text);
}

void EventPaste()
{
	//selection.copy();
}

void EventRevealInFolder()
{
	RunProgram("/sys/File managers/Eolite", #param);
}

void EventCopyFilePath()
{
	char copy_status_text[32];
	Clipboard__CopyText(#param);
	sprintf(#copy_status_text, #copied_chars, strlen(#param));
	DrawStatusBar(#copy_status_text);
}

void EventEnableEdit()
{
	enable_edit ^= 1;
	if (enable_edit) notify("'Edit mode is enabled.\nNow you can only replace text, not insert, nor delete.'I");
	draw_window();
}

//===================================================//
//                                                   //
//               DRAWS AND OTHER FUNCS               //
//                                                   //
//===================================================//

void EncodeToDos()
{
	real_encoding = user_encoding;

	// Autodetecting charset
	if (real_encoding == CH_AUTO) {
		real_encoding = CH_CP866;
		if (strstr(io.buffer_data, "\208\190")) real_encoding = CH_UTF8;
		else {
			if (chrnum(io.buffer_data, '\246')>5) 
			|| (strstr(io.buffer_data, "\239\240")) real_encoding = CH_CP1251;
		}
	}
	if (real_encoding != CH_CP866) {
		ChangeCharset(real_encoding, "CP866", io.buffer_data);		
	}
}

void LoadFile(dword f_path) 
{
	if (io.buffer_data) free(io.buffer_data);
	if (ESBYTE[f_path]) {
		strcpy(#param, f_path);
		if (!io.read(#param)) goto NO_DATA;
		sprintf(#title, "%s - %s", #param, #short_app_name);
		EncodeToDos();	
	}
	else {
		NO_DATA:
		io.buffer_data = malloc(sizeof(intro));
		strcpy(io.buffer_data, #intro);
		strcpy(#title, #short_app_name); 
	}
	list.ClearList();
}

int AddTopBarButton(dword _event, _hotkey, char image_id, int x, pressed) {
	if (_hotkey) key.add_n(_hotkey, _event);
	return DrawTopPanelButton(button.add(_event), x, 5, image_id, pressed);
}


void DrawToolbar()
{
	#define SMALL_GAP 26+5
	#define BIG_GAP 26+18
	incn x;
	bool thema = false;
	bool reopa = false;

	bool serha = search.draw(BTN_FIND_NEXT+10, BTN_FIND_CLOSE+10, Form.cheight - SEARCH_H - STATUSBAR_H);
	if (menu_id == COLOR_SCHEME) thema = true;
	if (menu_id == REOPEN_IN_APP) reopa = true;

	DrawBar(0, 0, Form.cwidth, TOOLBAR_H - 1, sc.work);
	DrawBar(0, TOOLBAR_H - 1, Form.cwidth, 1, sc.work_graph);
	
	x.set(-SMALL_GAP+8);
	if(enable_edit) AddTopBarButton(#EventNewFile,        ECTRL+SCAN_CODE_KEY_N, 2,  x.inc(SMALL_GAP), false);
	                AddTopBarButton(#EventOpenDialog,     ECTRL+SCAN_CODE_KEY_O, 0,  x.inc(SMALL_GAP), false);
	if(enable_edit) && (param[0]) AddTopBarButton(#EventSave,           ECTRL+SCAN_CODE_KEY_S, 5,  x.inc(SMALL_GAP), false);
	                AddTopBarButton(#EventShowFileInfo,   ECTRL+SCAN_CODE_KEY_I, 10, x.inc(SMALL_GAP), false);
	                AddTopBarButton(#EventMagnifyMinus,   ECTRL+SCAN_CODE_MINUS, 33, x.inc(BIG_GAP),   false);
	                AddTopBarButton(#EventMagnifyPlus,    ECTRL+SCAN_CODE_PLUS,  32, x.inc(SMALL_GAP), false);
	                AddTopBarButton(#EventClickSearch,    ECTRL+SCAN_CODE_KEY_F, 49, x.inc(BIG_GAP),   serha);  search_mx = EAX;
	x.set(Form.cwidth-4);
	                AddTopBarButton(#EventEnableEdit,       NULL,                  38, x.inc(-SMALL_GAP), enable_edit);
	//if(enable_edit) AddTopBarButton(#EventShowInfo,       NULL,                  -1, x.inc(-SMALL_GAP), false); burger_mx = EAX;
	                AddTopBarButton(#EventShowThemesList, NULL,                  40, x.inc(-BIG_GAP), thema); theme_mx = EAX;
	                AddTopBarButton(#EventShowReopenMenu, ECTRL+SCAN_CODE_KEY_E, 16, x.inc(-SMALL_GAP),   reopa); reopenin_mx = EAX;
	if(enable_edit) AddTopBarButton(#EventOpenSysfuncs,   NULL,                  18, x.inc(-SMALL_GAP), false);
	if(enable_edit) AddTopBarButton(#EventOpenPipet,      NULL,                  39, x.inc(-SMALL_GAP), false);
}

void DrawStatusBar(dword _in_text)
{
	static char status_text[64];
	if (Form.status_window>2) return;
	if (_in_text) strncpy(#status_text, _in_text, sizeof(status_text));
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, sc.work_graph);
	DrawBar(0,Form.cheight - STATUSBAR_H+1, Form.cwidth,STATUSBAR_H-1, sc.work);
	WriteText(5, Form.cheight - STATUSBAR_H + 4, 0x80, sc.work_text, #status_text);
	if (param[0]) {
		WriteTextCenter(Form.cwidth-70, Form.cheight - STATUSBAR_H + 4,
			60, sc.work_text, real_encoding*10+#charsets);
		DefineHiddenButton(Form.cwidth-70, Form.cheight - STATUSBAR_H + 1,
			60, 12, BTN_CHANGE_CHARSET+10);
	}
}

void draw_window()
{
	int old_w = list.w;
	DefineAndDrawWindow(Form.left,Form.top,Form.width,Form.height,0x73,0,#title,0);
	GetProcessInfo(#Form, SelfInfo);
	sc.get();
	if (Form.status_window>2) return;
	if (Form.width  < 450) { MoveSize(OLD,OLD,450,OLD); return; }
	if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); return; }
	
	button.init(40);
	key.init(40);

	SetSizes(font_size);

	if ((list.w == old_w) && (list.count)) {
		DrawPage(); 
	} else {
		ParseAndPaint();
	}

	DrawToolbar();
	DrawStatusBar(NULL);
}

void DrawPage()
{
	scroll.max_area = list.count;
	scroll.cur_area = list.visible;
	scroll.position = list.first;
	scroll.all_redraw = 0;
	scroll.start_x = list.x + list.w;
	scroll.start_y = list.y;
	scroll.size_y = list.h;
	scrollbar_v_draw(#scroll);

	DrawRectangle(scroll.start_x, scroll.start_y, scroll.size_x, 
		scroll.size_y-1, scroll.bckg_col);
	PaintVisible();
}


void SetSizes(char _size)
{
	font_size = _size;
	if (font_size == 'S') list.SetFont(6, 9, 00001000b);
	if (font_size == 'M') list.SetFont(8, 14, 00011000b);
	list.item_w = list.font_w;
	list.horisontal_selelection = true;
	list.SetSizes(0, TOOLBAR_H, Form.cwidth-scroll.size_x-1, 
		Form.cheight - TOOLBAR_H - calc(search.visible * SEARCH_H) - STATUSBAR_H /*- TAB_H*/, 
		math.round(list.font_h * 1.4));
}