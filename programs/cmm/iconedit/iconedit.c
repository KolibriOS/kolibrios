/*
 * Icon Editor for KolibriOS
 * Authors: Leency, Nicolas
 * Licence: GPL v2
*/

/*
TODO:
window colors
enhance icon
pipet aside color view
*/

#define MEMSIZE 4096*40

#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/mem.h"
#include "../lib/obj/libimg.h"
#include "../lib/patterns/rgb.h"
#include "../lib/patterns/libimg_load_skin.h"

#include "colors_mas.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define T_TITLE "Icon Editor 0.45"

#define TOOLBAR_H    24+8
#define PANEL_LEFT_W 16+5+5+3+3
#define PALLETE_SIZE 116
#define TB_ICON_PADDING 26

#define PAL_ITEMS_X_COUNT 13
#define COLSIZE 18
#define RIGHT_BAR_W PAL_ITEMS_X_COUNT*COLSIZE

#define TO_CANVAS_X(xval) xval - canvas.x/zoom.value
#define TO_CANVAS_Y(yval) yval - canvas.y/zoom.value

block canvas = { NULL, NULL, NULL, NULL };
block wrapper = { PANEL_LEFT_W, TOOLBAR_H, NULL, NULL };
block right_bar = { NULL, TOOLBAR_H, RIGHT_BAR_W+10, NULL };

block b_color_gradient = {NULL, 30+TOOLBAR_H, RIGHT_BAR_W, 30};
block b_last_colors = {NULL, 70+TOOLBAR_H, RIGHT_BAR_W, COLSIZE*2};
block b_default_palette = {NULL, COLSIZE*2+10+70+TOOLBAR_H, RIGHT_BAR_W, COLSIZE*9};

dword color1 = 0x000000;
dword color2 = 0xFFFfff;
dword tool_color;

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
	BTN_PENCIL,
	BTN_PICK,
	BTN_FILL,
	BTN_LINE,
	BTN_RECT,
	BTN_SELECT,
	BTN_ZOOM_IN,
	BTN_ZOOM_OUT,
	BTNS_PALETTE_COLOR_MAS = 100,
	BTNS_LAST_USED_COLORS = 400
};

proc_info Form;

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

_image image;

#include "actions_history.h"

libimg_image open_image;
_ActionsHistory actionsHistory;

enum {
	TOOL_NONE = -1,
	TOOL_PENCIL,
	TOOL_PIPETTE,
	TOOL_FILL,
	TOOL_LINE,
	TOOL_RECT,
	TOOL_SELECT
};

struct Tool {
	int id;
	
	void (*activate)();
	void (*deactivate)();
	void (*onMouseEvent)(int x, int y, int lkm, int pkm);
	void (*onKeyEvent)(dword keycode);
	void (*onCanvasDraw)();
};

Tool tools[6];
int currentTool = -1;

void resetCurrentTool() {
	if ((currentTool != TOOL_NONE) && (tools[currentTool].deactivate != 0)) {
		tools[currentTool].deactivate();
	}
	
	currentTool = TOOL_NONE;
}

void setCurrentTool(int index) {
	resetCurrentTool();

	currentTool = index;
	
	if ((index != TOOL_NONE) && (tools[index].activate != 0))
		tools[index].activate();

	DrawLeftPanel();
	DrawCanvas();
}

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void FillTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	if (canvas.hovered()) && (currentTool==TOOL_FILL) && (mouse.up)
	{
		EventFill(mouseY-canvas.y/zoom.value, 
				mouseX-canvas.x/zoom.value, tool_color);
		actionsHistory.saveCurrentState();			
		DrawCanvas();
	}
}

void PipetteTool_activate() {
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE);
}

void PipetteTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	tool_color = GetPixelUnderMouse();
	DrawBar(Form.cwidth-30, 5, 20, 20, tool_color);
	
	if (mouse.down) {
		SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
		if (mouse.key&MOUSE_LEFT) EventSetActiveColor(1, tool_color);
		if (mouse.key&MOUSE_RIGHT) EventSetActiveColor(2, tool_color);
		
		setCurrentTool(TOOL_PENCIL);
	}
}

bool PencilTool_Drawing = false;

void PencilTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	if (canvas.hovered()) 
	{
		if ((PencilTool_Drawing == true) && (!mouse.key)) {
			actionsHistory.saveCurrentState();
			PencilTool_Drawing = false;
		}

		if (mouse.key) {
			image.set_pixel(mouseY-canvas.y/zoom.value, 
				mouseX-canvas.x/zoom.value, tool_color);
			PencilTool_Drawing = true;
		}
		DrawCanvas();
	}
}

void PencilTool_reset() {
	PencilTool_Drawing = false;
}

// Line tool
struct SimpleFigureTool_State {
	int startX, startY;
	int lastTempPosX, lastTempPosY;
};

enum {
	TOOL_LINE_STATE,
	TOOL_RECT_STATE
};

dword currentFigToolState = -1;
SimpleFigureTool_State figTool_States[2];

void SimpleFigureTool_Reset() {
	if (currentTool == TOOL_LINE)
		currentFigToolState = TOOL_LINE_STATE;
	else if (currentTool == TOOL_RECT)
		currentFigToolState = TOOL_RECT_STATE;

	figTool_States[currentFigToolState].startX = -1;
	figTool_States[currentFigToolState].startY = -1;
	figTool_States[currentFigToolState].lastTempPosX = -1;
	figTool_States[currentFigToolState].lastTempPosY = -1;
}

int mouseX_last;
int mouseY_last;
bool first_click_in_canvas = false;

void SimpleFigureTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	if (mouse.down) && (canvas.hovered()) first_click_in_canvas = true;
	if (first_click_in_canvas)
	{
		if (mouseX>canvas.x+canvas.w-zoom.value) mouseX = canvas.x+canvas.w-zoom.value;
		if (mouseY>canvas.y+canvas.h-zoom.value) mouseY = canvas.y+canvas.h-zoom.value;
		if (mouseX<canvas.x) mouseX = canvas.x;
		if (mouseY<canvas.y) mouseY = canvas.y;

		if (mouse.key) {
			if ((figTool_States[currentFigToolState].startX < 0) || (figTool_States[currentFigToolState].startY < 0)) {
				figTool_States[currentFigToolState].startX = mouseX;
				figTool_States[currentFigToolState].startY = mouseY;
			}
			else {
				if ((calc(mouseX - canvas.x/zoom.value) != figTool_States[currentFigToolState].lastTempPosX)
					|| (calc(mouseY - canvas.y/zoom.value) != figTool_States[currentFigToolState].lastTempPosY)) 
				{
					DrawCanvas();
				}
			}
			mouseX_last = mouseX;
			mouseY_last = mouseY;
		}
		if (mouse.up) {
			if ((figTool_States[currentFigToolState].startX >= 0) && (figTool_States[currentFigToolState].startY >= 0)) {
				// Draw line from start position to current position
				if (currentTool == TOOL_LINE) {
					DrawLine(figTool_States[currentFigToolState].startX - canvas.x/zoom.value, 
						figTool_States[currentFigToolState].startY - canvas.y/zoom.value, 
						mouseX - canvas.x/zoom.value, 
						mouseY - canvas.y/zoom.value, 
						tool_color, 
						1);
				}
				else if (currentTool == TOOL_RECT) {
					DrawRectangleInCanvas(figTool_States[currentFigToolState].startX - canvas.x/zoom.value, 
						figTool_States[currentFigToolState].startY - canvas.y/zoom.value, 
						mouseX - canvas.x/zoom.value, 
						mouseY - canvas.y/zoom.value, tool_color, 1);
				}

				DrawCanvas();

				actionsHistory.saveCurrentState();

				// Reset start position
				figTool_States[currentFigToolState].startX = -1;
				figTool_States[currentFigToolState].startY = -1;

				first_click_in_canvas = false;
			}
		}
	}
}

