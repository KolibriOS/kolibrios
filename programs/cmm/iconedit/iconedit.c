/*
 * Icon Editor for KolibriOS
 * Authors: Leency, Nicolas
 * Licence: GPL v2
*/

#define MEMSIZE 1024*2000

#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/mem.h"
#include "../lib/cursor.h"
#include "../lib/list_box.h"
#include "../lib/events.h"

#include "../lib/obj/libimg.h"
#include "../lib/obj/box_lib.h"

#include "../lib/patterns/rgb.h"
#include "../lib/patterns/toolbar_button.h"

#include "colors_mas.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#ifdef LANG_RUS
char edit_menu_items[] = 
"Вырезать|Ctrl+X
Копировать|Ctrl+C
Вставить|Ctrl+V";
char image_menu_items[] = 
"Посчитать количество уникальных цветов
Заменить все цвета 1 на 2
Проверить иконку";
?define T_MENU_IMAGE "Иконка"
?define T_TEST_ICON "Проверка иконки"
?define T_TITLE "Редактор иконок 0.70a Beta"
?define T_UNIC_COLORS_COUNT "'Уникальных цветов: %i.' -I"
?define T_TOO_BIG_IMAGE_FOR_PREVIEW "'IconEdit
Изображение слишком большое для предпросмотра!' -tE"
?define T_ERROR_CROP_TOOL "'Для обрезки изображения вначале нужно выделить область.' -W"
?define T_ERROR_IMA_ICONEDIT "'Это просто редактор иконок, выбраное
изображение слишком велико для него!' -E"
#else
char edit_menu_items[] = 
"Cut|Ctrl+X
Copy|Ctrl+C
Paste|Ctrl+V";
char image_menu_items[] = 
"Count unic colors used
Replace all colors equal to 1 by 2
Test icon";
?define T_MENU_IMAGE "Icon"
?define T_TEST_ICON "Test Icon"
?define T_TITLE "Icon Editor 0.70 Beta"
?define T_UNIC_COLORS_COUNT "'Image has %i unique colors.' -I"
?define T_TOO_BIG_IMAGE_FOR_PREVIEW "'IconEdit
Image is too big for preview!' -tE"
?define T_ERROR_CROP_TOOL "'You need to select something before using crop tool.' -W"
?define T_ERROR_IMA_ICONEDIT "'Hey, this is just an icon editor,
selected image is too big to open!' -E"
#endif



#define PALLETE_SIZE 116

#define TOPBAR_H    24+8
int leftbar_w;

#define PAL_ITEMS_X_COUNT 13
#define COLSIZE 18
#define RIGHT_BAR_W PAL_ITEMS_X_COUNT*COLSIZE

#define TO_CANVAS_X(xval) xval - canvas.x/zoom.value
#define TO_CANVAS_Y(yval) yval - canvas.y/zoom.value

block canvas = { NULL, NULL, NULL, NULL };
block wrapper = { 0, TOPBAR_H, NULL, NULL };
block right_bar = { NULL, 10+TOPBAR_H, RIGHT_BAR_W+10, NULL };
block image_menu_btn = { NULL, 4, NULL, 22 };

dword linear_gradient[RIGHT_BAR_W];
block b_color_gradient = {NULL, 40+TOPBAR_H, RIGHT_BAR_W, 25};
//block b_opacity_gradient = {NULL, 75+TOPBAR_H, RIGHT_BAR_W, 15};
block b_last_colors = {NULL, 75+TOPBAR_H, RIGHT_BAR_W, COLSIZE};
block b_default_palette = {NULL, COLSIZE+10+75+TOPBAR_H, RIGHT_BAR_W, COLSIZE*9};

dword transparent = 0xBFCAD2;
dword color1 = 0x000000;
dword color2 = 0xBFCAD2;
dword tool_color;

signed hoverX;
signed hoverY;
signed priorHoverX;
signed priorHoverY;
bool canvasMouseMoved = false;

EVENTS button;
EVENTS key;

enum {
	BTNS_PALETTE_COLOR_MAS = 100,
	BTNS_LAST_USED_COLORS = 400
};

proc_info Form;
dword semi_white;
bool bg_dark=false;

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

