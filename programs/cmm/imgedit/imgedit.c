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

#define PAD 12
#define TOOLBAR_ITEM_H PAD+PAD
#define TOOLBAR_W 110
#define ISIZE 18

block canvas = { TOOLBAR_W + PAD + PAD, 0, NULL, NULL };

EVENTS button;
EVENTS key;

proc_info Form;
dword semi_white;
bool bg_dark=false;

char default_dir[4096] = "/rd/1";
od_filter filter2 = { 69, "BMP\0GIF\0ICO\0CUR\0JPEG\0JPG\0PNG\0PNM\0TGA\0TIFF\0TIF\0WBMP\0XBM\0XCF\Z80\0\0" };

libimg_image icons18;
libimg_image main_image;

char win_title[256] = "ImageEdit";

scroll_bar scroll_v = { 15,NULL,NULL,NULL,15,2,NULL,0,0,0xeeeeee,0xBBBbbb,0xeeeeee};
scroll_bar scroll_h = { NULL,NULL,15,NULL,15,2,NULL,0,0,0xeeeeee,0xBBBbbb,0xeeeeee};

/*
struct scroll_bar
{
	word size_x, start_x, size_y, start_y;
	dword btn_height, type, max_area, cur_area, position,
	bckg_col, frnt_col, line_col, redraw;
	word delta, delta2, r_size_x, r_start_x, r_size_y, r_start_y;
	dword m_pos, m_pos_2, m_keys, run_size, position2, work_size, all_redraw, ar_offset;
};
*/

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void init_ui()
{
	sc.get();
	semi_white = MixColors(sc.work, 0xFFFfff, bg_dark*90 + 96);
	bg_dark = skin_is_dark();
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
			break;

		case evButton:
			pressed_button_id = GetButtonID();
			if (pressed_button_id==1) ExitProcess();
			button.press(pressed_button_id);
			break;

		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_DOWN) {
				scroll_v.position = math.min(scroll_v.position+25, scroll_v.max_area - scroll_v.cur_area);
				draw_canvas();
			}
			if (key_scancode == SCAN_CODE_UP) {
				scroll_v.position = math.max(scroll_v.position-25, 0);
				draw_canvas();
			}
			break;

		case evReDraw:
			draw_window();
			break;
	}
}

void draw_window()
{
	incn tx;
	DefineAndDrawWindow(random(100)+40, 40+random(100), screen.width/3*2, screen.height/3*2, 0x73, NULL, #win_title, 0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window&ROLLED_UP) return;
	if (Form.width  < 560) { MoveSize(OLD,OLD,560,OLD); return; }
	if (Form.height < 430) { MoveSize(OLD,OLD,OLD,430); return; }
	button.init(40);
	key.init(40);

	DrawBar(0, 0, canvas.x, Form.cheight, sc.work);

	canvas.w = Form.cwidth - canvas.x;
	canvas.h = Form.cheight;
	if (main_image.h > canvas.h) canvas.w -= scroll_v.size_x + 1;
	if (main_image.w > canvas.w) canvas.h -= scroll_h.size_y + 1;

	DrawBar(canvas.x, 0, 1, canvas.h, sc.work_graph);
	if (main_image.h > canvas.h) && (main_image.w > canvas.w) {
		DrawBar(canvas.x+canvas.w, canvas.y+canvas.h, scroll_v.size_x+1, scroll_h.size_y+1, sc.work);
	}

	scroll_v.all_redraw = scroll_h.all_redraw = 1;
	scroll_v.bckg_col = scroll_h.bckg_col = MixColors(sc.work, 0xBBBbbb, 80);
	scroll_v.frnt_col = scroll_h.frnt_col = MixColors(sc.work,0xFFFfff,120);
	scroll_v.line_col = scroll_h.line_col = sc.work_graph;

	#define GAP_S 24+7
	#define GAP_B 24+23
	tx.set(PAD-GAP_S);
	//draw_icon(10, ECTRL + SCAN_CODE_KEY_N, PAD, tx.inc(GAP_S), 02, "Create image");
	draw_icon(#event_open, ECTRL + SCAN_CODE_KEY_O, PAD, tx.inc(GAP_S), 00, "Open image");
	//draw_icon(13, ECTRL + SCAN_CODE_LEFT,  PAD, tx.inc(GAP_B), 30);
	//draw_icon(14, ECTRL + SCAN_CODE_RIGHT, PAD, tx.inc(GAP_S), 31);
	//draw_icon(15, ECTRL + SCAN_CODE_UP,    PAD, tx.inc(GAP_S), 32);
	//draw_icon(16, ECTRL + SCAN_CODE_DOWN,  PAD, tx.inc(GAP_S), 33);

 	//draw_icon(#event_save,     ECTRL + SCAN_CODE_KEY_S, PAD, tx.inc(GAP_B), 05, "Save file");
 	//draw_icon(12, 0,                                    PAD, tx.inc(GAP_B), 05, "PNG");
	//draw_icon(12, 0,                                    PAD, tx.inc(GAP_S), 05, "BMP");
	//draw_icon(12, 0,                                    PAD, tx.inc(GAP_S), 05, "RAW");
	draw_icon(#event_save,     ECTRL + SCAN_CODE_KEY_S, PAD, tx.inc(GAP_S), 05, "Save as PNG");

 	draw_icon(0, 0,                       PAD, tx.inc(GAP_B), 46, "Crop");
	draw_icon(0, 0,                       PAD, tx.inc(GAP_S), 06, "Resize");
	draw_icon(0, 0,                       PAD, tx.inc(GAP_S), 52, "Color depth");
	draw_icon(#event_flip_hor, ECTRL + SCAN_CODE_KEY_H, PAD,     tx.inc(GAP_S), 34, NULL);
	draw_icon(#event_flip_ver, ECTRL + SCAN_CODE_KEY_V, PAD*4+3, tx.n,          35, NULL);
	draw_icon(#event_rotate,   ECTRL + SCAN_CODE_KEY_R, PAD*7+6, tx.n,          36, NULL);

	draw_image_info(tx.inc(GAP_B));
	draw_canvas();
}

char* libimg_bpp[] = { "8 pal", "24", "32", "15", "16",
"1 mono", "8 gray", "2 pal", "4 pal", "8 pal" };

void draw_image_info(int _y)
{
	WriteText(PAD, _y, 0x90, sc.work_text, "Properties");
	DrawBar(PAD, _y+14, TOOLBAR_W, 1, sc.work_graph);
	WriteText(PAD, _y+22, 0x90, sc.work_text, "Width:");
	WriteText(PAD, _y+42, 0x90, sc.work_text, "Heigh:");
	WriteText(PAD, _y+62, 0x90, sc.work_text, "Depth:");

	WriteText(PAD+60, _y+22, 0x90, sc.work_text, itoa(main_image.w));
	WriteText(PAD+60, _y+42, 0x90, sc.work_text, itoa(main_image.h));
	WriteText(PAD+60, _y+62, 0x90, sc.work_text, libimg_bpp[main_image.type-1]);
}

void draw_icon(dword _event, _key, _x, _y, _icon_n, _text)
{
	int w;
	if (_text) w = TOOLBAR_W; else w = PAD + PAD + 8;
	DrawBar(_x, _y, w, TOOLBAR_ITEM_H+1, semi_white);
	PutPixel(_x,_y,sc.work);
	PutPixel(_x,_y+TOOLBAR_ITEM_H,sc.work);
	PutPixel(_x+w-1,_y,sc.work);
	PutPixel(_x+w-1,_y+TOOLBAR_ITEM_H,sc.work);
	if (_event) DefineHiddenButton(_x, _y, w, TOOLBAR_ITEM_H, button.add(_event));
	if (_text) WriteText(_x+PAD+ISIZE+2, _y+9, 0x80, sc.work_text, _text);
	img_draw stdcall(icons18.image, _x+7, _y+3, ISIZE, ISIZE, 0, _icon_n*ISIZE);
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

void event_save()
{
	o_dialog.type = 1; //save file
	strcpy(#filename_area, "image.png");
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		update_title(#openfile_path);
		img_to_rgb stdcall (main_image.image);
		save_image(main_image.imgsrc, main_image.w, main_image.h, #openfile_path);
	}
}

stop:

