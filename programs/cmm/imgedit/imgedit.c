#define MEMSIZE 1024*100

#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/mem.h"
#include "../lib/cursor.h"
#include "../lib/list_box.h"
#include "../lib/events.h"

#include "../lib/obj/libimg.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/proc_lib.h"

#include "../lib/patterns/rgb.h"
#include "../lib/patterns/toolbar_button.h"
#include "../lib/patterns/simple_open_dialog.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define PAD 13
#define TOOLBAR_ITEM_H PAD+PAD
#define TOOLBAR_W 132
#define STATUSBAR_H 20
#define ISIZE 18

block canvas = { TOOLBAR_W + PAD + PAD, 0, NULL, NULL };

EVENTS button;
EVENTS key;

proc_info Form;
dword semi_white;

char default_dir[4096] = "/sys";
od_filter filter2 = { 69, "BMP\0GIF\0ICO\0CUR\0JPEG\0JPG\0PNG\0PNM\0TGA\0TIFF\0TIF\0WBMP\0XBM\0XCF\Z80\0\0" };

libimg_image icons18;
libimg_image main_image;

char win_title[256] = "ImageEdit";

scroll_bar scroll_v = { 15,NULL,NULL,NULL,15,2,NULL,0,0,0xeeeeee,0xBBDDFF,0xeeeeee};
scroll_bar scroll_h = { NULL,NULL,15,NULL,15,2,NULL,0,0,0xeeeeee,0xBBDDFF,0xeeeeee};

enum { SAVE_AS_PNG=1, SAVE_AS_BMP=2, SAVE_AS_RAW=4 };
int saving_type=SAVE_AS_PNG;

char* libimg_bpp[] = { "8pal", "24", "32", "15", "16",
"mono", "8gray", "2pal", "4pal", "8pal" };

enum {
	TOOL_CROP=1, 
	TOOL_RESIZE=2,
	TOOL_COLOR_DEPTH=4,
	TOOL_FLIP_ROTATE=8
};
int active_tool = NULL;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void init_ui()
{
	sc.get();
	semi_white = MixColors(sc.work, 0xFFFfff, skin_is_dark()*90 + 96);
	icons18.load("/sys/icons16.png");
	icons18.replace_color(0xffFFFfff, semi_white);
	icons18.replace_color(0xffCACBD6, MixColors(sc.work, 0, 200));
}

