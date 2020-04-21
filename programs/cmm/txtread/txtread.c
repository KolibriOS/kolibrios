#define MEMSIZE 1024*80

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/io.h"
#include "../lib/gui.h"
#include "../lib/list_box.h"
#include "../lib/kfont.h"

#include "../lib/obj/box_lib.h"
#include "../lib/obj/libini.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/iconv.h"
#include "../lib/obj/proc_lib.h"

#include "../lib/patterns/simple_open_dialog.h"

//===================================================//
//                                                   //
//                 INTERNAL INCLUDES                 //
//                                                   //
//===================================================//

proc_info Form;
llist list;

#define TOOLBAR_H 34
#define TOOLBAR_ICON_WIDTH  24
#define TOOLBAR_ICON_HEIGHT 22

dword bg_color;
dword text_color;

#include "search.h"
#include "prepare_page.h"
#include "data.h"

int encoding;
int curcol_scheme;

#include "ini.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

scroll_bar scroll = { 15,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

char title[4196];

bool help_opened = false;

int charsets_mx,
    reopenin_mx,
    colscheme_mx,
    search_mx;

enum {
	OPEN_FILE,
	MAGNIFY_MINUS,
	MAGNIFY_PLUS,
	CHANGE_CHARSET,
	REOPEN_IN_APP,
	SHOW_INFO,
	SHOW_FILE_PROPERTIES,
	COLOR_SCHEME,
	SEARCH_BTN,
	BTN_FIND_NEXT,
	BTN_FIND_CLOSE
};

dword menu_id;

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
}

