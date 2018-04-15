#define MEMSIZE 4096*10

#include "../lib/gui.h"
#include "../lib/clipboard.h"
#include "../lib/patterns/rgb.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

proc_info Form;
dword pick_active = true;
dword picked_color = 0;
char picked_color_string[11];
#define FORM_W 167
#define FORM_H 60
#define COLOR_BLOCK_SIZE 42
#define COLOR_BLOCK_PADDING FORM_H - COLOR_BLOCK_SIZE / 2
#define COLOR_BLOCK_X FORM_W - COLOR_BLOCK_SIZE - COLOR_BLOCK_PADDING
#define BUTTON_ID_CLOSE 1
#define BUTTON_ID_COPY  2
#define BUTTON_ID_PICK  3

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	int btn;
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
	SetWindowLayerBehaviour(-1, ZPOS_ALWAYS_TOP);
	loop() switch(WaitEvent())
	{
		case evMouse:
			mouse.get();
			
			if (mouse.x>0) && (mouse.x<FORM_W) && (mouse.y>0) && (mouse.y<FORM_H) 
			{
				EventDragWindow();
			}
			else if (pick_active)
			{
				picked_color = GetPixelColorFromScreen(mouse.x + Form.left, mouse.y + Form.top);
				EventUpdateWindowContent();
				if (mouse.down) && (mouse.key&MOUSE_LEFT) pick_active = false;
			}
			break;

		case evButton:
			btn = GetButtonID();
			if (btn == BUTTON_ID_CLOSE) ExitProcess();
			if (btn == BUTTON_ID_COPY) EventCopyHex();
			if (btn == BUTTON_ID_PICK) pick_active = true;
			break;

		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) ExitProcess();
			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) 
				if (key_scancode==SCAN_CODE_KEY_C) EventCopyHex();
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}

void draw_window()
{
	DefineDragableWindow(215, 100, FORM_W, FORM_H);
	GetProcessInfo(#Form, SelfInfo);
	DrawRectangle3D(0, 0, FORM_W, FORM_H, 0xCCCccc, 0x888888);
	DrawRectangle3D(1, 1, FORM_W-2, FORM_H-2, 0xCCCccc, 0x888888);
	DrawBar(2,2,FORM_W-3,FORM_H-3,0xFFFfff);

	DrawRectangle(COLOR_BLOCK_X-2, COLOR_BLOCK_PADDING-2, COLOR_BLOCK_SIZE+3, COLOR_BLOCK_SIZE+3, 0xCBC6C5);

	DefineHiddenButton(0, 0, COLOR_BLOCK_X, skin_height, 476+BT_NOFRAME);
	DefineHiddenButton(COLOR_BLOCK_X-1, COLOR_BLOCK_PADDING-1, COLOR_BLOCK_SIZE+1, COLOR_BLOCK_SIZE+1, BUTTON_ID_PICK);

	DrawCopyButton(67, 11, 35, 14, 0x777777);

	EventUpdateWindowContent();
}

void EventUpdateWindowContent()
{
	sprintf(#picked_color_string, "%A", picked_color);
	rgb.DwordToRgb(picked_color);
	
	EDI = 0xFFFfff;
	WriteText(12,12, 0xD0, 0x000111, #picked_color_string+4);
	
	WriteNumber(12,33, 0xD0, 0xff0000, 3, rgb.r);
	WriteNumber(43,33, 0xD0, 0x008000, 3, rgb.g);
	WriteNumber(73,33, 0xD0, 0x0000ff, 3, rgb.b);

	DrawBar(COLOR_BLOCK_X, COLOR_BLOCK_PADDING, COLOR_BLOCK_SIZE, COLOR_BLOCK_SIZE, picked_color);
}

void DrawCopyButton(dword _x, _y, _w, _h, _color)
{
	DefineHiddenButton(_x+1, _y+1, _w-2, _h-2, BUTTON_ID_COPY);
	DrawRectangle(_x, _y, _w, _h, _color);
	WriteTextCenter(_x, _h-8/2 + _y, _w, _color, "Copy");
}

void EventCopyHex()
{
	Clipboard__CopyText(#picked_color_string+4);
}