void main()
{
	int pressed_button_id;
	load_dll(libimg, #libimg_init, 1);
	load_dll(boxlib, #box_lib_init,0);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);

	init_ui();
	open_image("/sys/home.png");

	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop() switch(WaitEvent())
	{
		case evMouse:
			mouse.get();
			scrollbar_v_mouse stdcall(#scroll_v);
			scrollbar_h_mouse stdcall(#scroll_h);
			if (scroll_v.delta) || (scroll_h.delta) draw_canvas();
			if (EAX = mouse.vert) {
				if (EAX<10) event_scroll_canvas(SCAN_CODE_DOWN); 
				else event_scroll_canvas(SCAN_CODE_UP);
			} if (EAX = mouse.hor) {
				debugval("mouse.hor", mouse.hor);
				if (EAX<10) event_scroll_canvas(SCAN_CODE_RIGHT); 
				else event_scroll_canvas(SCAN_CODE_LEFT);
			}
			break;

		case evButton:
			pressed_button_id = GetButtonID();
			if (pressed_button_id==1) ExitProcess();
			button.press(pressed_button_id);
			break;

		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_DOWN) event_scroll_canvas(SCAN_CODE_DOWN);
			if (key_scancode == SCAN_CODE_UP) event_scroll_canvas(SCAN_CODE_UP);
			if (key_scancode == SCAN_CODE_LEFT) event_scroll_canvas(SCAN_CODE_LEFT);
			if (key_scancode == SCAN_CODE_RIGHT) event_scroll_canvas(SCAN_CODE_RIGHT);
			key.press(key_scancode);
			break;

		case evReDraw:
			draw_window();
			break;
	}
}

void draw_window()
{
	incn tx;
	char save_as_type[32];
	sc.get();
	Form.width = screen.w/6*5;
	Form.height = screen.h/6*5;
	DefineAndDrawWindow(screen.w-Form.width/2, screen.h-Form.height/2, Form.width, Form.height, 0x73, NULL, #win_title, 0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window&ROLLED_UP) return;
	if (Form.width  < 560) { MoveSize(OLD,OLD,560,OLD); return; }
	if (Form.height < 430) { MoveSize(OLD,OLD,OLD,430); return; }
	button.init(40);
	key.init(40);

	DrawBar(0, 0, canvas.x, Form.cheight, sc.work);

	canvas.w = Form.cwidth - canvas.x;
	canvas.h = Form.cheight - STATUSBAR_H;
	if (main_image.h > canvas.h) canvas.w -= scroll_v.size_x + 1;
	if (main_image.w > canvas.w) canvas.h -= scroll_h.size_y + 1;

	DrawBar(canvas.x, 0, 1, canvas.h, sc.work_text);
	if (main_image.h > canvas.h) && (main_image.w > canvas.w) {
		DrawBar(canvas.x+canvas.w, canvas.y+canvas.h, scroll_v.size_x+1, scroll_h.size_y+1, sc.work);
	}

	scroll_v.all_redraw = scroll_h.all_redraw = 1;
	if (skin_is_dark()) 
	{
		scroll_v.bckg_col = scroll_h.bckg_col = sc.work_light;
		scroll_v.frnt_col = scroll_h.frnt_col = sc.button;
		scroll_v.line_col = scroll_h.line_col = sc.button_text;
	}
	scroll_v.line_col = scroll_h.line_col = sc.work_text;		

	#define GAP_S TOOLBAR_ITEM_H+8
	#define GAP_B TOOLBAR_ITEM_H+23
	tx.set(PAD-GAP_S);
	//draw_tool_btn(10, ECTRL + SCAN_CODE_KEY_N, PAD, tx.inc(GAP_S), 02, "Create image", false);
	draw_tool_btn(#event_open, ECTRL + SCAN_CODE_KEY_O, PAD, tx.inc(GAP_S), 00, "Open image", false);
	//draw_tool_btn(13, ECTRL + SCAN_CODE_LEFT,  PAD, tx.inc(GAP_B), 30, false);
	//draw_tool_btn(14, ECTRL + SCAN_CODE_RIGHT, PAD, tx.inc(GAP_S), 31, false);
	//draw_tool_btn(15, ECTRL + SCAN_CODE_UP,    PAD, tx.inc(GAP_S), 32, false);
	//draw_tool_btn(16, ECTRL + SCAN_CODE_DOWN,  PAD, tx.inc(GAP_S), 33, false);

 	draw_tool_btn(#event_save, ECTRL + SCAN_CODE_KEY_S, PAD, tx.inc(GAP_B), 05, "Save file", false);
 	draw_tool_btn(#event_save_png, 0, PAD, tx.inc(GAP_S), -1, "PNG", saving_type & SAVE_AS_PNG);
	draw_tool_btn(#event_save_bmp, 0, PAD*2+34, tx.n,     -1, "BMP", saving_type & SAVE_AS_BMP);
	draw_tool_btn(#event_save_raw, 0, PAD*3+68, tx.n,     -1, "RAW", saving_type & SAVE_AS_RAW);

 	draw_tool_btn(#event_activate_crop,  0, PAD, tx.inc(GAP_B), 46, "Crop", active_tool & TOOL_CROP);
	draw_tool_btn(#event_activate_resize,0, PAD, tx.inc(GAP_S), 06, "Resize", active_tool & TOOL_RESIZE);
	draw_tool_btn(#event_activate_depth, 0, PAD, tx.inc(GAP_S), 52, "Color depth", active_tool & TOOL_COLOR_DEPTH);
	draw_tool_btn(#event_activate_flprot,0, PAD, tx.inc(GAP_S), 36, "Flip/Rotate", active_tool & TOOL_FLIP_ROTATE);
	//draw_tool_btn(#event_flip_hor, ECTRL + SCAN_CODE_KEY_H, PAD,      tx.inc(GAP_S), 34, NULL, false);
	//draw_tool_btn(#event_flip_ver, ECTRL + SCAN_CODE_KEY_V, PAD*2+34, tx.n,          35, NULL, false);
	//draw_tool_btn(#event_rotate,   ECTRL + SCAN_CODE_KEY_R, PAD*3+68, tx.n,          36, NULL, false);

	draw_status_bar();
	draw_canvas();
}

void draw_status_bar()
{
	char img_info[24];
	//draw_image_info
	sprintf(#img_info, "%ix%i@%s", main_image.w, main_image.h, libimg_bpp[main_image.type-1]);
	DrawBar(canvas.x, Form.cheight - STATUSBAR_H, Form.cwidth - canvas.x, STATUSBAR_H, sc.work);
	WriteText(canvas.x, Form.cheight - STATUSBAR_H + 2, 0x90, sc.work_text, #img_info);
}

void draw_tool_btn(dword _event, _hotkey, _x, _y, _icon_n, _text, _active)
{
	int w = TOOLBAR_W;
	if (!_text) w = PAD + PAD + 12;
	if (_icon_n==-1) w = strlen(_text) * 8 + 14;
	if (_active) EDX = sc.button; else EDX = semi_white;
	DrawBar(_x, _y, w, TOOLBAR_ITEM_H+1, EDX);
	PutPixel(_x,_y,sc.work);
	PutPixel(_x,_y+TOOLBAR_ITEM_H,sc.work);
	PutPixel(_x+w-1,_y,sc.work);
	PutPixel(_x+w-1,_y+TOOLBAR_ITEM_H,sc.work);
	if (_event) DefineHiddenButton(_x, _y, w, TOOLBAR_ITEM_H, button.add(_event));
	if (_hotkey) key.add_n(_hotkey, _event);
	if (_icon_n!=-1) {
		img_draw stdcall(icons18.image, _x+7, _y+4, ISIZE, ISIZE, 0, _icon_n*ISIZE);
		_x += PAD+ISIZE+2;
	} else {
		_x += 7;
	}
	if (_text) {
		if (_active) EDX = sc.button_text; else EDX = sc.work_text;
		WriteText(_x, _y+6, 0x90, EDX, _text);
	}
}

void draw_scroll_v()
{
	scroll_v.max_area = main_image.h;
	scroll_v.cur_area = scroll_v.size_y = canvas.h;
	scroll_v.start_x = canvas.x + canvas.w;
	scroll_v.start_y = 0;
	if (main_image.h > canvas.h) scrollbar_v_draw stdcall (#scroll_v);
}

void draw_scroll_h()
{
	scroll_h.max_area = main_image.w;
	scroll_h.cur_area = scroll_h.size_x = canvas.w;
	scroll_h.start_x = canvas.x;
	scroll_h.start_y = canvas.y + canvas.h;
	if (main_image.w > canvas.w) scrollbar_h_draw stdcall (#scroll_h);
}

void draw_canvas()
{
	int content_w, content_h;
	content_w = math.min(main_image.w, canvas.w-1);
	content_h = math.min(main_image.h, canvas.h);

	if (main_image.image) {
		img_draw stdcall(main_image.image, canvas.x+1, canvas.y,
		content_w, content_h, scroll_h.position, scroll_v.position);
 	}

	DrawBar(canvas.x+1+content_w, canvas.y, canvas.w - content_w - 1, content_h, 0xBFCAD2);
	DrawBar(canvas.x+1, canvas.y+content_h, canvas.w - 1, canvas.h - content_h, 0xBFCAD2);

	draw_scroll_v();
	draw_scroll_h();
}

void update_title(char* _new_title)
{
	strcpy(#win_title+9, " - ");
	strlcpy(#win_title+12, _new_title, sizeof(win_title));
	DrawTitle(#win_title);
}

void open_image(char* _path)
{
	main_image.load(_path);
	update_title(_path);
	scroll_v.position = 0;
	scroll_h.position = 0;
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void event_open()
{
	o_dialog.type = 0; //open file
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		open_image(#openfile_path);
		draw_canvas();
	}
}

void event_flip_hor()
{
	img_flip stdcall (main_image.image, FLIP_HORIZONTAL);
	draw_canvas();
}

void event_flip_ver()
{
	img_flip stdcall (main_image.image, FLIP_VERTICAL);
	draw_canvas();
}

void event_rotate()
{
	img_rotate stdcall (main_image.image, ROTATE_90_CW);
	main_image.w >< main_image.h;
	draw_window();
}

void event_save_png() { saving_type = SAVE_AS_PNG; draw_window(); }
void event_save_bmp() { saving_type = SAVE_AS_BMP; draw_window(); }
void event_save_raw() { saving_type = SAVE_AS_RAW; draw_window(); }

void event_activate_crop() { active_tool = TOOL_CROP; draw_window(); }
void event_activate_resize() { active_tool = TOOL_RESIZE; draw_window(); }
void event_activate_depth() { active_tool = TOOL_COLOR_DEPTH; draw_window(); }
void event_activate_flprot() { active_tool = TOOL_FLIP_ROTATE; draw_window(); }

void event_save()
{
	o_dialog.type = 1; //save file
	switch (saving_type) {
		case SAVE_AS_PNG:
				strcpy(#filename_area, "image.png");
				OpenDialog_start stdcall (#o_dialog);
				if (o_dialog.status) {
					update_title(#openfile_path);
					img_to_rgb stdcall (main_image.image);
					save_image(main_image.imgsrc, main_image.w, main_image.h, #openfile_path);
				}
				break;
		case SAVE_AS_BMP:
				notify("Not implemented yet.");
				break;
		case SAVE_AS_RAW:
				notify("Not implemented yet.");
				break;
	}
}

void event_scroll_canvas(int _direction)
{
	switch(_direction) {
		case SCAN_CODE_DOWN:
			scroll_v.position = math.min(scroll_v.position+25, 
				scroll_v.max_area - scroll_v.cur_area);
			break;
		case SCAN_CODE_UP:
			scroll_v.position = math.max(scroll_v.position-25, 0);
			break;
		case SCAN_CODE_RIGHT:
			scroll_h.position = math.min(scroll_h.position+25, 
				scroll_h.max_area - scroll_h.cur_area);
			break;
		case SCAN_CODE_LEFT:
			scroll_h.position = math.max(scroll_h.position-25, 0);
	}
	draw_canvas();
}

stop:

