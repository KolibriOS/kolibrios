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

//Sizes
#define PAD 13
#define TOOLBAR_ITEM_H PAD + PAD
#define TOOLBAR_W 132
#define STATUSBAR_H 20
#define HEADERH TOOLBAR_ITEM_H + 14
#define HEADER_TEXTY HEADERH - 14 / 2
#define CANVASX TOOLBAR_W + PAD + PAD
#define CANVASY HEADERH + 2

// Colors
#define COL_WORK        0x242424
#define COL_WORK_TEXT   0xBEBEBE
#define COL_LIGHT       0x424242
#define COL_DARK        0x1D1D1D
#define COL_LINE        0x010101
#define COL_BUTTON      0x181818
#define COL_BUTTON_TEXT 0x18A2CC

block canvas = { CANVASX, CANVASY, NULL, NULL };

EVENTS button;
EVENTS key;

proc_info Form;
int pressed_button_id;

char default_dir[4096] = "/sys";
od_filter filter2 = { 69, "BMP\0GIF\0ICO\0CUR\0JPEG\0JPG\0PNG\0PNM\0TGA\0TIFF\0TIF\0WBMP\0XBM\0XCF\Z80\0\0" };

libimg_image icons18;
libimg_image icons18a;
libimg_image pixie_skin;
libimg_image main_image;

scroll_bar scroll_v = { 15,NULL,NULL,HEADERH+1,15,2,NULL,0,0,COL_DARK,COL_LIGHT,COL_LINE};
scroll_bar scroll_h = { NULL,TOOLBAR_W+PAD+PAD,15,NULL,15,2,NULL,0,0,COL_DARK,COL_LIGHT,COL_LINE};

enum { SAVE_AS_PNG=1, SAVE_AS_BMP=2, SAVE_AS_RAW=4 };
int saving_type=SAVE_AS_PNG;

char* libimg_bpp[] = { "-", "8pal", "24", "32", "15", "16",
"mono", "8gray", "2pal", "4pal", "8gr/a" };

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
	icons18.load("/sys/icons16.png");
	icons18.replace_2colors(0xffFFFfff, COL_LIGHT, 0xffCACBD6, COL_LIGHT-0x080808);
	icons18a.load("/sys/icons16.png");
	icons18a.replace_2colors(0xffFFFfff, COL_BUTTON, 0xffCACBD6, 0);

	pixie_skin.load("/sys/media/pixieskn.png");
}

