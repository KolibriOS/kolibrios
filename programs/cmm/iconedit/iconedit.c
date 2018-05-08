/*
 * Icon Editor for KolibriOS
 * Authors: Leency, Nicolas
 * Licence: GPL v2
*/

#define MEMSIZE 4096*500

#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/mem.h"
#include "../lib/cursor.h"

#include "../lib/obj/libimg.h"
#include "../lib/obj/box_lib.h"

#include "../lib/patterns/rgb.h"
#include "../lib/patterns/libimg_load_skin.h"

#include "colors_mas.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define T_TITLE "Icon Editor 0.56 Alpha"

#define TOPBAR_H    24+8
#define LEFTBAR_W 16+5+5+3+3
#define PALLETE_SIZE 116

#define PAL_ITEMS_X_COUNT 13
#define COLSIZE 18
#define RIGHT_BAR_W PAL_ITEMS_X_COUNT*COLSIZE

#define TO_CANVAS_X(xval) xval - canvas.x/zoom.value
#define TO_CANVAS_Y(yval) yval - canvas.y/zoom.value

block canvas = { NULL, NULL, NULL, NULL };
block wrapper = { LEFTBAR_W, TOPBAR_H, NULL, NULL };
block right_bar = { NULL, 10+TOPBAR_H, RIGHT_BAR_W+10, NULL };

block b_color_gradient = {NULL, 40+TOPBAR_H, RIGHT_BAR_W, 30};
block b_last_colors = {NULL, 80+TOPBAR_H, RIGHT_BAR_W, COLSIZE*2};
block b_default_palette = {NULL, COLSIZE*2+10+80+TOPBAR_H, RIGHT_BAR_W, COLSIZE*9};

dword color1 = 0x000000;
dword color2 = 0xBFCAD2;
dword tool_color;

signed hoverX;
signed hoverY;
signed priorHoverX;
signed priorHoverY;
bool canvasMouseMoved = false;

enum {
	BTN_NEW = 40,
	BTN_OPEN,
	BTN_SAVE,
	BTN_MOVE_LEFT,
	BTN_MOVE_RIGHT,
	BTN_MOVE_UP,
	BTN_MOVE_DOWN,
	BTN_FLIP_HOR,
	BTN_FLIP_VER,
	BTN_ROTATE_LEFT,
	BTN_ROTATE_RIGHT,
	BTN_TEST_ICON,
	BTN_PENCIL,
	BTN_PICK,
	BTN_FILL,
	BTN_LINE,
	BTN_RECT,
	BTN_BAR,
	BTN_SELECT,
	BTN_SCREEN_COPY,
	BTN_ZOOM_IN,
	BTN_ZOOM_OUT,
	BTN_CANVAS_RESIZE,
	BTN_CROP,
	BTNS_PALETTE_COLOR_MAS = 100,
	BTNS_LAST_USED_COLORS = 400
};

proc_info Form;
dword semi_white;

more_less_box zoom = { 11, 1, 40, "Zoom" };

dword default_palette[] = {
0x330000,0x331900,0x333300,0x193300,0x003300,0x003319,0x003333,0x001933,0x000033,0x190033,
0x330033,0x330019,0x000000,0x660000,0x663300,0x666600,0x336600,0x006600,0x006633,0x006666,
0x003366,0x000066,0x330066,0x660066,0x660033,0x202020,0x990000,0x994C00,0x999900,0x4C9900,
0x009900,0x00994C,0x009999,0x004C99,0x000099,0x4C0099,0x990099,0x99004C,0x404040,0xCC0000,
0xCC6600,0xCCCC00,0x66CC00,0x00CC00,0x00CC66,0x00CCCC,0x0066CC,0x0000CC,0x6600CC,0xCC00CC,
0xCC0066,0x606060,0xFF0000,0xFF8000,0xFFFF00,0x80FF00,0x00FF00,0x00FF80,0x00FFFF,0x0080FF,
0x0000FF,0x7F00FF,0xFF00FF,0xFF007F,0x808080,0xFF3333,0xFF9933,0xFFFF33,0x99FF33,0x33FF33,
0x33FF99,0x33FFFF,0x3399FF,0x3333FF,0x9933FF,0xFF33FF,0xFF3399,0xA0A0A0,0xFF6666,0xFFB266,
0xFFFF66,0xB2FF66,0x66FF66,0x66FFB2,0x66FFFF,0x66B2FF,0x6666FF,0xB266FF,0xFF66FF,0xFF66B2,
0xC0C0C0,0xFF9999,0xFFCC99,0xFFFF99,0xCCFF99,0x99FF99,0x99FFCC,0x99FFFF,0x99CCFF,0x9999FF,
0xCC99FF,0xFF99FF,0xFF99CC,0xE0E0E0,0xFFCCCC,0xFFE5CC,0xFFFFCC,0xE5FFCC,0xCCFFCC,0xCCFFE5,
0xCCFFFF,0xCCE5FF,0xCCCCFF,0xE5CCFF,0xFFCCFF,0xFFCCE5,0xFFFFFF	
};
dword last_used_colors[13*2] = {
0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,
0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,
0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF
};

