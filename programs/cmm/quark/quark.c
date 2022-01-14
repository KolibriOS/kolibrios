/*
	Quark Code Edit
	Author: Kiril Lipatov aka Leency
	Licence: GPLv2

	The core components of this app are:
		1. textbuf: page data
		2. list: text grid with keyboard and mouse events
		3. lines: the mas of pointers for each line start
		4. selection
*/

#define MEMSIZE 50*1024

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
#include "../lib/clipboard.h"
#include "../lib/math.h"

#include "../lib/obj/box_lib.h"
#include "../lib/obj/libini.h"
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
scroll_bar scroll = { 15,200,398,44,0,2,115,15,0,0xeeeeee,
	0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

#define TOOLBAR_H 38
#define TOOLBAR_ICON_WIDTH  24
#define TOOLBAR_ICON_HEIGHT 22
#define STATUSBAR_H 15
#define TAB_H 20

int user_encoding;
int real_encoding = CH_CP866;
int curcol_scheme;
int font_size;

bool search_next = false;

#include "data.h"
#include "textbuf.h"
#include "selection.h"
#include "search.h"
#include "prepare_page.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

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
	BTN_FIND_PREVIOUS,
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
	load_dll(libini,    #lib_init,       1);
	load_dll(iconv_lib, #iconv_open,     0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
}

void LoadFileFromDocPack()
{
	dword bufsize = atoi(#file_path + 1) + 20;
	dword bufpointer = malloc(bufsize);

	ESDWORD[bufpointer+0] = 0;
	ESDWORD[bufpointer+4] = 8;
	IpcSetArea(bufpointer, bufsize);

	SetEventMask(EVM_IPC);
	if (@WaitEventTimeout(200) != evIPC) {
		notify("'IPC FAIL'E");
	} else {
		textbuf.set(bufpointer + 16, ESDWORD[bufpointer+12]);
	}
	free(bufpointer);
	file_path[0]='\0';
	sprintf(#title, "#DOCPACK - %s", #short_app_name);
}

void main()
{
	InitDlls();
	LoadIniSettings();
	EventSetColorScheme(curcol_scheme);
	if (file_path[0] == '*') {
		LoadFileFromDocPack();
	} else {
		if (streq(#file_path,"-new")) {Form.left+=40;Form.top+=40;}
		LoadFile(#file_path);
	}
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(@WaitEventTimeout(400))
	{
		case evMouse:  HandleMouseEvent(); break;
		case evKey:    HandleKeyEvent(); break;
		case evButton: HandleButtonEvent(); break;
		case evReDraw: draw_window(); break;
		default:       DrawStatusBar(" "); //clean DrawStatusBar text with delay
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
		case BTN_FIND_NEXT:      EventSearchNext();       break;
		case BTN_FIND_PREVIOUS:  EventSearchPrevious();   break;
		case BTN_FIND_CLOSE:     search.hide();           break;
		case BTN_CHANGE_CHARSET: EventShowCharsetsList(); break;
	}
}

void HandleKeyEvent()
{
	GetKeys();

	switch (key_scancode)
	{
		case SCAN_CODE_ESC:
			search.hide();
			return;
		case SCAN_CODE_ENTER:
			if (! search_box.flags & ed_focus) break;
		case SCAN_CODE_F3:
			if (key_modifier & KEY_LSHIFT) {
				EventSearchPrevious();
			} else {
				EventSearchNext();
			}
			return;
	}

	if (search.edit_key()) return;

	if (key_modifier & KEY_LCTRL) || (key_modifier & KEY_RCTRL) {
		if (key.press(ECTRL + key_scancode)) return;
		switch (key_scancode)
		{
			case SCAN_CODE_KEY_A: EventSelectAllText();    return;
			case SCAN_CODE_KEY_C: EventCopy();             return;
			//case SCAN_CODE_KEY_X: EventCut();              return;
			//case SCAN_CODE_KEY_V: EventPaste();            return;
			case SCAN_CODE_UP:    EventMagnifyPlus();      return;
			case SCAN_CODE_DOWN:  EventMagnifyMinus();     return;
			case SCAN_CODE_TAB:   EventShowCharsetsList(); return;
			case SCAN_CODE_KEY_F: search.show();           return;
		}
	}

	if (key_modifier & KEY_LSHIFT) || (key_modifier & KEY_RSHIFT) {
		selection.set_start();
	} else {
		//EventInsertCharIntoText();
		selection.cancel();
	}

	if (key_scancode == SCAN_CODE_LEFT) && (!list.cur_x) && (list.cur_y) list.column_max = lines.len(list.cur_y-1);
	if (list.ProcessKey(key_scancode)) {
		if (key_modifier & KEY_LSHIFT) || (key_modifier & KEY_RSHIFT) selection.set_end();
		DrawPage();
		return;
	}
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
	if (search.find_next(list.first+1)) {
		list.first = EAX;
		list.CheckDoesValuesOkey();
		search_next = true;
		DrawPage();
	}
}

bool EventSearchPrevious()
{
	if (search.find_prior(list.first)) {
		list.first = EAX;
		list.CheckDoesValuesOkey();
		search_next = true;
		DrawPage();
	}
}

void EventOpenDialog()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		LoadFile(#openfile_path);
		ParseAndPaint();
	}
}

void EventShowFileInfo()
{
	char ss_param[4096];
	if (!file_path) return;
	strcpy(#ss_param, "-p ");
	strcpy(#ss_param+3, #file_path);
	RunProgram("/sys/File managers/Eolite", #ss_param);
}

void EventMagnifyMinus()
{
	font_size = math.max(0, font_size-1);
	SetFontSize(font_size);
	ParseAndPaint();
}

void EventMagnifyPlus()
{
	font_size = math.min(5, font_size+1);
	SetFontSize(font_size);
	ParseAndPaint();
}

void EventShowCharsetsList()
{
	menu_id = CHANGE_CHARSET;
	open_lmenu(Form.cwidth-4, Form.cheight - 6, MENU_BOT_RIGHT,
		user_encoding+1,
		"UTF-8\nKOI8-RU\nCP1251\nCP1252\nISO8859-5\nCP866\nAUTO");
}

void EventShowReopenMenu()
{
	menu_id = REOPEN_IN_APP;
	open_lmenu(reopenin_mx, 29, MENU_TOP_LEFT, NULL,
		"Tinypad\nCodeEdit\nWebView\nFB2Read\nHexView\nOther");
}

void EventShowThemesList()
{
	menu_id = COLOR_SCHEME;
	open_lmenu(theme_mx, 29, MENU_TOP_LEFT,
		curcol_scheme+1, #color_scheme_names);
}

void EventShowRmbMenu()
{
	menu_id = RMB_MENU;
	open_lmenu(mouse.x, mouse.y, MENU_TOP_LEFT, NULL, #rmb_menu);
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

void EventChangeCharset(dword id)
{
	if (file_path[0]=='\0') return;
	user_encoding = id;
	LoadFile(#file_path);
	ParseAndPaint();
	draw_window();
}

void EventOpenFileInOtherApp(dword _id)
{
	dword app;
	byte open_param[4096];
	switch(_id) {
		case 0: app = "/sys/tinypad"; break;
		case 1: app = "/sys/develop/cedit"; break;
		case 2: app = "/sys/network/webview"; break;
		case 3: app = "/sys/fb2read"; break;
		case 4: app = "/sys/develop/heed"; break;
		case 5: open_param[0]='~';
			strcpy(#open_param+1,#file_path);
			RunProgram("/sys/@open", #open_param);
			return;
	}
	RunProgram(app, #file_path);
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

/*
void EventInsertCharIntoText()
{
	dword i;
	dword cursor_pos = lines.get(list.cur_y) + list.cur_x;

	switch(key_scancode)
	{
		case SCAN_CODE_DOWN:
		case SCAN_CODE_UP:
		case SCAN_CODE_LEFT:
		case SCAN_CODE_RIGHT:
		case SCAN_CODE_HOME:
		case SCAN_CODE_END:
		case SCAN_CODE_PGUP:
		case SCAN_CODE_PGDN:
			return;
		case SCAN_CODE_BS:
			if (selection.is_active()) {
				EventDeleteSelectedText();
			} else {
				if (!list.cur_x) && (!list.cur_y) break;
				textbuf.del(cursor_pos-1, cursor_pos);
				if (!list.cur_x) && (list.cur_y) {
					list.column_max = lines.len(list.cur_y-1);
					list.KeyLeft();
				}
				list.KeyLeft();
			}
			ParseAndPaint();
			return;
		case SCAN_CODE_DEL:
			if (selection.is_active()) {
				EventDeleteSelectedText();
			} else {
				if (cursor_pos < textbuf.p + textbuf.len) textbuf.del(cursor_pos, cursor_pos+1);
			}
			ParseAndPaint();
			return;
		default:
			if (selection.is_active()) {
				EventDeleteSelectedText();
				Parse();
			}
			cursor_pos = lines.get(list.cur_y) + list.cur_x;
			textbuf.insert_ch(cursor_pos, key_ascii);
			list.KeyRight();
			Parse();
			list.column_max = lines.len(list.cur_y);
			if (key_scancode == SCAN_CODE_ENTER) list.KeyRight();
			DrawPage();
	}
}
*/

void EventRbmMenuClick(dword id)
{
	switch(id) {
		case 0: EventCopy(); break;
		case 1: EventRevealInFolder(); break;
		case 2: EventCopyFilePath(); break;
	}
}

void EventSelectAllText()
{
	selection.select_all();
	DrawPage();
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

/*
void EventCut()
{
	if (!selection.is_active()) {
		selection.start_offset = lines.get(list.cur_y);
		selection.end_offset = lines.get(list.cur_y+1);
	}
	EventCopy();
	EventDeleteSelectedText();
	ParseAndPaint();
}

void EventPaste()
{
	int i;
	dword buf = Clipboard__GetSlotData(Clipboard__GetSlotCount()-1);
	if (selection.is_active()) {
		EventDeleteSelectedText();
	} 
	cursor_pos = lines.get(list.cur_y) + list.cur_x;
	textbuf.insert_str(cursor_pos, buf+12, ESDWORD[buf]-12);
	for (i=0; i<ESDWORD[buf]-12; i++) list.KeyRight();
	ParseAndPaint();
}

void EventDeleteSelectedText()
{
	textbuf.del(selection.start_offset, selection.end_offset);
	list.cur_x = math.min(selection.start_x, selection.end_x);
	list.cur_y = math.min(selection.start_y, selection.end_y);
	selection.cancel();
}
*/

void EventRevealInFolder()
{
	RunProgram("/sys/File managers/Eolite", #file_path);
}

void EventCopyFilePath()
{
	char copy_status_text[32];
	Clipboard__CopyText(#file_path);
	sprintf(#copy_status_text, #copied_chars, strlen(#file_path));
	DrawStatusBar(#copy_status_text);
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
		if (strstr(textbuf.p, "\208\190")) real_encoding = CH_UTF8;
		else {
			if (chrnum(textbuf.p, '\246')>5)
			|| (strstr(textbuf.p, "\239\240")) real_encoding = CH_CP1251;
		}
	}
	if (real_encoding != CH_CP866) {
		ChangeCharset(real_encoding, CH_CP866, textbuf.p);
	}
}

void LoadFile(dword f_path)
{
	if (ESBYTE[f_path]) {
		strcpy(#file_path, f_path);
		if (!io.read(#file_path)) goto NO_DATA;
		textbuf.set(io.buffer_data, io.FILES_SIZE);
		free(io.buffer_data);
		sprintf(#title, "%s - %s", #file_path, #short_app_name);
		EncodeToDos();
	}
	else {
		NO_DATA:
		textbuf.set(#intro, sizeof(intro)-1);
		strcpy(#title, #short_app_name);
	}
	list.ClearList();
}

int TopBarBt(dword _event, _hotkey, char image_id, int x, pressed) {
	if (_hotkey) key.add_n(_hotkey, _event);
	return DrawTopPanelButton(button.add(_event), x, 5, image_id, pressed);
}


void DrawToolbar()
{
	#define GAP_S 26+7
	#define GAP_B 26+19
	incn x;
	bool thema = false;
	bool reopa = false;

	if (menu_id == COLOR_SCHEME) thema = true;
	if (menu_id == REOPEN_IN_APP) reopa = true;

	DrawBar(0, 0, Form.cwidth, TOOLBAR_H - 1, sc.work);
	DrawBar(0, TOOLBAR_H - 1, Form.cwidth, 1, sc.line);

	x.set(-GAP_S+8);
	TopBarBt(#EventOpenDialog,     ECTRL+SCAN_CODE_KEY_O, 0,  x.inc(GAP_S), false);
	TopBarBt(#EventShowFileInfo,   ECTRL+SCAN_CODE_KEY_I, 10, x.inc(GAP_S), false);
	TopBarBt(#EventMagnifyMinus,   ECTRL+SCAN_CODE_MINUS, 33, x.inc(GAP_B),   false);
	TopBarBt(#EventMagnifyPlus,    ECTRL+SCAN_CODE_PLUS,  32, x.inc(GAP_S), false);
	TopBarBt(#EventClickSearch,    ECTRL+SCAN_CODE_KEY_F, 49, x.inc(GAP_B),   search.visible);  search_mx = EAX;
	TopBarBt(#EventShowThemesList, NULL,                  40, x.inc(GAP_B), thema); theme_mx = EAX;
	TopBarBt(#EventShowReopenMenu, ECTRL+SCAN_CODE_KEY_E, 16, x.inc(GAP_S), reopa); reopenin_mx = EAX;
}

void DrawStatusBar(dword _in_text)
{
	static char status_text[64];
	if (Form.status_window&ROLLED_UP) return;
	if (_in_text) strncpy(#status_text, _in_text, sizeof(status_text));
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, sc.line);
	DrawBar(0,Form.cheight - STATUSBAR_H+1, Form.cwidth,STATUSBAR_H-1, sc.work);
	WriteText(5, Form.cheight - STATUSBAR_H + 4, 0x80, sc.work_text, #status_text);
	if (file_path[0]) {
		WriteTextCenter(Form.cwidth-70, Form.cheight - STATUSBAR_H + 4,
			60, sc.work_text, real_encoding*10+#charsets);
		DefineHiddenButton(Form.cwidth-70, Form.cheight - STATUSBAR_H + 1,
			60, 12, BTN_CHANGE_CHARSET+10);
	}
}

void draw_window()
{
	int old_w = list.w;
	if (CheckActiveProcess(Form.ID)) EventMenuClick();
	DefineAndDrawWindow(Form.left,Form.top,Form.width,Form.height,0x73,0,#title,0);
	GetProcessInfo(#Form, SelfInfo);
	sc.get();
	if (Form.status_window&ROLLED_UP) return;
	if (Form.width  < 450) { MoveSize(OLD,OLD,450,OLD); return; }
	if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); return; }

	button.init(40);
	key.init(40);

	SetFontSize(font_size);

	if ((list.w == old_w) && (list.count)) {
		DrawPage();
	} else {
		ParseAndPaint();
	}

	DrawToolbar();
	DrawSearch();
	DrawStatusBar(NULL);
}

bool DrawSearch()
{
	char matches[30];
	int _y = Form.cheight - SEARCH_H - STATUSBAR_H;
	if (!search.visible) return false;
	DrawBar(0, _y, Form.cwidth, 1, sc.line);
	DrawBar(0, _y+1, Form.cwidth, SEARCH_H-1, sc.work);

	search_box.top = _y + 6;
	search_box.width = math.min(Form.width - 200, 150);

	DrawRectangle(search_box.left-1, search_box.top-1, search_box.width+2, 23,sc.line);

	edit_box_draw stdcall(#search_box);

	DrawCaptButton(search_box.left+search_box.width+14, search_box.top-1, 30,
		TOOLBAR_ICON_HEIGHT+1, BTN_FIND_PREVIOUS+10, sc.light, sc.work_text, "<");
	DrawCaptButton(search_box.left+search_box.width+44, search_box.top-1, 30,
		TOOLBAR_ICON_HEIGHT+1, BTN_FIND_NEXT+10, sc.light, sc.work_text, ">");

	sprintf(#matches, T_MATCHES, search.found.count);
	WriteTextWithBg(search_box.left+search_box.width+14+85,
		search_box.top+3, 0xD0, sc.work_text, #matches, sc.work);

	DefineHiddenButton(Form.cwidth-26, search_box.top-1, TOOLBAR_ICON_HEIGHT+1,
		TOOLBAR_ICON_HEIGHT+1, BTN_FIND_CLOSE+10);
	WriteText(Form.cwidth-26+7, search_box.top+2, 0x81, sc.line, "x");
	return true;
}

void SetFontSize(char _size)
{
	font_size = _size;
	if (font_size == 0) list.SetFont(  6,      9, 00001000b);
	if (font_size == 1) list.SetFont(  8,     14, 00011000b);
	if (font_size == 2) list.SetFont(2*6,    2*9, 00001001b);
	if (font_size == 3) list.SetFont(2*8, 2*14-2, 00011001b);
	if (font_size == 4) list.SetFont(3*6,    3*9, 00001010b);
	if (font_size == 5) list.SetFont(3*8, 3*14-2, 00011010b);
	list.item_w = list.font_w;
	list.horisontal_selelection = true;
	list.SetSizes(0, TOOLBAR_H, Form.cwidth-scroll.size_x-1,
		Form.cheight - TOOLBAR_H - calc(search.visible * SEARCH_H) - STATUSBAR_H /*- TAB_H*/,
		math.round(list.font_h * 1.3));
}