void main()
{
	load_dll(libimg, #libimg_init, 1);
	load_dll(boxlib, #box_lib_init,0);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);

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
	Form.width = screen.w/6*5;
	Form.height = screen.h/6*5;
	DefineAndDrawWindow(screen.w-Form.width/2, screen.h-Form.height/2, Form.width, Form.height, 0x42, NULL, NULL, 0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window&ROLLED_UP) return;
	if (Form.width  < 560) { MoveSize(OLD,OLD,560,OLD); return; }
	if (Form.height < 430) { MoveSize(OLD,OLD,OLD,430); return; }
	scroll_v.all_redraw = scroll_h.all_redraw = 1;
	draw_content();
}

void draw_content()
{
	incn tx;
	button.init(40);
	key.init(40);

	init_ui();
	canvas.w = Form.cwidth - CANVASX - 1;
	canvas.h = Form.cheight - STATUSBAR_H - CANVASY - 1;
	if (main_image.h > canvas.h) canvas.w -= scroll_v.size_x + 1;
	if (main_image.w > canvas.w) canvas.h -= scroll_h.size_y + 1;

	//window border and panel border
	DrawRectangle(0,0,Form.width,Form.height,COL_LINE);
	DrawRectangle(1,1,Form.width-2,Form.height-2,COL_LIGHT);
	DrawBar(CANVASX, 0, 1, Form.cheight, COL_LINE);

	//draw title
	DrawBar(2, 2, CANVASX-2, HEADERH-1, COL_DARK);
	WriteText(PAD+5, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "ImageEditor Pro");

	//draw header
	DrawBar(CANVASX+1, 2, Form.width-CANVASX-2, HEADERH-1, COL_WORK);
	DrawBar(CANVASX+1, CANVASY-1, Form.width-CANVASX-2, 1, COL_LINE);
	img_draw stdcall(pixie_skin.image, Form.width-63, 7, 57, 18, 265, 0);
	DefineHiddenButton(Form.width-63, 7, 28, 17, button.add(#MinimizeWindow));
	DefineHiddenButton(Form.width-35, 7, 28, 17, 1);
	draw_acive_panel();

	//left panel bg
	DrawBar(2, 1+HEADERH, CANVASX-2, Form.cheight-2-HEADERH, COL_WORK);

	if (main_image.h > canvas.h) && (main_image.w > canvas.w) {
		DrawBar(CANVASX+canvas.w, CANVASY+canvas.h, scroll_v.size_x+1, scroll_h.size_y+1, COL_WORK);
	}

	#define GAP_S TOOLBAR_ITEM_H+8
	#define GAP_B TOOLBAR_ITEM_H+23
	tx.set(PAD-GAP_S+HEADERH);
	//draw_tool_btn(10, ECTRL + SCAN_CODE_KEY_N, PAD, tx.inc(GAP_S), 02, "Create image", false);
	draw_tool_btn(#event_open, ECTRL + SCAN_CODE_KEY_O, PAD, tx.inc(GAP_S), 00, "Open image", false);
	//draw_tool_btn(13, ECTRL + SCAN_CODE_LEFT,  PAD, tx.inc(GAP_B), 30, false);
	//draw_tool_btn(14, ECTRL + SCAN_CODE_RIGHT, PAD, tx.inc(GAP_S), 31, false);
	//draw_tool_btn(15, ECTRL + SCAN_CODE_UP,    PAD, tx.inc(GAP_S), 32, false);
	//draw_tool_btn(16, ECTRL + SCAN_CODE_DOWN,  PAD, tx.inc(GAP_S), 33, false);
	if (!main_image.image) goto _NO_IMAGE;

 	draw_tool_btn(#event_save, ECTRL + SCAN_CODE_KEY_S, PAD, tx.inc(GAP_B), 05, "Save as", false);
 	draw_tool_btn(#event_save_png, 0, PAD, tx.inc(GAP_S), -1, "PNG", saving_type & SAVE_AS_PNG);
	draw_tool_btn(#event_save_bmp, 0, PAD*2+34, tx.n,     -1, "BMP", saving_type & SAVE_AS_BMP);
	draw_tool_btn(#event_save_raw, 0, PAD*3+68, tx.n,     -1, "RAW", saving_type & SAVE_AS_RAW);

 	draw_tool_btn(#event_activate_crop,  0, PAD, tx.inc(GAP_B), 46, "Crop", active_tool & TOOL_CROP);
	draw_tool_btn(#event_activate_resize,0, PAD, tx.inc(GAP_S), 06, "Resize", active_tool & TOOL_RESIZE);
	draw_tool_btn(#event_activate_depth, 0, PAD, tx.inc(GAP_S), 52, "Color depth", active_tool & TOOL_COLOR_DEPTH);
	draw_tool_btn(#event_activate_flprot,0, PAD, tx.inc(GAP_S), 36, "Flip/Rotate", active_tool & TOOL_FLIP_ROTATE);
	_NO_IMAGE:

	draw_status_bar();
	draw_canvas();
}

void draw_status_bar()
{
	char img_info[24];
	sprintf(#img_info, "%i\01%i\02%s", main_image.w, main_image.h, libimg_bpp[main_image.type]);
	DrawBar(CANVASX+1, Form.cheight - STATUSBAR_H - 1, Form.cwidth - CANVASX -2, STATUSBAR_H, COL_WORK);
	WriteText(CANVASX+4, Form.cheight - STATUSBAR_H + 2, 0x90, COL_WORK_TEXT, #img_info);
	for (ESI=0; img_info[ESI]!=0; ESI++) {
		if (img_info[ESI] == '\01') img_info[ESI]='x';
		else if (img_info[ESI] == '\02') img_info[ESI]='@';
		else img_info[ESI]=' ';
	}
	ECX = 0x90 << 24 + COL_BUTTON_TEXT;
	$int 64
}

int draw_tool_btn(dword _event, _hotkey, _x, _y, _icon_n, _text, _active)
{
	int w = TOOLBAR_W;
	dword img_ptr = icons18.image;
	if (!_text) w = PAD + PAD + 6;
	if (_icon_n==-1) w = strlen(_text) * 8 + 14;
	if (_active==-1) {
		$push COL_LIGHT
		EDX = COL_LINE;
	} else if (_active) {
		img_ptr = icons18a.image;
		$push COL_BUTTON_TEXT
		EDX = COL_BUTTON;

	} else {
		$push COL_WORK_TEXT
		EDX = COL_LIGHT;
	}
	DrawBar(_x, _y, w, TOOLBAR_ITEM_H+1, EDX);
	PutPixel(_x,_y,COL_WORK);
	PutPixel(_x,_y+TOOLBAR_ITEM_H,COL_WORK);
	PutPixel(_x+w-1,_y,COL_WORK);
	PutPixel(_x+w-1,_y+TOOLBAR_ITEM_H,COL_WORK);
	if (_event) DefineHiddenButton(_x, _y, w, TOOLBAR_ITEM_H, button.add(_event));
	if (_hotkey) key.add_n(_hotkey, _event);
	if (_icon_n!=-1) {
		#define ISIZE 18
		img_draw stdcall(img_ptr, _x+7, _y+4, ISIZE, ISIZE, 0, _icon_n*ISIZE);
		_x += PAD+ISIZE+2;
	} else {
		_x += 7;
	}
	$pop EDX
	if (_text) {
		WriteText(_x, _y+6, 0x90, EDX, _text);
	}
	return w;
}

void draw_canvas()
{
	int content_w = math.min(main_image.w, canvas.w-1);
	int content_h = math.min(main_image.h, canvas.h);
	if (main_image.image) {
		img_draw stdcall(main_image.image, CANVASX+1, CANVASY,
		content_w, content_h, scroll_h.position, scroll_v.position);
 	}
	DrawBar(CANVASX+1+content_w, CANVASY, canvas.w - content_w - 1, content_h, COL_DARK);
	DrawBar(CANVASX+1, CANVASY+content_h, canvas.w - 1, canvas.h - content_h, COL_DARK);
	//Draw scroll V
	scroll_v.max_area = main_image.h;
	scroll_v.cur_area = scroll_v.size_y = canvas.h + 1;
	scroll_v.start_x = CANVASX + canvas.w;
	if (main_image.h > canvas.h) scrollbar_v_draw stdcall (#scroll_v);
	//Draw scroll H
	scroll_h.max_area = main_image.w;
	scroll_h.cur_area = scroll_h.size_x = canvas.w;
	scroll_h.start_y = CANVASY + canvas.h;
	if (main_image.w > canvas.w) scrollbar_h_draw stdcall (#scroll_h);

}

void set_file_path(char* _new_title)
{
	strcpy(#param, _new_title);
	draw_status_bar();
}

void open_image(char* _path)
{
	main_image.load(_path);
	set_file_path(_path);
	scroll_v.position = 0;
	scroll_h.position = 0;
}

int color_depth_id;
void event_set_color_depth() {
	img_convert stdcall(main_image.image, 0, pressed_button_id-color_depth_id, 0, 0);
	if (!EAX) {
		notify("'ImageEdit Pro\nConvertation error' -Et");
	} else {
		$push eax
		img_destroy stdcall(main_image.image);
		$pop eax
		main_image.image = EAX;
		main_image.set_vars();
		draw_acive_panel();
		draw_status_bar();
		draw_canvas();
	}
}

void draw_acive_panel()
{
	int i, x = CANVASX + PAD;
	bool a;
	switch(active_tool) {
		case TOOL_CROP:
			WriteText(CANVASX+PAD, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "Crop tool");
			break;
		case TOOL_RESIZE:
			WriteText(CANVASX+PAD, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "New width");
			WriteText(CANVASX+PAD+150, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "New height");
			draw_tool_btn(#event_rotate_left, SCAN_CODE_ENTER, CANVASX + PAD + 300, 7, -1, "Apply", false);
			break;
		case TOOL_COLOR_DEPTH:
			WriteText(CANVASX+PAD, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "Color depth");
			x += 11*8 + PAD;
			color_depth_id = button.new_id;
			for (i=1; i<11; i++) {
				if (main_image.type == i) {
					//this is current image depth
					a = true; 
				} else {
					//this is image ve san set
					a = false;
					//probe does libimg support converting current image gepth to i-one
					img_create stdcall(1, 1, main_image.type);
					img_convert stdcall(EAX, 0, i, 0, 0);
					if (EAX) {
						img_destroy stdcall(EAX); 
					} else {
						a = -1;
					}
				}
				x += draw_tool_btn(#event_set_color_depth, SCAN_CODE_ENTER, x, 7, -1, libimg_bpp[i], a) + PAD;			
			}
			break;
		case TOOL_FLIP_ROTATE:
			WriteText(CANVASX+PAD, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "Flip");
			draw_tool_btn(#event_flip_hor, ECTRL + SCAN_CODE_KEY_H, CANVASX + PAD + 040, 7, 34, NULL, false);
			draw_tool_btn(#event_flip_ver, ECTRL + SCAN_CODE_KEY_V, CANVASX + PAD + 080, 7, 35, NULL, false);
			WriteText(CANVASX+PAD + 142, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "Rotate");
			draw_tool_btn(#event_rotate_left,   ECTRL + SCAN_CODE_KEY_L, CANVASX + PAD + 200, 7, 37, NULL, false);
			draw_tool_btn(#event_rotate_right,   ECTRL + SCAN_CODE_KEY_R, CANVASX + PAD + 240, 7, 36, NULL, false);
			// WriteText(CANVASX+PAD + 142, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "Move");
			// DrawTopPanelButton1(#EventMoveLeft,       ECTRL + SCAN_CODE_LEFT,  tx.inc(GAP_B), 30);
			// DrawTopPanelButton1(#EventMoveRight,      ECTRL + SCAN_CODE_RIGHT, tx.inc(GAP_S), 31);
			// DrawTopPanelButton1(#EventMoveUp,         ECTRL + SCAN_CODE_UP,    tx.inc(GAP_S), 32);
			// DrawTopPanelButton1(#EventMoveDown,       ECTRL + SCAN_CODE_DOWN,  tx.inc(GAP_S), 33);
			break;
		default:
			WriteText(CANVASX+PAD, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "Welcome to ImageEditor Pro! Try to open a file.");
	}
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
		draw_window();
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

void event_rotate_left()
{
	img_rotate stdcall (main_image.image, ROTATE_270_CW);
	main_image.w >< main_image.h;
	draw_content();
}

void event_rotate_right()
{
	img_rotate stdcall (main_image.image, ROTATE_90_CW);
	main_image.w >< main_image.h;
	draw_content();
}

void event_save_png() { saving_type = SAVE_AS_PNG; draw_content(); }
void event_save_bmp() { saving_type = SAVE_AS_BMP; draw_content(); }
void event_save_raw() { saving_type = SAVE_AS_RAW; draw_content(); }

void event_activate_crop() { active_tool = TOOL_CROP; draw_content(); }
void event_activate_resize() { active_tool = TOOL_RESIZE; draw_content(); }
void event_activate_depth() { active_tool = TOOL_COLOR_DEPTH; draw_content(); }
void event_activate_flprot() { active_tool = TOOL_FLIP_ROTATE; draw_content(); }

void event_save()
{
	o_dialog.type = 1; //save file
	switch (saving_type) {
		case SAVE_AS_PNG:
				strcpy(#filename_area, "image.png");
				OpenDialog_start stdcall (#o_dialog);
				if (o_dialog.status) {
					set_file_path(#openfile_path);
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