void SimpleFigureTool_onCanvasDraw() {
	if ((figTool_States[currentFigToolState].startX >= 0) && (figTool_States[currentFigToolState].startY >= 0) && (mouse.key)) {
		if (currentTool == TOOL_LINE) {
			DrawLine(figTool_States[currentFigToolState].startX - canvas.x/zoom.value, 
				figTool_States[currentFigToolState].startY - canvas.y/zoom.value, 
				mouseX_last - canvas.x/zoom.value, 
				mouseY_last - canvas.y/zoom.value, 
				tool_color, 
				2);
		}
		else if (currentTool == TOOL_RECT) {
			DrawRectangleInCanvas(figTool_States[currentFigToolState].startX - canvas.x/zoom.value, 
				figTool_States[currentFigToolState].startY - canvas.y/zoom.value, 
				mouseX_last - canvas.x/zoom.value, 
				mouseY_last - canvas.y/zoom.value, 
				tool_color, 
				2);
		}

		figTool_States[currentFigToolState].lastTempPosX = mouseX_last - canvas.x/zoom.value;
		figTool_States[currentFigToolState].lastTempPosY = mouseY_last - canvas.y/zoom.value;
	}
}

// Selection
int selection_start_x = -1;
int selection_start_y = -1;
int selection_end_x = -1;
int selection_end_y = -1;
bool selection_active = false;

dword SelectionTool_buffer = 0;
dword SelectionTool_buffer_r = 0;
dword SelectionTool_buffer_c = 0;

bool selection_moving_started = false;
int selection_pivot_x = -1;
int selection_pivot_y = -1;

void SelectTool_normalizeSelection() {
	int t;

	// Restructuring of the selection coordinates
	if (selection_end_x < selection_start_x) {
		t = selection_start_x;
		selection_start_x = selection_end_x;
		selection_end_x = t;
	} 

	if (selection_end_y < selection_start_y) {
		t = selection_end_y;
		selection_end_y = selection_start_y;
		selection_start_y = t;
	}
}

void reset_selection_moving() {
	if (selection_moving_started) {
		SelectTool_drawBuffer(selection_start_x, selection_start_y, 1);

		selection_pivot_x = -1;
		selection_pivot_y = -1;
		
		selection_moving_started = false;
		
		actionsHistory.saveCurrentState();
		DrawCanvas();
	}
}

bool is_selection_moving() {
	return selection_moving_started;
}

void reset_selection() {
	reset_selection_moving();
	
	selection_start_x = -1;
	selection_start_y = -1;
	selection_end_x = -1;
	selection_end_y = -1;	
}

void SelectTool_activate() {
	reset_selection();

	selection_active = false;
}

void SelectTool_deactivate() {
	reset_selection_moving();
}

bool SelectTool_pointInSelection(int x, int y) {
	if (x >= selection_start_x) && (x <= selection_end_x) && (y >= selection_start_y) && (y <= selection_end_y)
		return true;
	else 
		return false;
}


void SelectTool_copyToBuffer() {
	dword offset, r, c;

		if (SelectionTool_buffer != 0)
			free(SelectionTool_buffer);

		SelectionTool_buffer_r = selection_end_y - selection_start_y + 1;
		SelectionTool_buffer_c = selection_end_x - selection_start_x + 1;
		SelectionTool_buffer = malloc(SelectionTool_buffer_r * SelectionTool_buffer_c * 4);

		for (r = selection_start_y; r <= selection_end_y; r++) {
			for (c = selection_start_x; c <= selection_end_x; c++) {
				offset = calc(SelectionTool_buffer_c * calc(r - selection_start_y) + calc(c - selection_start_x)) * 4;

				ESDWORD[SelectionTool_buffer + offset] = image.get_pixel(r, c);
			}
		}
}