void main()
{   	
	InitDlls();	
	OpenDialog_init stdcall (#o_dialog);
	if (param) strcpy(#openfile_path, #param);
	LoadIniSettings();
	EventSetColorScheme(curcol_scheme);
	kfont.init(DEFAULT_FONT);
	Libimg_LoadImage(#skin, abspath("toolbar.png"));
	LoadFile(#param);
	list.no_selection = true;
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
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
			case evReDraw:
				if (CheckActiveProcess(Form.ID)) EventMenuClick();
				draw_window();
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
	switch(btn-10)
	{
		case OPEN_FILE:
			EventOpenDialog();
			break;
		case SHOW_FILE_PROPERTIES:
			EventShowFileProperties();
			break;
		case MAGNIFY_PLUS:
			EventMagnifyPlus();
			break;
		case MAGNIFY_MINUS:
			EventMagnifyMinus();
			break;
		case CHANGE_CHARSET:
			EventShowCharsetsList();
			break;
		case REOPEN_IN_APP:
			EventShowReopenMenu();
			break;
		case COLOR_SCHEME:
			EventShowColorSchemesList();
			break;
		case SHOW_INFO:
			EventShowInfo();
			break;
		case SEARCH_BTN:
			if (search.visible) {
				search.hide();
			} else {
				search.show();
			}
			break;
		case BTN_FIND_NEXT:
			EventSearchNext();
			break;
		case BTN_FIND_CLOSE:
			search.hide();
			break;
	}
}

void HandleKeyEvent()
{
	int new_y;
	if (help_opened) {
		help_opened = false;
		DrawPage();
		return; 
	}
	GetKeys();
	if (key_modifier & KEY_LCTRL) || (key_modifier & KEY_RCTRL) {
		switch (key_scancode)
		{
			case SCAN_CODE_KEY_O:
				EventOpenDialog();
				return;
			case SCAN_CODE_KEY_I:
				EventShowFileProperties();
				return;
			case SCAN_CODE_PLUS:
			case SCAN_CODE_UP:
				EventMagnifyPlus();
				return;
			case SCAN_CODE_DOWN:
			case SCAN_CODE_MINUS:
				EventMagnifyMinus();
				return;
			case SCAN_CODE_KEY_E:
				EventShowReopenMenu();
				return;
			case SCAN_CODE_TAB:
				EventShowCharsetsList();
				return;
			case SCAN_CODE_KEY_F:
				search.show();
				return;
			case SCAN_CODE_HOME:
				list.KeyHome();
				DrawPage();
		}
	}
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
	if (!search.edit_key()) {
		if (list.ProcessKey(key_scancode)) DrawPage();
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
	int new_y = search.find_next(list.first, bg_color);
	if (new_y) {
		list.first = new_y / list.item_h;
		list.CheckDoesValuesOkey();
		DrawPage();		
	}
}

void EventOpenDialog()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		LoadFile(#openfile_path);
		search.clear();
		ParseAndPaint();
	}
}

void EventShowFileProperties()
{
	char ss_param[4096];
	if (!param) return;
	strcpy(#ss_param, "-p ");
	strcpy(#ss_param+3, #param);
	RunProgram("/sys/File managers/Eolite", #ss_param);
}

void EventMagnifyPlus()
{
	kfont.size.pt++;
	if(!kfont.changeSIZE())
		kfont.size.pt--;
	else
		ParseAndPaint();
}

void EventMagnifyMinus()
{
	kfont.size.pt--;
	if(!kfont.changeSIZE())
		kfont.size.pt++;
	else
		ParseAndPaint();
}

void EventShowCharsetsList()
{
	menu_id = CHANGE_CHARSET;
	open_lmenu(Form.left+5 + charsets_mx, Form.top+29+skin_height, MENU_ALIGN_TOP_LEFT, 
		encoding+1, "UTF-8\nKOI8-RU\nCP1251\nCP1252\nISO8859-5\nCP866");
}

void EventShowReopenMenu()
{
	menu_id = REOPEN_IN_APP;
	open_lmenu(Form.left+5 + reopenin_mx, Form.top+29+skin_height, MENU_ALIGN_TOP_LEFT, 0,
		"Tinypad\nTextEdit\nWebView\nFB2Read\nHexView\nOther");
}

void EventShowColorSchemesList()
{
	menu_id = COLOR_SCHEME;
	open_lmenu(Form.left+2 + colscheme_mx + 26, 
		Form.top+29+skin_height, MENU_ALIGN_TOP_RIGHT, 
		curcol_scheme+1, #color_scheme_names);
}

void EventSetColorScheme(dword _setn)
{
	curcol_scheme = _setn;
	bg_color   = color_schemes[curcol_scheme*2];
	text_color = color_schemes[curcol_scheme*2+1];
	if (list.count) ParseAndPaint();
}

void EventShowInfo() {
	help_opened = true;
	DrawBar(list.x, list.y, list.w, list.h, bg_color);
	WriteText(list.x + 10, list.y + 10, 10000001b, text_color, VERSION);
	WriteTextLines(list.x + 10, list.y+40, 10110000b, text_color, ABOUT, 20);
}

void EventChangeCharset(dword id)
{
	encoding = id;
	LoadFile(#openfile_path);
	ParseAndPaint();
	draw_window();
}

void EventOpenFileInAnotherProgram(dword _id)
{
	dword app;
	byte open_param[4096];
	switch(_id) {
		case 0:
			app = "/sys/tinypad";
			break;
		case 1:
			app = "/sys/develop/t_edit";
			break;
		case 2:
			app = "/sys/network/webview";
			break;
		case 3:
			app = "/sys/fb2read";
			break;
		case 4:
			app = "/sys/develop/heed";
			break;
		case 5:
			open_param[0]='~';
			strcpy(#open_param+1,#param);
			RunProgram("/sys/@open", #open_param);
			break;
	}
	RunProgram(app, #param);
}

void EventMenuClick()
{
	dword click_id = get_menu_click();

	if (click_id) && (menu_id)
	{
		if (menu_id == CHANGE_CHARSET) EventChangeCharset(click_id-1);
		else if (menu_id == REOPEN_IN_APP) EventOpenFileInAnotherProgram(click_id-1);
		else if (menu_id == COLOR_SCHEME) EventSetColorScheme(click_id-1);
		else notify("'Error: wrong menu number'E");
	}
	menu_id = NULL;
}

//===================================================//
//                                                   //
//               DRAWS AND OTHER FUNCS               //
//                                                   //
//===================================================//

void LoadFile(dword f_path) 
{
	int tmp;
	if (ESBYTE[f_path]) {
		strcpy(#param, f_path);
		if (!io.read(#param)) goto NO_DATA;
		strcpy(#title, #param);
		strcat(#title, " - Text Reader"); 
	}
	else {
		NO_DATA:
		if (list.count) return;
		io.buffer_data = INTRO_TEXT;
		strcpy(#title, "Text Reader"); 
	}
	if (encoding!=CH_CP866) ChangeCharset(encoding, "CP866", io.buffer_data);
	list.ClearList();
}

int DrawToolbarButton(char image_id, int x)
{
	DrawOvalBorder(x, 5, TOOLBAR_ICON_WIDTH, TOOLBAR_ICON_HEIGHT, sc.work_graph, 
		sc.work_graph,sc.work_graph, sc.work_dark);
	img_draw stdcall(skin.image, x+1, 5+1, TOOLBAR_ICON_WIDTH, TOOLBAR_ICON_HEIGHT, TOOLBAR_ICON_WIDTH*image_id, 0);

	if (menu_id) && (menu_id == image_id) {
		DrawRectangle3D(x+1, 6, TOOLBAR_ICON_WIDTH-1, TOOLBAR_ICON_HEIGHT-1, 0xCCCccc, 0xF8FCF8);
		PutShadow(x+1, 6, TOOLBAR_ICON_WIDTH, TOOLBAR_ICON_HEIGHT, true, 2);
	} 
	DefineHiddenButton(x+1, 6, TOOLBAR_ICON_WIDTH, TOOLBAR_ICON_HEIGHT, 10+image_id);
	return x;
}

void draw_window()
{
	#define BUTTONS_GAP 5
	#define BLOCKS_GAP 18
	#define TOOLBAR_BUTTON_WIDTH 26
	incn x;
	int old_w;
	DefineAndDrawWindow(Form.left,Form.top,Form.width,Form.height,0x73,0,#title,0);
	GetProcessInfo(#Form, SelfInfo);
	sc.get();
	if (Form.status_window>2) return;

	if (Form.width  < 340) { MoveSize(OLD,OLD,340,OLD); return; }
	if (Form.height < 200) { MoveSize(OLD,OLD,OLD,200); return; }
	
	DrawBar(0, 0, Form.cwidth, TOOLBAR_H - 1, sc.work);
	DrawBar(0, TOOLBAR_H - 1, Form.cwidth, 1, sc.work_graph);
	
	x.n = 0;
	DrawToolbarButton(OPEN_FILE,       x.inc(8));
	DrawToolbarButton(SHOW_FILE_PROPERTIES, x.inc(TOOLBAR_BUTTON_WIDTH + BUTTONS_GAP));

	DrawToolbarButton(MAGNIFY_MINUS,   x.inc(TOOLBAR_BUTTON_WIDTH + BLOCKS_GAP));
	DrawToolbarButton(MAGNIFY_PLUS,    x.inc(TOOLBAR_BUTTON_WIDTH - 1));

	search_mx    = DrawToolbarButton(SEARCH_BTN,     x.inc(TOOLBAR_BUTTON_WIDTH + BLOCKS_GAP));
	charsets_mx  = DrawToolbarButton(CHANGE_CHARSET, x.inc(TOOLBAR_BUTTON_WIDTH + BUTTONS_GAP));
	reopenin_mx  = DrawToolbarButton(REOPEN_IN_APP,  x.inc(TOOLBAR_BUTTON_WIDTH + BUTTONS_GAP));

	x.n = Form.cwidth - 34;
	DrawToolbarButton(SHOW_INFO, x.n);
	colscheme_mx = DrawToolbarButton(COLOR_SCHEME,   x.inc(-TOOLBAR_BUTTON_WIDTH - BUTTONS_GAP));

	if (search.draw(BTN_FIND_NEXT+10, BTN_FIND_CLOSE+10)) {
		DrawRectangle3D(search_mx+1, 6, TOOLBAR_ICON_WIDTH-1, 
			TOOLBAR_ICON_HEIGHT-1, 0xCCCccc, 0xF8FCF8);
	}

	old_w = list.w;

	list.SetSizes(0, TOOLBAR_H, Form.cwidth-scroll.size_x-1, 
		Form.cheight-TOOLBAR_H-search.height(), math.round(kfont.size.pt * 1.4));

	if ((Form.cwidth-scroll.size_x-1 == old_w) && (list.count)) {
		DrawPage(); 
	} else {
		ParseAndPaint();
	}		
	
	DrawRectangle(scroll.start_x, scroll.start_y, scroll.size_x, scroll.size_y-1, scroll.bckg_col);
}

void DrawPage()
{
	kfont.ShowBufferPart(list.x, list.y, list.w, list.h, list.first*list.item_h*list.w);

	scroll.max_area = list.count;
	scroll.cur_area = list.visible;
	scroll.position = list.first;
	scroll.all_redraw = 0;
	scroll.start_x = list.x + list.w;
	scroll.start_y = list.y;
	scroll.size_y = list.h;
	scrollbar_v_draw(#scroll);
}