CustomCursor Cursor;
dword CursorBar = FROM "cursors/bar.cur";
dword CursorFill = FROM "cursors/fill.cur";
dword CursorLine = FROM "cursors/line.cur";
dword CursorPencil = FROM "cursors/pencil.cur";
dword CursorPipette = FROM "cursors/pipette.cur";
dword CursorRectangle = FROM "cursors/rectangle.cur";
dword CursorSelect = FROM "cursors/select.cur";

_image image;

#include "actions_history.h"

_ActionsHistory actionsHistory;

#include "tools.h"
#include "canvas_resize.h"

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

libimg_image top_icons;
libimg_image left_icons;

void main()
{
	word btn;
	libimg_image open_image;

	load_dll(libio,  #libio_init,  1);
	load_dll(libimg, #libimg_init, 1);
	load_dll(boxlib, #box_lib_init,0);

	Libimg_LoadImage(#top_icons, "/sys/icons16.png");
	Libimg_LoadImage(#left_icons, "/sys/icons16.png");

	system.color.get();
	semi_white = MixColors(system.color.work, 0xFFFfff, 96);
	Libimg_ReplaceColor(top_icons.image, top_icons.w, top_icons.h, 0xffFFFfff, semi_white);
	Libimg_ReplaceColor(top_icons.image, top_icons.w, top_icons.h, 0xffCACBD6, MixColors(semi_white, 0, 220));

	Libimg_ReplaceColor(left_icons.image, left_icons.w, left_icons.h, 0xffFFFfff, system.color.work);
	Libimg_ReplaceColor(left_icons.image, left_icons.w, left_icons.h, 0xffCACBD6, MixColors(system.color.work, 0, 200));

	if (!param[0]) {
		image.create(32, 32);
	}
	else
	{
		Libimg_LoadImage(#open_image, #param);

		if (open_image.w*open_image.h>MAX_CELL_SIZE*MAX_CELL_SIZE) {
			notify("'Hey, this is just an icon editor,\nselected image is too big to open!' -E");
			ExitProcess();
		}
		else {
			image.create(open_image.w, open_image.h);
			image.set_image(open_image.imgsrc);
		}
	}

	actionsHistory.init();

	initTools();
	setCurrentTool(TOOL_PENCIL);
	
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);

	loop() switch(WaitEvent())
	{
		case evMouse:
			if (Window_CanvasReSize.thread_exists()) break;
			mouse.get();
			
			if (mouse.lkm) tool_color = color1;
			if (mouse.pkm) tool_color = color2;
			if (mouse.mkm) break;

			hoverX = mouse.x - canvas.x / zoom.value;
			hoverY = mouse.y - canvas.y / zoom.value;
			if (hoverX<0) hoverX = 0;
			if (hoverY<0) hoverY = 0;
			if (hoverX>image.columns-1) hoverX = image.columns-1;
			if (hoverY>image.rows-1) hoverY = image.rows-1;
			canvasMouseMoved = false;
			if (priorHoverX != hoverX) canvasMouseMoved = true;
			if (priorHoverY != hoverY) canvasMouseMoved = true;
			priorHoverX = hoverX;
			priorHoverY = hoverY;
			//DrawBar(Form.cwidth-100, 3, 80, 12, 0xFFFfff);
			//WriteText(Form.cwidth-100, 3, 0x80, 0x000000, 
			//	sprintf(#param, "%i %i", hoverX, hoverY));

			if (currentTool != TOOL_NONE)
				tools[currentTool].onMouseEvent(mouse.x, mouse.y, mouse.lkm, mouse.pkm);

			if (mouse.vert) {
				if (mouse.vert==65535) zoom.inc();
				if (mouse.vert==1) zoom.dec();
				DrawEditArea();
			}

			if (wrapper.hovered()) SetCursor();
			else Cursor.Restore();

			if (mouse.down) {
				if (b_color_gradient.hovered()) 
				|| (b_last_colors.hovered())
				|| (b_default_palette.hovered()) {
					if (mouse.key&MOUSE_LEFT) EventSetActiveColor(1, GetPixelUnderMouse());
					if (mouse.key&MOUSE_RIGHT) EventSetActiveColor(2, GetPixelUnderMouse());
				}	
			}

			break;

		case evButton:
			if (Window_CanvasReSize.thread_exists()) break;
			btn = GetButtonID();

			if (zoom.click(btn)) DrawEditArea();

			switch(btn)
			{
				case BTN_NEW:
					EventCreateNewIcon();
					break;
				case BTN_OPEN:
					RunProgram("/sys/lod", sprintf(#param, "*png* %s",#program_path));
					break;
				case BTN_SAVE:
					EventSaveIconToFile();
					break;
				case BTN_MOVE_LEFT:
					EventMove(MOVE_LEFT);
					break;
				case BTN_MOVE_RIGHT:
					EventMove(MOVE_RIGHT);
					break;
				case BTN_MOVE_UP:
					EventMove(MOVE_UP);
					break;
				case BTN_MOVE_DOWN:
					EventMove(MOVE_DOWN);
					break;
				case BTN_FLIP_VER:
					EventMove(FLIP_VER);
					break;
				case BTN_FLIP_HOR:
					EventMove(FLIP_HOR);
					break;
				case BTN_TEST_ICON:
					EventTestIcon();
					break;
				case BTN_PENCIL:
					setCurrentTool(TOOL_PENCIL);
					break;
				case BTN_PICK:
					setCurrentTool(TOOL_PIPETTE);
					break;
				case BTN_FILL:
					setCurrentTool(TOOL_FILL);
					break;
				case BTN_LINE:
					setCurrentTool(TOOL_LINE);
					break;
				case BTN_RECT:
					setCurrentTool(TOOL_RECT);
					break;
				case BTN_BAR:
					setCurrentTool(TOOL_BAR);
					break;
				case BTN_SELECT:
					setCurrentTool(TOOL_SELECT);
					break;
				case BTN_SCREEN_COPY:
					setCurrentTool(TOOL_SCREEN_COPY);
					break;
				case BTN_CANVAS_RESIZE:
					notify("Sorry, not implemented yet.");
					break;
				case BTN_CROP:
					EventCrop();
					break;
				case CLOSE_BTN:
					EventExitIconEdit();
					break;
			}
			break;
	  
		case evKey:
			GetKeys();

			if (currentTool != TOOL_NONE) && (tools[currentTool].onKeyEvent != 0)
				tools[currentTool].onKeyEvent(key_scancode);

			if (key_scancode == SCAN_CODE_DEL) EventCleanCanvas();

			if (key_scancode == SCAN_CODE_KEY_P) setCurrentTool(TOOL_PENCIL);
			if (key_scancode == SCAN_CODE_KEY_I) setCurrentTool(TOOL_PIPETTE);
			if (key_scancode == SCAN_CODE_KEY_F) setCurrentTool(TOOL_FILL);
			if (key_scancode == SCAN_CODE_KEY_L) setCurrentTool(TOOL_LINE);
			if (key_scancode == SCAN_CODE_KEY_R) setCurrentTool(TOOL_RECT);
			if (key_scancode == SCAN_CODE_KEY_B) setCurrentTool(TOOL_BAR);
			if (key_scancode == SCAN_CODE_KEY_S) setCurrentTool(TOOL_SELECT);

			if (key_scancode == SCAN_CODE_KEY_T) EventTestIcon();

			if (key_scancode == SCAN_CODE_KEY_Z) actionsHistory.undoLastAction();
			if (key_scancode == SCAN_CODE_KEY_Y) actionsHistory.redoLastAction();

			if (key_scancode == SCAN_CODE_MINUS) {zoom.dec(); DrawEditArea();}
			if (key_scancode == SCAN_CODE_PLUS)  {zoom.inc(); DrawEditArea();}

			break;
		 
		case evReDraw:
			Window_CanvasReSize.thread_exists();
			DrawWindow();
			break;
	}
}

void DrawTopPanelButton(dword _id, _x, _icon_n)
{
	DrawWideRectangle(_x, 4, 22, 22, 3, semi_white);
	PutPixel(_x,4,system.color.work);
	PutPixel(_x,4+21,system.color.work);
	PutPixel(_x+21,4,system.color.work);
	PutPixel(_x+21,4+21,system.color.work);
	DefineHiddenButton(_x, 4, 21, 21, _id);
	img_draw stdcall(top_icons.image, _x+3, 7, 16, 16, 0, _icon_n*16);
}

void DrawLeftPanelButton(dword _id, _y, _icon_n)
{
	int x = 5;
	DrawRectangle(x, _y, 22-1, 22-1, system.color.work);
	DefineHiddenButton(x, _y, 21, 21, _id);
	img_draw stdcall(left_icons.image, x+3, _y+3, 16, 16, 0, _icon_n*16);
}

void DrawStatusBar()
{
	zoom.draw(wrapper.x, wrapper.y + wrapper.h + 6);

	sprintf(#param,"%i x %i", image.columns, image.rows);
	DrawCaptButton(
		wrapper.x+wrapper.w-calc(strlen(#param)*8) -6 - 1,
		zoom.y,
		calc(strlen(#param)*8)+6,
		18,
		BTN_CANVAS_RESIZE,
		system.color.work_button,
		system.color.work_button_text,
		#param
		);
}

void DrawWindow()
{
	#define GAP 27
	#define BLOCK_SPACE 10
	incn tx;
	system.color.get();
	DefineAndDrawWindow(115+random(100), 50+random(100), 700, 540, 0x73, NULL, T_TITLE, 0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	if (Form.width  < 560) { MoveSize(OLD,OLD,560,OLD); return; }
	if (Form.height < 430) { MoveSize(OLD,OLD,OLD,430); return; }

	right_bar.x = Form.cwidth - right_bar.w;
	b_color_gradient.x = b_last_colors.x = b_default_palette.x = right_bar.x;
	DrawBar(0, 0, Form.cwidth, TOPBAR_H-1, system.color.work);
	DrawBar(0, TOPBAR_H-1, Form.cwidth, 1, system.color.work_graph);

	tx.n = 5-GAP;
	DrawTopPanelButton(BTN_NEW,    tx.inc(GAP), 2); //not implemented
	DrawTopPanelButton(BTN_OPEN,   tx.inc(GAP), 0); //not implemented
	DrawTopPanelButton(BTN_SAVE,   tx.inc(GAP), 5);
	DrawTopPanelButton(BTN_MOVE_LEFT,  tx.inc(GAP+BLOCK_SPACE), 30);
	DrawTopPanelButton(BTN_MOVE_RIGHT, tx.inc(GAP),   31);
	DrawTopPanelButton(BTN_MOVE_UP,    tx.inc(GAP),   32);
	DrawTopPanelButton(BTN_MOVE_DOWN,  tx.inc(GAP),   33);
	
	DrawTopPanelButton(BTN_FLIP_HOR,   tx.inc(GAP+BLOCK_SPACE), 34);
	DrawTopPanelButton(BTN_FLIP_VER,   tx.inc(GAP),   35);

	DrawTopPanelButton(BTN_TEST_ICON,  tx.inc(GAP+BLOCK_SPACE), 12);

	DrawTopPanelButton(BTN_CROP,  tx.inc(GAP+BLOCK_SPACE), 46);
	// DrawTopPanelButton(BTN_ROTATE_LEFT,   tx.inc(GAP), 36); //not implemented
	// DrawTopPanelButton(BTN_ROTATE_RIGHT,  tx.inc(GAP), 37); //not implemented
	
	DrawEditArea();

	DrawBar(0, TOPBAR_H, LEFTBAR_W-1, Form.cheight - TOPBAR_H, system.color.work);
	DrawLeftPanel();

	DrawBar(wrapper.x+wrapper.w, TOPBAR_H, Form.cwidth-wrapper.x-wrapper.w,
		Form.cheight - TOPBAR_H, system.color.work);
	DrawActiveColor(right_bar.y);
	DrawColorPallets();
	DrawPreview();

	DrawBar(LEFTBAR_W-1, wrapper.y + wrapper.h, wrapper.w+1, 
		Form.cheight - wrapper.y - wrapper.h, system.color.work);
	DrawStatusBar();
}

void DrawLeftPanel()
{
	#define GAP 28
	incn ty;
	ty.n = right_bar.y - GAP - 2;
	DrawLeftPanelButton(BTN_PENCIL, ty.inc(GAP), 38);
	DrawLeftPanelButton(BTN_PICK,   ty.inc(GAP), 39);
	DrawLeftPanelButton(BTN_FILL,   ty.inc(GAP), 40);
	DrawLeftPanelButton(BTN_LINE,   ty.inc(GAP), 41);
	DrawLeftPanelButton(BTN_RECT,   ty.inc(GAP), 42);
	DrawLeftPanelButton(BTN_BAR,    ty.inc(GAP), 43);
	DrawLeftPanelButton(BTN_SELECT, ty.inc(GAP), 44);
	DrawLeftPanelButton(BTN_SCREEN_COPY, ty.inc(GAP), 45);
	DrawRectangle3D(5, currentTool*GAP+right_bar.y-2, 16+3+2, 16+3+2, 0x333333, 0x777777);
}

void DrawEditArea()
{
	dword color1=0xC0C0C0;
	int top_side;
	int left_side;

	wrapper.w = Form.cwidth - right_bar.w - 10 - wrapper.x;
	wrapper.h = Form.cheight - TOPBAR_H - 35;

	//canvas{
	canvas.w = image.columns * zoom.value;
	canvas.h = image.rows * zoom.value;
	if (canvas.w+2 > wrapper.w) || (canvas.h+2 > wrapper.h) { 
		zoom.value--;
		DrawEditArea();
		return;
	}
	canvas.x = -zoom.value*image.columns+wrapper.w/2 + wrapper.x;
	canvas.y = -zoom.value*image.rows+wrapper.h/2 + wrapper.y;
	DrawCanvas();
	//}

	left_side = canvas.x-wrapper.x-1;
	top_side = canvas.y-wrapper.y-1;

	DrawRectangle(wrapper.x-1, wrapper.y-1, wrapper.w, wrapper.h, system.color.work_graph);

	if (left_side>0)
	{
		DrawBar(wrapper.x, wrapper.y, wrapper.w-1, top_side, color1); //top
		DrawBar(wrapper.x, wrapper.y+wrapper.h-top_side-1, wrapper.w-1, top_side, color1); //bottom
	}
	if (top_side>0)
	{
		//left
		DrawBar(wrapper.x, wrapper.y+top_side, left_side, 
			wrapper.h-top_side-top_side, color1); 
		//right
		DrawBar(wrapper.x+wrapper.w-left_side-1, wrapper.y+top_side, left_side, 
			wrapper.h-top_side-top_side, color1);
	}
	DrawRectangle(canvas.x-1, canvas.y-1, canvas.w+1, canvas.h+1, 0x808080);
}

void DrawActiveColor(dword iny)
{
	#define CELL 20
	static dword outy;
	if (iny != NULL) outy = iny;
	DrawFrame(right_bar.x, outy, CELL, CELL, NULL);
	DrawBar(right_bar.x+2, outy+2, CELL-4, CELL-4, color1);
	sprintf(#param, "%A", color1);
	EDI = system.color.work;
	WriteText(right_bar.x + 30, outy + 3, 0xD0, system.color.work_text, #param+4);

	DrawFrame(right_bar.x+110, outy, CELL, CELL, NULL);
	DrawBar(right_bar.x+110+2, outy+2, CELL-4, CELL-4, color2);
	sprintf(#param, "%A", color2);
	EDI = system.color.work;
	WriteText(right_bar.x+110 + 30, outy + 3, 0xD0, system.color.work_text, #param+4);	
	DrawCurrentColorGradientByLightness();
}

void DrawCurrentColorGradientByLightness()
{
	int i;
	int w = right_bar.w-10/2;
	for (i=0; i<w; i++)
		DrawBar(b_color_gradient.x+i, b_color_gradient.y, 
			1, b_color_gradient.h, MixColors(color1,0xFFFfff,255*i/w));

	//current color marker	
	DrawBar(b_color_gradient.x+i-1, b_color_gradient.y-2, 3,2, 0x000000);

	for (i=0 ; i<=w; i++)
		DrawBar(b_color_gradient.x+w+w-i, b_color_gradient.y, 
			1, b_color_gradient.h, MixColors(color1,0x000000,255*i/w));
}

void DrawColorPallets()
{
	int r, c, i=0;
	//Last used colors
	for (r = 0; r < 2; r++)
	{
		for (c = 0; c < PAL_ITEMS_X_COUNT; c++, i++)
		{
			DrawBar(c*COLSIZE + b_last_colors.x, r*COLSIZE + b_last_colors.y, 
				COLSIZE, COLSIZE, last_used_colors[i]);
		}
	}
	i=0;
	//Default colors
	for (r = 0; r < 9; r++)
	{
		for (c = 0; c < PAL_ITEMS_X_COUNT; c++, i++)
		{
			DrawBar(c*COLSIZE + b_default_palette.x, r*COLSIZE + b_default_palette.y, 
				COLSIZE, COLSIZE, default_palette[PALLETE_SIZE-i]);
		}
	}
}

void DrawCanvasPixel(dword _r,_c,_color)
{
	DrawBar(_c*zoom.value + canvas.x, _r*zoom.value + canvas.y, 
	zoom.value, zoom.value, _color);
}

void DrawCanvas()
{
	int r, c;
	dword color;

	if ((currentTool != TOOL_NONE) && (tools[currentTool].onCanvasDraw != 0))
	{
		tools[currentTool].onCanvasDraw();
	}

	for (r = 0; r < image.rows; r++)
	{
		for (c = 0; c < image.columns; c++)
		{
			if (image.pixel_state.is_drawable(r,c)) 
				DrawCanvasPixel(r, c, image.get_pixel(r,c));
		}
	}
	image.pixel_state.reset_and_set_all_drawable();

	DrawPreview();
}

void DrawPreview()
{
	int x = right_bar.x;
	int y = wrapper.y + wrapper.h - image.rows-2;
	DrawRectangle(x, y, image.columns+1, image.rows+1, system.color.work_graph);
	_PutImage(x+1,y+1, image.columns, image.rows, image.get_image());
}

dword GetPixelUnderMouse()
{
	return GetPixelColorFromScreen(mouse.x + Form.left + 5, mouse.y + Form.top + skin_height);
}

int preview_size = 128;
void DrawImageWithBg(dword _x, _y, _col_to)
{
	_x *= preview_size;
	_y *= preview_size;
	DrawWideRectangle(_x,_y, preview_size, preview_size, preview_size-image.columns/2, _col_to);
	_PutImage(preview_size - image.columns / 2 + _x, preview_size - image.rows / 2 + _y,
		image.columns, image.rows, image.get_image_with_replaced_color(color2, _col_to));
}

void ShowWindow_TestIcon()
{
	loop() switch(WaitEvent())
	{
		case evButton:
			if (GetButtonID()) ExitProcess();
			break;
	  
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) ExitProcess();
			break;
		 
		case evReDraw:
			DefineAndDrawWindow(Form.left+100, Form.top+100, preview_size*2+9,
				preview_size*2+skin_height+4, 0x74, NULL, "Test Icon", 0);
			DrawImageWithBg(0, 0, 0x000000);
			DrawImageWithBg(1, 0, 0xFFFfff);
			DrawImageWithBg(0, 1, GetPixelColorFromScreen(0, 0));
			DrawImageWithBg(1, 1, system.color.work);
			break;
	}
}

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void EventCreateNewIcon()
{
	EventSaveIconToFile();
	Window_CanvasReSize.create();
}

void EventSaveIconToFile()
{
	int i=0;
	char save_file_name[4096];
	char save_path_stable[4096];
	strcpy(#save_path_stable, "/tmp0/1");
	do {
		i++;
		sprintf(#save_file_name, "%s/saved_icon_%i.png", #save_path_stable, i);
	} while (file_exists(#save_file_name));
	save_image(image.get_image(), image.columns, image.rows, #save_file_name);
}

void EventCleanCanvas()
{
	image.create(image.rows, image.columns);
	actionsHistory.saveCurrentState();
	DrawCanvas();
}

void EventExitIconEdit()
{
	EventSaveIconToFile();
	ExitProcess();
}

void EventSetActiveColor(int _number, _color)
{
	int i;
	for (i=13*2-1; i>0; i--) {
		last_used_colors[i] = last_used_colors[i-1];
	}
	last_used_colors[0] = _color;

	if (_number == 1) color1 = _color;
	if (_number == 2) color2 = _color;

	DrawActiveColor(NULL);
	DrawColorPallets();
}

void EventTestIcon()
{
	CreateThread(#ShowWindow_TestIcon, #test_icon_stak+4092);
}

void EventMove(dword _action)
{
	if (selection.state) {
		selection.buf.move(_action);
		SelectTool_onCanvasDraw();
	}
	else {
		image.move(_action);
		DrawCanvas();
	}
	actionsHistory.saveCurrentState();
}

void EventCrop()
{
	if (selection.state) {
		EventSaveIconToFile();
		image.create(selection.buf.rows, selection.buf.columns);
		selection.move_to_point(0,0);
		selection.apply_to_image();
		selection.reset();
		actionsHistory.init();
		DrawWindow();
	}
	else {
		notify("'You need to select something before usnig crop tool.' -W");
	}
}

stop:

char test_icon_stak[4096];