void SelectTool_onMouseEvent(int mouseX, int mouseY, int lkm, int pkm) {
	int click_x, click_y, dx, dy, m_x, m_y, r, c, color;
	dword pixel;
	
	m_x = TO_CANVAS_X(mouseX);
	m_y = TO_CANVAS_Y(mouseY);
	
	if (mouse.down) && (canvas.hovered()) && (!selection_active) {
		if (selection_start_x != -1) && (SelectTool_pointInSelection(m_x, m_y)) {
			if (selection_pivot_x == -1) {
				selection_pivot_x = m_x;
				selection_pivot_y = m_y;
				
				GetKeys();
				
				if (!selection_moving_started) && ( !(key_modifier&KEY_LSHIFT) ) {
					for (r = selection_start_y; r <= selection_end_y; r++)
						for (c = selection_start_x; c <= selection_end_x; c++) {
							image.set_pixel(r, c, color2);
						}
				}
				
				selection_moving_started = true;
			}
		}
		else {
		
			reset_selection();
			selection_active = true;
		}
	}

	if (selection_pivot_x != -1) {
		dx = m_x - selection_pivot_x;
		dy = m_y - selection_pivot_y;

		if (selection_start_x + dx < 0)
			dx = selection_start_x;
		
		if (selection_end_x + dx >= 32)
			dx = 31 - selection_end_x;
		
		if (selection_start_y + dy < 0)
			dy = selection_start_y;
		
		if (selection_end_y + dy >= 32)
			dy = 31 - selection_end_y;
		
		
		selection_start_x += dx;
		selection_end_x += dx;
		
		selection_start_y += dy;
		selection_end_y += dy;
		
		selection_pivot_x += dx;
		selection_pivot_y += dy;
		
		DrawCanvas();
	}
	
	if (selection_active)
	{
		if (mouseX>canvas.x+canvas.w-zoom.value) mouseX = canvas.x+canvas.w-zoom.value;
		if (mouseY>canvas.y+canvas.h-zoom.value) mouseY = canvas.y+canvas.h-zoom.value;

		if (mouseX<canvas.x) mouseX = canvas.x;
		if (mouseY<canvas.y) mouseY = canvas.y;

		if (mouse.key) {
			selection_end_x = TO_CANVAS_X(mouseX);
			selection_end_y = TO_CANVAS_Y(mouseY);

			if ((selection_start_x < 0) || (selection_start_y < 0)) {
				selection_start_x = TO_CANVAS_X(mouseX);
				selection_start_y = TO_CANVAS_Y(mouseY);
			}
			else {
				DrawCanvas();

				/**if ((calc(TO_CANVAS_X(mouseX)) != selection_end_x)
					|| (calc(TO_CANVAS_Y(mouseY)) != selection_end_y)) 
				{
					DrawCanvas();
				}*/
			}

		}
		
		if (mouse.up) {			
			selection_active = false;
			
			SelectTool_normalizeSelection();
			SelectTool_copyToBuffer();
		}
	}
	
	if (mouse.up) {
		if (selection_pivot_x != -1) {
			selection_pivot_x = -1;
			selection_pivot_y = -1;
		}
	}
}

void SelectTool_onCanvasDraw() {	
	if (selection_moving_started)
		SelectTool_drawBuffer(selection_start_x, selection_start_y, 2);

	if ((selection_start_x >= 0) && (selection_start_y >= 0) && (selection_end_x >= 0) && (selection_end_y >= 0)) {
		DrawSelection(selection_start_x, selection_start_y, selection_end_x, selection_end_y);
	}	
}

void SelectTool_drawBuffer(int insert_x, int insert_y, int target) {
	dword color;
	dword offset, r, c;
	dword insert_to_x, insert_to_y;
	
	if (SelectionTool_buffer != 0) {
		insert_to_x = insert_x + SelectionTool_buffer_c - 1;
			
		if (insert_to_x >= image.columns)
			insert_to_x = image.columns-1;

		insert_to_y = insert_y + SelectionTool_buffer_r - 1;
			
		if (insert_to_y >= image.rows)
			insert_to_y = image.rows-1;

		for (r = insert_y; r <= insert_to_y; r++) {
			for (c = insert_x; c <= insert_to_x; c++) {
					offset = calc(SelectionTool_buffer_c * calc(r - insert_y) + calc(c - insert_x)) * 4;

					color = ESDWORD[SelectionTool_buffer + offset];
					
					if (target == 1)
						image.set_pixel(r, c, color);
					else
						DrawBar(c*zoom.value + canvas.x, r*zoom.value + canvas.y, 
							zoom.value, zoom.value, color);
			}
		}	
	}	
}

void SelectTool_onKeyEvent(dword keycode) {
	dword offset, r, c;
	dword insert_x, insert_y, insert_to_x, insert_to_y;

	if (keycode == SCAN_CODE_KEY_V) {
		if (SelectionTool_buffer != 0) {
			reset_selection();
			
			selection_moving_started = true;
			selection_start_x = 0;
			selection_end_x = SelectionTool_buffer_c - 1;
			
			selection_start_y = 0;
			selection_end_y = SelectionTool_buffer_r - 1;
			
			DrawCanvas();
	
		}
	}
}

