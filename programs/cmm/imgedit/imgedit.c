#define MEMSIZE 1024*100

/*=====================================================
|                                                     |
|                         LIB                         |
|                                                     |
=====================================================*/

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

/*=====================================================
|                                                     |
|                        DATA                         |
|                                                     |
=====================================================*/

#include "data.h"

/*=====================================================
|                                                     |
|                        CODE                         |
|                                                     |
=====================================================*/

void main()
{
	/* Init external libraries */
	load_dll(libimg, #libimg_init, 1);
	load_dll(boxlib, #box_lib_init,0);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);

	/* Init UI */
	icons18.load("/sys/icons16.png");
	icons18.replace_2colors(0xffFFFfff, COL_LIGHT, 0xffCACBD6, COL_LIGHT-0x080808);
	icons18a.load("/sys/icons16.png");
	icons18a.replace_2colors(0xffFFFfff, COL_BUTTON, 0xffCACBD6, 0);
	pixie_skin.load("/sys/media/pixieskn.png");
	Form.width = screen.w/6*5;
	Form.height = screen.h/6*5;

	/* Handle application parameters */
	//if (!param) open_image("/sys/home.png");

	/* Set event mask and go to main loop */
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
	loop() switch(WaitEvent())
	{
		case evMouse:
			mouse.get();
			if (main_image.h > canvas.h) scrollbar_v_mouse stdcall(#scroll_v);
			if (main_image.w > canvas.w) scrollbar_h_mouse stdcall(#scroll_h);
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
	DefineAndDrawWindow(screen.w-Form.width/2, screen.h-Form.height/2, Form.width, Form.height, 0x42, NULL, NULL, 0);
	GetProcessInfo(#Form, SelfInfo);
	/* Draw window borders */
	DrawRectangle(0,0,Form.width,Form.height,COL_LINE);
	DrawRectangle(1,1,Form.width-2,Form.height-2,COL_LIGHT);
	/* Check if the window is collapsed into header */
	if (Form.status_window&ROLLED_UP) {
		DrawBar(2, 2, Form.width-3, skin_h-4, COL_WORK);
		WriteText(5, skin_h-11/2, 0x90, COL_WORK_TEXT, "ImageEditor Pro [Collapsed]");
		draw_window_manipulation_buttons(skin_h-17/2);
		return;
	}
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

	canvas.w = Form.cwidth - CANVASX - 1;
	canvas.h = Form.cheight - STATUSBAR_H - CANVASY - 1;
	if (main_image.h > canvas.h) canvas.w -= scroll_v.size_x + 1;
	if (main_image.w > canvas.w) canvas.h -= scroll_h.size_y + 1;

	//draw panel border
	DrawBar(CANVASX, 0, 1, Form.cheight, COL_LINE);

	//draw title
	DrawBar(2, 2, CANVASX-2, HEADERH-1, COL_DARK);
	WriteText(PAD+5, HEADER_TEXTY, 0x90, COL_WORK_TEXT, "ImageEditor Pro");

	//draw header
	DrawBar(CANVASX+1, CANVASY-1, Form.width-CANVASX-2, 1, COL_LINE);
	draw_acive_panel();
	draw_window_manipulation_buttons(7);

	//left panel bg
	DrawBar(2, 1+HEADERH, CANVASX-2, Form.cheight-2-HEADERH, COL_WORK);

	if (main_image.h > canvas.h) && (main_image.w > canvas.w) {
		DrawBar(CANVASX+canvas.w, CANVASY+canvas.h, scroll_v.size_x+1, scroll_h.size_y+1, COL_WORK);
	}

	#define GAP_S TOOLBAR_ITEM_H+8
	#define GAP_B TOOLBAR_ITEM_H+23
	tx.set(PAD-GAP_S+HEADERH);
	//draw_tool_btn(10, ECTRL + SCAN_CODE_KEY_N, PAD, tx.inc(GAP_S), 02, "Create image", false);
	draw_tool_btn(#event_open, ECTRL + SCAN_CODE_KEY_O, PAD, tx.inc(GAP_S), 00, "Open image  ", false);
	if (main_image.image) {
	 	draw_tool_btn(#event_activate_export, ECTRL + SCAN_CODE_KEY_S, PAD, tx.inc(GAP_S), 05, "Export image", active_tool & TOOL_EXPORT);
	 	//draw_tool_btn(#event_activate_crop,  0, PAD, tx.inc(GAP_B), 46, "Crop", active_tool & TOOL_CROP);
		//draw_tool_btn(#event_activate_resize,0, PAD, tx.inc(GAP_B), 06, "Resize      ", active_tool & TOOL_RESIZE);
		draw_tool_btn(#event_activate_depth, 0, PAD, tx.inc(GAP_B), 52, "Color depth ", active_tool & TOOL_COLOR_DEPTH);
		draw_tool_btn(#event_activate_flprot,0, PAD, tx.inc(GAP_S), 36, "Flip/Rotate ", active_tool & TOOL_FLIP_ROTATE);		
	}
	draw_status_bar();
	draw_canvas();
}

void draw_window_manipulation_buttons(int y)
{
	DrawBar(Form.width-65, 2, 63, HEADERH-1, COL_WORK);
	img_draw stdcall(pixie_skin.image, Form.width-63, y, 57, 18, 265, 0);
	DefineHiddenButton(Form.width-63, y, 28, 17, button.add(#MinimizeWindow));
	DefineHiddenButton(Form.width-35, y, 28, 17, 1);
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
	dword img_ptr = icons18.image;

	int w = 32; //default width: icon only
	if (_text) {
		w = strlen(_text) * 8 + 15;
		if (_icon_n!=-1) w += 26;
	}

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

void event_set_color_depth() {
	img_convert stdcall(main_image.image, 0, pressed_button_id-color_depth_btnid_1, 0, 0);
	if (!EAX) {
		notify("'ImageEdit Pro\nNot possible to convert into specified color depth' -Et");
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

void WritePanelText(int _x, _text) { WriteTextWithBg(_x, HEADER_TEXTY, 0xD0, COL_WORK_TEXT, _text, COL_WORK); }
void draw_acive_panel()
{
	char edit_width_text[8];
	edit_box edit_width;
	int i, x = CANVASX + PAD;
	bool a;
	DrawBar(CANVASX+1, 2, Form.width-CANVASX-64, HEADERH-1, COL_WORK);
	switch(active_tool) {
		case TOOL_EXPORT:
			WritePanelText(CANVASX+PAD, "Format");
			x += 6*8 + PAD;
		 	x += draw_tool_btn(#event_save_png, 0, x, 7, -1, "PNG", saving_type & SAVE_AS_PNG) + PAD;
			x += draw_tool_btn(#event_save_pnm, 0, x, 7, -1, "PNM", saving_type & SAVE_AS_PNM) + PAD;
			x += draw_tool_btn(#event_save_bmp, 0, x, 7, -1, "BMP", saving_type & SAVE_AS_BMP) + PAD + PAD;
			if (saving_type) draw_tool_btn(#event_save,     0, x, 7, 53, "Export", false);
			break;
		case TOOL_CROP:
			WritePanelText(CANVASX+PAD, "Crop tool");
			break;
		case TOOL_RESIZE:
			WritePanelText(CANVASX+PAD, "New width");
			WritePanelText(CANVASX+PAD+150, "New height");
			draw_tool_btn(#event_rotate_left, SCAN_CODE_ENTER, CANVASX + PAD + 300, 7, -1, "Apply", false);
			EditBox_Create(#edit_width, 9*8+CANVASX+PAD+PAD, HEADER_TEXTY-3, 50, sizeof(edit_width_text), #edit_width_text, 0b);
			edit_box_draw stdcall (#edit_width);
			break;
		case TOOL_COLOR_DEPTH:
			WritePanelText(CANVASX+PAD, "Color depth");
			x += 11*8 + PAD;
			color_depth_btnid_1 = button.new_id;
			for (i=1; i<11; i++) {
				DeleteButton(color_depth_btnid_1+i);
				if (main_image.type == i) {
					a = true; //this is current image depth
				} else {
					a = false; //this is image depth we can set
					//probe does libimg support converting current image gepth to i-one
					img_create stdcall(1, 1, main_image.type);
					img_convert stdcall(EAX, 0, i, 0, 0);
					if (EAX) {
						img_destroy stdcall(EAX); 
					} else {
						a = -1;
						if (Form.width-CANVASX-64 < 652) {
							button.add(1);
							continue;							
						}
					}
				}
				x += draw_tool_btn(#event_set_color_depth, SCAN_CODE_ENTER, x, 7, -1, libimg_bpp[i], a) + PAD;			
			}
			break;
		case TOOL_FLIP_ROTATE:
			WritePanelText(CANVASX+PAD, "Flip");
			draw_tool_btn(#event_flip_hor, ECTRL + SCAN_CODE_KEY_H, CANVASX + PAD + 040, 7, 34, NULL, false);
			draw_tool_btn(#event_flip_ver, ECTRL + SCAN_CODE_KEY_V, CANVASX + PAD + 080, 7, 35, NULL, false);
			WritePanelText(CANVASX+PAD + 142, "Rotate");
			draw_tool_btn(#event_rotate_left,  ECTRL + SCAN_CODE_KEY_L, CANVASX + PAD + 200, 7, 37, NULL, false);
			draw_tool_btn(#event_rotate_right, ECTRL + SCAN_CODE_KEY_R, CANVASX + PAD + 240, 7, 36, NULL, false);
			// WritePanelText(CANVASX+PAD + 142, "Move");
			//draw_tool_btn(#event_move_left,  ECTRL + SCAN_CODE_LEFT,  PAD, tx.inc(GAP_B), 30, false);
			//draw_tool_btn(#event_move_right, ECTRL + SCAN_CODE_RIGHT, PAD, tx.inc(GAP_S), 31, false);
			//draw_tool_btn(#event_move_up,    ECTRL + SCAN_CODE_UP,    PAD, tx.inc(GAP_S), 32, false);
			//draw_tool_btn(#event_move_down,  ECTRL + SCAN_CODE_DOWN,  PAD, tx.inc(GAP_S), 33, false);
			break;
		default:
			if (!param) WritePanelText(CANVASX+PAD, "Welcome to ImageEditor Pro! Try to open a file.");
	}
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void event_save_png() { saving_type = SAVE_AS_PNG; draw_acive_panel(); }
void event_save_bmp() { saving_type = SAVE_AS_BMP; draw_acive_panel(); }
void event_save_raw() { saving_type = SAVE_AS_RAW; draw_acive_panel(); }
void event_save_pnm() { saving_type = SAVE_AS_PNM; draw_acive_panel(); }
void event_activate_export() { active_tool = TOOL_EXPORT; draw_content(); }
void event_activate_crop() { active_tool = TOOL_CROP; draw_content(); }
void event_activate_resize() { active_tool = TOOL_RESIZE; draw_content(); }
void event_activate_depth() { active_tool = TOOL_COLOR_DEPTH; draw_content(); }
void event_activate_flprot() { active_tool = TOOL_FLIP_ROTATE; draw_content(); }

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

void event_save()
{
	dword _image_format, _image_depth;
	strcpy(#filename_area, #param + strrchr(#param, '/'));
	if (EAX = strrchr(#filename_area, '.'))	filename_area[EAX-1] = '\0';
	switch (saving_type) {
		case SAVE_AS_PNG:
				strcat(#filename_area, "_IEP.png");
				_image_format = LIBIMG_FORMAT_PNG;
				_image_depth = IMAGE_BPP24;
				break;
		case SAVE_AS_BMP:
				strcat(#filename_area, "_IEP.bmp");
				_image_format = LIBIMG_FORMAT_BMP;
				_image_depth = IMAGE_BPP24;
				break;
		case SAVE_AS_PNM:
				strcat(#filename_area, "_IEP.pnm");
				_image_format = LIBIMG_FORMAT_PNM;
				_image_depth = IMAGE_BPP24;
				break;
		case SAVE_AS_RAW:
				notify("Not implemented yet.");
				return;
	}
	o_dialog.type = 1; //save file
	OpenDialog_start stdcall (#o_dialog);
	if (o_dialog.status) {
		set_file_path(#openfile_path);
		img_convert stdcall(main_image.image, 0, _image_depth, 0, 0);
		if (!EAX) {
			notify("'ImageEdit Pro\nCan not convert image into specified color depth' -Et");
		} else {
			$push eax //converted data
			img_encode stdcall(EAX, _image_format, 0); //<=EAX data, ECX size
			$push eax //encoded data
			CreateFile(ECX, EAX, #openfile_path);
			$pop ecx
			free(ECX);
			$pop eax
			img_destroy stdcall(EAX);
		}
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