#define LAST_USED_MAX 13
dword last_used_colors[LAST_USED_MAX] = {
0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,
0xFFFFFF,0xFFFFFF,0xFFFFFF
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

libimg_image icons16;

void main()
{
	word btn;
	libimg_image open_image;

	load_dll(libimg, #libimg_init, 1);
	load_dll(boxlib, #box_lib_init,0);

	icons16.load("/sys/icons16.png");
	leftbar_w = icons16.w + 16;

	sc.get();
	bg_dark = skin_is_dark();

	semi_white = MixColors(sc.work, 0xFFFfff, bg_dark*90 + 96);
	icons16.replace_color(0xffFFFfff, sc.work);
	icons16.replace_color(0xffCACBD6, MixColors(sc.work, 0, 200));

	//fix line and rectandle color for dark skins
	if (bg_dark) icons16.replace_color(0xff545454, 0xffD3D3D4);

	EventSetActiveColor(1, color1);

	if (!param[0]) {
		image.create(32, 32);
	}
	else
	{
		open_image.load(#param);
		open_image.convert_into(IMAGE_BPP24);

		if (open_image.w*open_image.h>MAX_CELL_SIZE*MAX_CELL_SIZE) {
			notify(T_ERROR_IMA_ICONEDIT);
			ExitProcess();
		}
		else {
			image.create(open_image.h, open_image.w);
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

			button.press(btn);
	  
		case evKey:
			GetKeys();

			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) key.press(ECTRL + key_scancode);

			if (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) {
				if (key_scancode == SCAN_CODE_DEL) EventCleanCanvas();
			}

			if (currentTool != TOOL_NONE) && (tools[currentTool].onKeyEvent != 0)
				tools[currentTool].onKeyEvent(key_scancode);

			key.press(key_scancode);

			if (key_scancode == SCAN_CODE_KEY_Z) actionsHistory.undoLastAction();
			if (key_scancode == SCAN_CODE_KEY_Y) actionsHistory.redoLastAction();

			if (key_scancode == SCAN_CODE_MINUS) {zoom.dec(); DrawEditArea();}
			if (key_scancode == SCAN_CODE_PLUS)  {zoom.inc(); DrawEditArea();}

			break;
		 
		case evReDraw:
			Window_CanvasReSize.thread_exists();
			if (CheckActiveProcess(Form.ID)) EventCheckMenuItemSelected();
			DrawWindow();
			break;
	}
}

void DrawTopPanelButton1(dword _event, _hotkey, _x, _icon_n)
{
	#define ISIZE 18
	#define YPOS 6
	DefineHiddenButton(_x-4, YPOS-4, ISIZE+7, ISIZE+7, button.add(_event));
	img_draw stdcall(icons16.image, _x, YPOS, ISIZE, ISIZE, 0, _icon_n*ISIZE);
	if (_hotkey) key.add_n(_hotkey, _event);
}


int DrawFlatPanelButton(dword _id, _x, _y, _text)
{
	#define P 10
	int w = strlen(_text)*6 + P + P;
	DrawBar(_x, _y, w, 22, semi_white);
	PutPixel(_x,_y,sc.work);
	PutPixel(_x,_y+21,sc.work);
	PutPixel(_x+w-1,_y,sc.work);
	PutPixel(_x+w-1,_y+21,sc.work);
	DefineHiddenButton(_x, _y, w, 21, _id);
	WriteText(_x+P, _y+7, 0x80, sc.work_text, _text);
	return w;
}

void DrawLeftPanelButton(dword _event, _hotkey, _y, _icon_n)
{
	int x = 5;
	DrawRectangle(x, _y, icons16.w + 5, icons16.w + 5, sc.work);
	DefineHiddenButton(x, _y, icons16.w + 5, icons16.w + 5, button.add(_event));
	img_draw stdcall(icons16.image, x+3, _y+3, icons16.w, 
		icons16.w, 0, _icon_n*icons16.w);
	key.add_n(_hotkey, _event);
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
		button.add(#EventCanvasResize),
		sc.button,
		sc.button_text,
		#param
		);
}


void DrawWindow()
{
	#define GAPH 27
	#define GAPV 28
	#define GAP_S 24+7
	#define GAP_B 24+20
	#define BLOCK_SPACE 10
	incn tx;
	incn ty;
	sc.get();
	DefineAndDrawWindow(115+random(100), 50+random(100), 700, 540, 0x73, NULL, T_TITLE, 0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window&ROLLED_UP) return;
	if (Form.width  < 560) { MoveSize(OLD,OLD,560,OLD); return; }
	if (Form.height < 430) { MoveSize(OLD,OLD,OLD,430); return; }
	button.init(40);
	key.init(40);

	right_bar.x = Form.cwidth - right_bar.w;
	b_color_gradient.x = b_last_colors.x = b_default_palette.x = right_bar.x;
	DrawBar(0, 0, Form.cwidth, TOPBAR_H-1, sc.work);
	DrawBar(0, TOPBAR_H-1, Form.cwidth, 1, sc.line);

	DrawTopPanelButton1(#EventCreateNewIcon,  ECTRL + SCAN_CODE_KEY_N, tx.set(7),    2);
	DrawTopPanelButton1(#EventOpenIcon,       ECTRL + SCAN_CODE_KEY_O, tx.inc(GAP_S), 0);
	DrawTopPanelButton1(#EventSaveIconToFile, ECTRL + SCAN_CODE_KEY_S, tx.inc(GAP_S), 5);
	DrawTopPanelButton1(#EventMoveLeft,       ECTRL + SCAN_CODE_LEFT,  tx.inc(GAP_B), 30);
	DrawTopPanelButton1(#EventMoveRight,      ECTRL + SCAN_CODE_RIGHT, tx.inc(GAP_S), 31);
	DrawTopPanelButton1(#EventMoveUp,         ECTRL + SCAN_CODE_UP,    tx.inc(GAP_S), 32);
	DrawTopPanelButton1(#EventMoveDown,       ECTRL + SCAN_CODE_DOWN,  tx.inc(GAP_S), 33);
	DrawTopPanelButton1(#EventFlipHor,        0, tx.inc(GAP_B), 34);
	DrawTopPanelButton1(#EventFlipVer,        0, tx.inc(GAP_S), 35);
	DrawTopPanelButton1(#EventRotateLeft,     ECTRL + SCAN_CODE_KEY_L, tx.inc(GAP_S), 37);
	DrawTopPanelButton1(#EventRotateRight,    ECTRL + SCAN_CODE_KEY_R, tx.inc(GAP_S), 36);
	DrawTopPanelButton1(#EventCrop,           0, tx.inc(GAP_B), 46);

	image_menu_btn.x = tx.inc(GAP_B);
	image_menu_btn.w = DrawFlatPanelButton(button.add(#EventShowImageMenu), image_menu_btn.x, image_menu_btn.y, T_MENU_IMAGE);
	//tx.inc(image_menu_btn.w + BLOCK_SPACE);
	
	DrawEditArea();

	DrawBar(0, TOPBAR_H, leftbar_w-1, Form.cheight - TOPBAR_H, sc.work);

	ty.n = right_bar.y - GAPV - 2;

	DrawLeftPanelButton(#EventSelectToolPencil, SCAN_CODE_KEY_P, ty.inc(GAPV), 38);
	DrawLeftPanelButton(#EventSelectToolPick,   SCAN_CODE_KEY_I, ty.inc(GAPV), 39);
	DrawLeftPanelButton(#EventSelectToolFill,   SCAN_CODE_KEY_F, ty.inc(GAPV), 40);
	DrawLeftPanelButton(#EventSelectToolLine,   SCAN_CODE_KEY_L, ty.inc(GAPV), 41);
	DrawLeftPanelButton(#EventSelectToolRect,   SCAN_CODE_KEY_R, ty.inc(GAPV), 42);
	DrawLeftPanelButton(#EventSelectToolBar,    SCAN_CODE_KEY_B, ty.inc(GAPV), 43);
	DrawLeftPanelButton(#EventSelectToolSelect, SCAN_CODE_KEY_S, ty.inc(GAPV), 44);
	DrawLeftPanelButton(#EventSelectToolScrCopy,SCAN_CODE_KEY_Q, ty.inc(GAPV), 45);
	DrawLeftPanelSelection();

	button.add_n(1, #EventExitIconEdit);
	key.add_n(ECTRL + SCAN_CODE_KEY_T, #EventTestIcon);

	DrawBar(wrapper.x+wrapper.w, TOPBAR_H, Form.cwidth-wrapper.x-wrapper.w,
		Form.cheight - TOPBAR_H, sc.work);
	DrawActiveColor(right_bar.y);
	DrawColorPallets();
	DrawPreview();

	DrawBar(leftbar_w-1, wrapper.y + wrapper.h, wrapper.w+1, 
		Form.cheight - wrapper.y - wrapper.h, sc.work);
	DrawStatusBar();
}

void DrawLeftPanelSelection()
{
	if (previousTool!=-1) DrawRectangle3D(5, previousTool*GAPV+right_bar.y-2, icons16.w+5, icons16.w+5, sc.work, sc.work);
	DrawRectangle3D(5, currentTool*GAPV+right_bar.y-2, icons16.w+5, icons16.w+5, 0x333333, 0x777777);
}

void DrawEditArea()
{
	dword color1=0xC0C0C0;
	int top_side;
	int left_side;

	wrapper.x = icons16.w + 16;
	wrapper.w = Form.cwidth - right_bar.w - 10 - wrapper.x;
	wrapper.h = Form.cheight - TOPBAR_H - 35;

	//canvas{
	canvas.w = image.columns * zoom.value;
	canvas.h = image.rows * zoom.value;
	if (canvas.w+2 > wrapper.w) || (canvas.h+2 > wrapper.h) { 
		zoom.value--;
		if (zoom.x) zoom.redraw();
		DrawEditArea();
		return;
	}
	canvas.x = -zoom.value*image.columns+wrapper.w/2 + wrapper.x;
	canvas.y = -zoom.value*image.rows+wrapper.h/2 + wrapper.y;
	DrawCanvas();
	//}

	left_side = canvas.x-wrapper.x-1;
	top_side = canvas.y-wrapper.y-1;

	DrawRectangle(wrapper.x-1, wrapper.y-1, wrapper.w, wrapper.h, sc.line);

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

	DrawFrame(right_bar.x+CELL+5, outy, CELL, CELL, NULL);
	DrawBar(right_bar.x+CELL+5+2, outy+2, CELL-4, CELL-4, color2);

	//sprintf(#param, "%A", color1);
	//WriteTextWithBg(right_bar.x+30, outy+3, 0xD0, sc.work_text, #param+4, sc.work);
	DrawCurrentColorGradient();
}

int lmax;
void GenerateCurrentColorGradient()
{
	int i, avg, rmax;

	rgb.DwordToRgb(color1);
	avg = 255 - calc(rgb.r + rgb.g + rgb.b / 3);

	lmax = b_color_gradient.w *avg/255 | 1;
	rmax = b_color_gradient.w - lmax | 1;
	if (lmax == 0) lmax=1;
	if (rmax == 0) rmax=1;
	
	for (i=0; i<lmax; i++) {
		linear_gradient[i] = MixColors(color1,0xFFFfff,255*i/lmax);
	}

	for (i=0 ; i<=rmax; i++) {
		linear_gradient[lmax+rmax - i] = MixColors(color1,0x000000,255*i/rmax);
	}
}

int DrawGradientMarker(dword marker_x, marker_color)
{
	if (marker_x > b_color_gradient.w - 1) marker_x = b_color_gradient.w - 1;
	DrawBar(b_color_gradient.x + marker_x-2, b_color_gradient.y-3, 5, 1, marker_color);
	DrawBar(b_color_gradient.x + marker_x-1, b_color_gradient.y-2, 3, 1, marker_color);
	PutPixel(b_color_gradient.x + marker_x, b_color_gradient.y-1, marker_color);
	return marker_x;
}

int old_marker_pos;
void DrawCurrentColorGradient()
{
	int i;
	for (i=0 ; i<b_color_gradient.w; i++) {
		DrawBar(b_color_gradient.x+i, b_color_gradient.y, 1, b_color_gradient.h, linear_gradient[i]);		
	}
	DrawGradientMarker(old_marker_pos, sc.work);
	old_marker_pos = DrawGradientMarker(lmax, 0xFFFfff * bg_dark);
}

void DrawColorPallets()
{
	int r, c, i=0;
	//Last used colors
	for (r = 0; r < LAST_USED_MAX/PAL_ITEMS_X_COUNT; r++)
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
	int y = b_default_palette.y + b_default_palette.h + 6;
	int preview_h = Form.cheight - y;

	if (image.columns > right_bar.w) return;
	if (image.rows > preview_h) return;

	PutImage(right_bar.w - image.columns / 2 + x - 3,
		preview_h - image.rows / 2 + y, 
		image.columns, image.rows, image.get_image()
		);
}

dword GetPixelUnderMouse()
{
	return GetPixelColorFromScreen(mouse.x + Form.left + 5, mouse.y + Form.top + skin_h);
}

int preview_size = 128;
void DrawImageWithBg(dword _x, _y, _col_to)
{
	_x *= preview_size;
	_y *= preview_size;
	DrawWideRectangle(_x,_y, preview_size, preview_size, preview_size-image.columns/2, _col_to);
	PutImage(preview_size - image.columns / 2 + _x, preview_size - image.rows / 2 + _y,
		image.columns, image.rows, image.get_image_with_replaced_color(color2, _col_to));
}

void ShowWindow_TestIcon()
{
	if (image.rows>=preview_size) || (image.columns>=preview_size) {
		notify(T_TOO_BIG_IMAGE_FOR_PREVIEW);
		return;
	}
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
				preview_size*2+skin_h+4, 0x74, NULL, T_TEST_ICON, 0);
			DrawImageWithBg(0, 0, 0x000000);
			DrawImageWithBg(1, 0, 0xFFFfff);
			DrawImageWithBg(0, 1, GetPixelColorFromScreen(0, 0));
			DrawImageWithBg(1, 1, sc.work);
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

void EventOpenIcon()
{
	RunProgram("/sys/lod", sprintf(#param, "*png* %s",#program_path));
}

#ifdef LANG_RUS
#define TEXT_FILE_SAVED_AS "'Файл сохранен как %s' -O"
#else
#define TEXT_FILE_SAVED_AS "'File saved as %s' -O"
#endif
void EventSaveIconToFile()
{
	int i=0;
	char save_file_name[4096];
	char save_path_stable[4096];
	char save_success_message[4096+200];
	strcpy(#save_path_stable, "/tmp0/1");
	do {
		i++;
		sprintf(#save_file_name, "%s/icon_%i.png", #save_path_stable, i);
	} while (file_exists(#save_file_name));
	save_image(image.get_image(), image.columns, image.rows, #save_file_name);

	sprintf(#save_success_message, TEXT_FILE_SAVED_AS, #save_file_name);
	notify(#save_success_message);
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
	if (last_used_colors[0] == _color) return;
	for (i=LAST_USED_MAX-1; i>0; i--) {
		last_used_colors[i] = last_used_colors[i-1];
	}
	last_used_colors[0] = _color;

	if (_number == 1) color1 = _color;
	if (_number == 2) color2 = _color;

	if (b_color_gradient.hovered()) {
		lmax = mouse.x - b_color_gradient.x;
	}
	else {
		GenerateCurrentColorGradient();
	}
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
		notify(T_ERROR_CROP_TOOL);
	}
}

void EventShowImageMenu()
{
	open_lmenu(image_menu_btn.x, image_menu_btn.y + image_menu_btn.h, 
		MENU_TOP_LEFT, NULL, #image_menu_items);
}

void EventShowEditMenu()
{
	open_lmenu(image_menu_btn.x, image_menu_btn.y + image_menu_btn.h, 
		MENU_TOP_LEFT, NULL, #edit_menu_items);
}

void EventCheckMenuItemSelected()
{
	switch(get_menu_click()) {
		case 1: 
			EventCountColorsUsed();
			break;
		case 2: 
			EventReplaceImageColors(color1, color2);
			break;
		case 3: 
			EventTestIcon();
			break;
	}
}

void EventCountColorsUsed()
{
	char res_str[64];
	int cur, prev;
	int max = image.rows*image.columns;
	int resi=0;
	bool unic;
	for (cur=0; cur<max; cur++) {
		unic = true;
		for (prev=0; prev<cur; prev++) {
			if (image.mas[prev] == image.mas[cur]) {unic=false; break;}
		}
		if (unic) resi++;
	}
	notify( sprintf(#res_str, T_UNIC_COLORS_COUNT, resi) );
}

void EventReplaceImageColors(dword c1, c2)
{
	int max = image.rows*image.columns;
	int cur;
	for (cur=0; cur<max; cur++) {
		if (image.mas[cur] == color1) image.mas[cur] = color2;
	}
}

void EventCanvasResize()
{
	notify("Sorry, not implemented yet.");
}

void EventMoveLeft() { EventMove(MOVE_LEFT); }
void EventMoveRight() { EventMove(MOVE_RIGHT); }
void EventMoveUp() { EventMove(MOVE_UP); }
void EventMoveDown() { EventMove(MOVE_DOWN); }
void EventFlipHor() { EventMove(FLIP_HOR); }
void EventFlipVer() { EventMove(FLIP_VER); }
void EventRotateLeft() { EventMove(ROTATE_LEFT); }
void EventRotateRight() { EventMove(ROTATE_RIGHT); }

void EventSelectToolPencil() { setCurrentTool(TOOL_PENCIL); }
void EventSelectToolPick() { setCurrentTool(TOOL_PIPETTE); }
void EventSelectToolFill() { setCurrentTool(TOOL_FILL); }
void EventSelectToolLine() { setCurrentTool(TOOL_LINE); }
void EventSelectToolRect() { setCurrentTool(TOOL_RECT); }
void EventSelectToolBar() { setCurrentTool(TOOL_BAR); }
void EventSelectToolSelect() { setCurrentTool(TOOL_SELECT); }
void EventSelectToolScrCopy() { setCurrentTool(TOOL_SCREEN_COPY);  }

char test_icon_stak22[4096];

stop:

char test_icon_stak[4096];