void initTools() 
{
	tools[0].id = TOOL_PENCIL;
	tools[0].onMouseEvent = #PencilTool_onMouseEvent;
	tools[0].deactivate = #PencilTool_reset;
	
	tools[1].id = TOOL_PIPETTE;
	tools[1].activate = #PipetteTool_activate;
	tools[1].onMouseEvent = #PipetteTool_onMouseEvent;
	
	tools[2].id = TOOL_FILL;
	tools[2].onMouseEvent = #FillTool_onMouseEvent;
	
	tools[3].id = TOOL_LINE;
	tools[3].activate = #SimpleFigureTool_Reset;
	tools[3].deactivate = #SimpleFigureTool_Reset;
	tools[3].onMouseEvent = #SimpleFigureTool_onMouseEvent;
	tools[3].onCanvasDraw = #SimpleFigureTool_onCanvasDraw;
	
	tools[4].id = TOOL_RECT;
	tools[4].activate = #SimpleFigureTool_Reset;
	tools[4].deactivate = #SimpleFigureTool_Reset;
	tools[4].onMouseEvent = #SimpleFigureTool_onMouseEvent;
	tools[4].onCanvasDraw = #SimpleFigureTool_onCanvasDraw;	

	tools[5].id = TOOL_SELECT;
	tools[5].activate = #SelectTool_activate;
	tools[5].deactivate = #SelectTool_deactivate;
	tools[5].onMouseEvent = #SelectTool_onMouseEvent;
	tools[5].onCanvasDraw = #SelectTool_onCanvasDraw;	
	tools[5].onKeyEvent = #SelectTool_onKeyEvent;	
}

void main()
{
	word btn;

	load_dll(libio,  #libio_init,  1);
	load_dll(libimg, #libimg_init, 1);
	Libimg_LoadImage(#skin, "/sys/icons16.png");
	//system.color.get();
	//Libimg_ReplaceColor(tools_img.image, tools_img.w, tools_img.h, 0xFFF8C0D0, system.color.work);
	
	image.create(32, 32);

	if (param[0]) {
		Libimg_LoadImage(#open_image, #param);
		if (open_image.w==32) && (open_image.h==32) {
			image.set_image(open_image.imgsrc);
		}
		else {
			notify("'Error: image format is unacceptable (PNG, 32x32x16b expected)' -E");
		}
	}

	actionsHistory.init();

	initTools();
	setCurrentTool(TOOL_PENCIL);
	
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);

	loop() switch(WaitEvent())
	{
		case evMouse:
			mouse.get();
			
			if (mouse.lkm) tool_color = color1;
			if (mouse.pkm) tool_color = color2;
			if (mouse.mkm) break;

			if (currentTool != TOOL_NONE)
				tools[currentTool].onMouseEvent(mouse.x, mouse.y, mouse.lkm, mouse.pkm);

			if (mouse.vert) {
				if (mouse.vert==65535) zoom.inc();
				if (mouse.vert==1) zoom.dec();
				DrawEditArea();
			}

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
			btn = GetButtonID();

			switch(btn)
			{
				case BTN_NEW:
					image.create(32, 32);
					DrawCanvas();
					break;
				case BTN_OPEN:
					RunProgram("/sys/lod", sprintf(#param, "*png* %s",#program_path));
					break;
				case BTN_SAVE:
					EventSave();
					break;
				case BTN_MOVE_LEFT:
					image.move(MOVE_LEFT);
					DrawCanvas();
					break;
				case BTN_MOVE_RIGHT:
					image.move(MOVE_RIGHT);
					DrawCanvas();
					break;
				case BTN_MOVE_UP:
					image.move(MOVE_UP);
					DrawCanvas();
					break;
				case BTN_MOVE_DOWN:
					image.move(MOVE_DOWN);
					DrawCanvas();
					break;
				case BTN_FLIP_VER:
					image.move(FLIP_VER);
					DrawCanvas();
					break;
				case BTN_FLIP_HOR:
					image.move(FLIP_HOR);
					DrawCanvas();
					break;
				case BTN_PENCIL:
					setCurrentTool(TOOL_PENCIL);
					break;
				case BTN_PICK:
					setCurrentTool(TOOL_PIPETTE);
					//EventPickActivate();
					break;
				case BTN_FILL:
					setCurrentTool(TOOL_FILL);
					//EventFillActivate();
					break;
				case BTN_LINE:
					setCurrentTool(TOOL_LINE);
					break;
				case BTN_RECT:
					setCurrentTool(TOOL_RECT);
					break;
				case BTN_SELECT:
					setCurrentTool(TOOL_SELECT);
					break;
				case BTN_ZOOM_IN:
					zoom.inc();
					DrawEditArea();
					break;
				case BTN_ZOOM_OUT:
					zoom.dec();
					DrawEditArea();
					break;
				case CLOSE_BTN:
					ExitProcess();
					break;
			}
			break;
	  
		case evKey:
			GetKeys();

			if (currentTool != TOOL_NONE) && (tools[currentTool].onKeyEvent != 0)
				tools[currentTool].onKeyEvent(key_scancode);

			if (key_scancode == SCAN_CODE_ESC) setCurrentTool(TOOL_PENCIL);
			if (key_scancode == SCAN_CODE_KEY_P) setCurrentTool(TOOL_PENCIL);
			if (key_scancode == SCAN_CODE_KEY_I) setCurrentTool(TOOL_PIPETTE);
			if (key_scancode == SCAN_CODE_KEY_F) setCurrentTool(TOOL_FILL);
			if (key_scancode == SCAN_CODE_KEY_L) setCurrentTool(TOOL_LINE);
			if (key_scancode == SCAN_CODE_KEY_R) setCurrentTool(TOOL_RECT);

			if (key_scancode == SCAN_CODE_KEY_Z) && (key_modifier&KEY_LCTRL) actionsHistory.undoLastAction();
			if (key_scancode == SCAN_CODE_KEY_Y) && (key_modifier&KEY_LCTRL) actionsHistory.redoLastAction();

			if (key_scancode == SCAN_CODE_MINUS) {zoom.dec(); DrawEditArea();}
			if (key_scancode == SCAN_CODE_PLUS)  {zoom.inc();  DrawEditArea();}

			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}

void DrawToolbarButton(dword _id, _x, _icon_n)
{
	DrawWideRectangle(_x, 4, 22, 22, 3, 0xFFFfff);
	DefineHiddenButton(_x, 4, 21, 21, _id);
	img_draw stdcall(skin.image, _x+3, 7, 16, 16, 0, _icon_n*16);
}

void DrawLeftPanelButton(dword _id, _y, _icon_n)
{
	int x = 5;
	DrawWideRectangle(x, _y, 22, 22, 3, 0xFFFfff);
	DefineHiddenButton(x, _y, 21, 21, _id);
	img_draw stdcall(skin.image, x+3, _y+3, 16, 16, 0, _icon_n*16);
}

void DrawStatusBar()
{
	zoom.draw(wrapper.x, wrapper.y + wrapper.h + 6);

	sprintf(#param,"Canvas: %ix%i", image.rows, image.columns);
	WriteText(wrapper.x+wrapper.w-calc(strlen(#param)*8), zoom.y+2, 0x90, system.color.work_text, #param);
}

void draw_window()
{
	incn tx;
	system.color.get();
	DefineAndDrawWindow(115+random(100), 50+random(100), 700, 540, 0x33, system.color.work, T_TITLE, 0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	if (Form.width  < 560) { MoveSize(OLD,OLD,560,OLD); return; }
	if (Form.height < 430) { MoveSize(OLD,OLD,OLD,430); return; }

	right_bar.x = Form.cwidth - right_bar.w;
	b_color_gradient.x = b_last_colors.x = b_default_palette.x = right_bar.x;

	tx.n = 10-TB_ICON_PADDING;
	DrawToolbarButton(BTN_NEW,    tx.inc(TB_ICON_PADDING), 2); //not implemented
	DrawToolbarButton(BTN_OPEN,   tx.inc(TB_ICON_PADDING), 0); //not implemented
	DrawToolbarButton(BTN_SAVE,   tx.inc(TB_ICON_PADDING), 5);
	DrawToolbarButton(BTN_MOVE_LEFT,  tx.inc(TB_ICON_PADDING+8), 30);
	DrawToolbarButton(BTN_MOVE_RIGHT, tx.inc(TB_ICON_PADDING),   31);
	DrawToolbarButton(BTN_MOVE_UP,    tx.inc(TB_ICON_PADDING),   32);
	DrawToolbarButton(BTN_MOVE_DOWN,  tx.inc(TB_ICON_PADDING),   33);
	
	DrawToolbarButton(BTN_FLIP_HOR,   tx.inc(TB_ICON_PADDING+8), 34);
	DrawToolbarButton(BTN_FLIP_VER,   tx.inc(TB_ICON_PADDING),   35);
	// DrawToolbarButton(BTN_ROTATE_LEFT,   tx.inc(TB_ICON_PADDING), 36); //not implemented
	// DrawToolbarButton(BTN_ROTATE_RIGHT,  tx.inc(TB_ICON_PADDING), 37); //not implemented

	DrawLeftPanel();
	
	DrawEditArea();

	DrawActiveColor(right_bar.y);
	DrawColorPallets();

	DrawStatusBar();
}

void DrawLeftPanel()
{
	incn ty;
	ty.n = TOOLBAR_H-TB_ICON_PADDING;
	DrawLeftPanelButton(BTN_PENCIL, ty.inc(TB_ICON_PADDING), 38);
	DrawLeftPanelButton(BTN_PICK,   ty.inc(TB_ICON_PADDING), 39);
	DrawLeftPanelButton(BTN_FILL,   ty.inc(TB_ICON_PADDING), 40);
	DrawLeftPanelButton(BTN_LINE,   ty.inc(TB_ICON_PADDING), 41);
	DrawLeftPanelButton(BTN_RECT,   ty.inc(TB_ICON_PADDING), 42);
	DrawLeftPanelButton(BTN_SELECT,   ty.inc(TB_ICON_PADDING), 43);
	DrawRectangle3D(5, currentTool*TB_ICON_PADDING+TOOLBAR_H, 16+3+2, 16+3+2, 0x333333, 0x777777);
}

void DrawEditArea()
{
	dword color1=0xC0C0C0;
	int top_side;
	int left_side;

	wrapper.w = Form.cwidth - right_bar.w - 10 - wrapper.x;
	wrapper.h = Form.cheight - TOOLBAR_H - 35;

	//canvas{
	canvas.w = image.columns * zoom.value;
	canvas.h = image.rows * zoom.value;
	if (canvas.w+2 > wrapper.w) || (canvas.h+2 > wrapper.h) { 
		zoom.dec();
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
	static dword outy;
	if (iny != NULL) outy = iny;
	DrawBar(right_bar.x, outy, 20, 20, color1);
	sprintf(#param, "%A", color1);
	EDI = system.color.work;
	WriteText(right_bar.x + 30, outy + 3, 0xD0, system.color.work_text, #param+4);

	DrawBar(right_bar.x+110, outy, 20, 20, color2);
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

void DrawCanvas()
{
	int r, c;
	for (r = 0; r < image.rows; r++)
	{
		for (c = 0; c < image.columns; c++)
		{
			DrawBar(c*zoom.value + canvas.x, r*zoom.value + canvas.y, 
				zoom.value, zoom.value, image.get_pixel(r, c));
		}
	}
	
	if ((currentTool != TOOL_NONE) && (tools[currentTool].onCanvasDraw != 0))
		tools[currentTool].onCanvasDraw();

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

//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void EventSave()
{
	dword encoded_data=0;
	dword encoded_size=0;
	dword image_ptr = 0;
	
	image_ptr = create_image(Image_bpp24, 32, 32);

	if (image_ptr == 0) {
		notify("'Error saving file, probably not enought memory!' -E");
	}
	else {
		EDI = image_ptr;
		memmov(EDI._Image.Data, image.get_image(), image.rows * image.columns * 3);

		encoded_data = encode_image(image_ptr, LIBIMG_FORMAT_PNG, 0, #encoded_size);

		img_destroy stdcall(image_ptr);

		if(encoded_data == 0) {
			notify("'Error saving file, incorrect data!' -E");
		}
		else {
			if (CreateFile(encoded_size, encoded_data, "/rd/1/saved_image.png") == 0) {
				notify("'File saved as /rd/1/saved_image.png' -O");
			}
			else {
				notify("'Error saving file, probably not enought space on ramdisk!' -E");
			}
		}
	}
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

void EventFill(dword _r, _c, _color)
{
	#define MARKED 6
	int r, c, i, restart;

	dword old_color = image.get_pixel(_r, _c);
	image.set_pixel(_r, _c, MARKED);

	do {
		restart=false;	
		for (r = 0; r < image.rows; r++)
			for (c = 0; c < image.columns; c++)
			{
				IF (image.get_pixel(r,c) != old_color) continue;
				IF (image.get_pixel(r,c) == MARKED) continue;
				
				IF (c>0)               && (image.get_pixel(r,c-1) == MARKED) image.set_pixel(r,c,MARKED);
				IF (r>0)               && (image.get_pixel(r-1,c) == MARKED) image.set_pixel(r,c,MARKED);
				IF (c<image.columns-1) && (image.get_pixel(r,c+1) == MARKED) image.set_pixel(r,c,MARKED);
				IF (r<image.rows-1)    && (image.get_pixel(r+1,c) == MARKED) image.set_pixel(r,c,MARKED);
				
				IF (image.get_pixel(r,c)==MARKED) restart=true;
			}
	}while(restart);

	for (i=0; i<image.columns*image.rows; i++) 
			IF (image.mas[i]==MARKED) image.mas[i] = _color;
}

// target - image (1) or canvas (2)
void DrawLine(int x1, int y1, int x2, int y2, dword color, int target) {
	int dx, dy, signX, signY, error, error2;

   dx = x2 - x1;
   
   if (dx < 0)
	   dx = -dx;
   
   dy = y2 - y1;

   if (dy < 0)
	   dy = -dy;
   
   if (x1 < x2)
	   signX = 1;
   else
	   signX = -1;
   
   if (y1 < y2)
	   signY = 1;
   else
	   signY = -1;
   
   error = dx - dy;

	if (target == 1)
		image.set_pixel(y2, x2, color);
	else
		DrawBar(x2*zoom.value + canvas.x, y2*zoom.value + canvas.y, 
				zoom.value, zoom.value, color);
   
   while((x1 != x2) || (y1 != y2)) 
  {
		if (target == 1)
			image.set_pixel(y1, x1, color);
		else
			DrawBar(x1*zoom.value + canvas.x, y1*zoom.value + canvas.y, 
				zoom.value, zoom.value, color);
		
	   error2 = error * 2;

       if(error2 > calc(-dy)) 
       {
           error -= dy;
           x1 += signX;
       }
	   
       if(error2 < dx) 
       {
           error += dx;
           y1 += signY;
       }
   }

}

void DrawRectangleInCanvas(int x1, int y1, int x2, int y2, dword color, int target) {
	DrawLine(x1, y1, x2, y1, color, target);
	DrawLine(x2, y1, x2, y2, color, target);
	DrawLine(x2, y2, x1, y2, color, target);
	DrawLine(x1, y2, x1, y1, color, target);
}

#define SELECTION_COLOR 0xAAE5EF

void DrawSelection(int x1, int y1, int x2, int y2) {
	int p1x, p1y, p2x, p2y, r, c, old_color, new_color;
	dword offset;

	if (x1 <= x2) {
		p1x = x1;
		p2x = x2;
	}
	else {
		p1x = x2;
		p2x = x1;
	}

	if (y1 <= y2) {
		p2y = y1;
		p1y = y2;
	}
	else {
		p2y = y2;
		p1y = y1;
	}

	for (r = p1y; r >= p2y; r--) {
		for (c = p1x; c <= p2x; c++) {
			
			if (selection_moving_started) && (SelectTool_pointInSelection(c, r)) {
				offset = calc(SelectionTool_buffer_c * calc(r - selection_start_y) + calc(c - selection_start_x)) * 4;
				old_color = ESDWORD[SelectionTool_buffer + offset];
			}
			else {
				old_color = image.get_pixel(r, c);
			}
			
			new_color = MixColors(old_color, SELECTION_COLOR, 64);
			
			DrawBar(c*zoom.value + canvas.x, r*zoom.value + canvas.y, 
				zoom.value, zoom.value, new_color);

		}
	}
}