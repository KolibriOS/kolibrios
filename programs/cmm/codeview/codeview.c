
#define MEMSIZE 4096 * 200

//libraries
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
#include "..\lib\obj\iconv.h"
#include "..\lib\obj\proc_lib.h"

//useful patterns
#include "..\lib\patterns\simple_open_dialog.h"

char homepage[] = FROM "html\\homepage.htm""\0";
char page_not_found[] = FROM "html\\page_not_found_en.htm""\0";

char version[]="C-- Code View";
char accept_language[]= "Accept-Language: en\n";

#define URL_SERVICE_HOME "CodeView:home"

proc_info Form;


dword TOOLBAR_H = 40;
dword STATUSBAR_H = 0;


dword col_bg = 0xE3E2E2;
dword panel_color  = 0xE3E2E2;
dword border_color = 0x8C8C8C;

bool debug_mode = false;

bool open_in_a_new_window = false;

enum { 
	REFRESH_BUTTON, 
	EDIT_SOURCE,
};

#define URL_SIZE 4000;
#include "..\TWB\TWB.c"
#include "highlight_c.h"

char default_dir[] = "/rd/1";
od_filter filter2 = { 16, "C\0H\0C--\0H--\0CPP\0\0" };

char current_path[URL_SIZE+1];
char edit_path[URL_SIZE+1];
int	mouse_twb;
edit_box address_box = {250,60,30,0xffffff,0x94AECE,0xffffff,0xffffff,0x10000000,URL_SIZE-2,#edit_path,#mouse_twb,2,19,19};

#define SKIN_Y 24

void LoadLibraries()
{
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(iconv_lib, #iconv_open,0);
	load_dll(Proc_lib,  #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
}

void main()
{
	int i;
	LoadLibraries();
	if (param) strcpy(#current_path, #param); else strcpy(#current_path, URL_SERVICE_HOME);
	WB1.list.SetFont(8, 14, 10011000b);
	WB1.list.no_selection = true;
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	loop() switch(WaitEvent())
	{
		case evMouse:
			edit_box_mouse stdcall (#address_box);
			mouse.get();
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
			if (GetButtonID()==1) ExitProcess();
			break;

		case evKey:
			GetKeys();

			if (SCAN_CODE_F5 == key_scancode) {
				OpenPage(#current_path);
			}
			if (SCAN_CODE_F3 == key_scancode) {
				RunProgram("/rd/1/tinypad", #current_path);
			}

			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) {
				if (key_scancode == SCAN_CODE_KEY_O) {EventOpenDialog();break;} 
			}

			if (address_box.flags & 0b10)  
			{
				if (key_ascii == ASCII_KEY_ENTER) {
					OpenPage(#edit_path);
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
				if (WB1.list.ProcessKey(key_scancode)) WB1.DrawPage();
			}
			break;

		case evReDraw:
			DefineAndDrawWindow(GetScreenWidth()-800/2-random(80),GetScreenHeight()-600/2-random(80),800,600,0x73,col_bg,0,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) { DrawTitle(#header); break; }
			if (Form.height<120) { MoveSize(OLD,OLD,OLD,120); break; }
			if (Form.width<280) { MoveSize(OLD,OLD,280,OLD); break; }
			SetElementSizes();
			draw_window();
			break;
	}
}

void SetElementSizes()
{
	address_box.top = TOOLBAR_H/2-10;
	basic_line_h = calc(WB1.list.font_h * 130) / 100;
	address_box.left = address_box.top;
	address_box.width = Form.cwidth - address_box.left - address_box.left -14;
	WB1.list.SetSizes(0, TOOLBAR_H, Form.width - 10 - scroll_wv.size_x, 
		Form.cheight - TOOLBAR_H - STATUSBAR_H, basic_line_h);
	WB1.list.wheel_size = 7 * basic_line_h;
	WB1.list.column_max = WB1.list.w - scroll_wv.size_x / WB1.list.font_w;
	WB1.list.visible = WB1.list.h;
	if (WB1.list.w!=WB1.DrawBuf.bufw) {
		WB1.DrawBuf.Init(WB1.list.x, WB1.list.y, WB1.list.w, 32700);
		OpenPage(#current_path);
	}
}

void draw_window()
{
	DrawBar(0,0, Form.cwidth,TOOLBAR_H-2, panel_color);
	DrawBar(0,TOOLBAR_H-2, Form.cwidth,1, 0xD7D0D3);
	DrawBar(0,TOOLBAR_H-1, Form.cwidth,1, border_color);
	DrawRectangle(address_box.left-3, address_box.top-3, address_box.width+4, 25,border_color);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,STATUSBAR_H, col_bg);
	DrawBar(0,Form.cheight - STATUSBAR_H, Form.cwidth,1, border_color);
	DrawEditBoxWebView();
	if (!header) {
		OpenPage(#current_path);
	} else { 
		WB1.DrawPage(); 
		DrawEditBoxWebView(); 
	}
	DrawRectangle(scroll_wv.start_x, scroll_wv.start_y, scroll_wv.size_x, scroll_wv.size_y-1, scroll_wv.bckg_col);
}


void OpenPage(dword _path)
{
	dword buf, size;
	strcpy(#current_path, _path);
	if (streq(_path, URL_SERVICE_HOME)) {
		LoadInternalPage(#homepage, sizeof(homepage)); 
		return;
	}
	file_size stdcall (_path);
	if (EBX)
	{
		size = EBX;
		buf = malloc(size);
		ReadFile(0, size, buf, _path);
		ShowCodeSource();
		free(buf);
		return;
	}
	LoadInternalPage(NULL,NULL);
}

DrawEditBoxWebView()
{
	int skin_x_offset;
	DrawBar(address_box.left-2, address_box.top-2, address_box.width+3, 2, address_box.color);
	DrawBar(address_box.left-2, address_box.top, 2, 22, address_box.color);
	address_box.size = address_box.pos = address_box.shift = address_box.shift_old = strlen(#edit_path);
	address_box.offset = 0;
	edit_box_draw stdcall(#address_box);
	skin_x_offset = 51;
	img_draw stdcall(skin.image, address_box.left+address_box.width+1, address_box.top-3, 17, skin.h, skin_x_offset, SKIN_Y);
}

void LoadInternalPage(dword _bufpos, _bufsize)
{
	if (bufpointer) free(bufpointer);

	if (!_bufpos) || (!_bufsize) {
		LoadInternalPage(#page_not_found, sizeof(page_not_found));
		return;
	}
	strcpy(#edit_path, #current_path);
	DrawEditBoxWebView();

	bufpointer = _bufpos;
	bufsize = _bufsize;
	bufpointer = malloc(bufsize);
	strcpy(bufpointer, _bufpos);

	WB1.list.first = 0;
	WB1.ParseHtml();
	WB1.DrawPage();
}

void EventOpenDialog()
{
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		OpenPage(#openfile_path);
	}
}


void DrawStatusBar() {return;};
void EventClickLink() {return;};
void EventShowLinkMenu() {return;};

char anchor[256];

stop: