#define MEMSIZE 1024*20

#include "../lib/gui.h"
#include "../lib/clipboard.h"
#include "../lib/patterns/rgb.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

dword picked_color;
char picked_color_string[7];
#define FORM_W 167
#define FORM_H 60
#define PICKED_SIZE 42
#define PICKED_PADDING FORM_H - PICKED_SIZE / 2
#define PICKED_X FORM_W - PICKED_SIZE - PICKED_PADDING
#define BUTTON_CLOSE 1
#define BUTTON_COPY  2
#define BUTTON_PICK  3

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	bool pick_active = true;
	proc_info Form;
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE);
	@SetWindowLayerBehaviour(-1, ZPOS_ALWAYS_TOP);
	loop() switch(@WaitEvent())
	{
		case evMouse:
			mouse.get();			
			if (mouse.x>0) && (mouse.x<FORM_W) && (mouse.y>0) && (mouse.y<FORM_H) {
				EventDragWindow();
			} else if (pick_active) {
				picked_color = GetPixelColorFromScreen(mouse.x + Form.left, mouse.y + Form.top);
				EventUpdateWindowContent();
				if (mouse.down) && (mouse.key&MOUSE_LEFT) pick_active = false;
			}
			break;

		case evButton:
			@GetButtonID();
			if (EAX == BUTTON_CLOSE) ExitProcess();
			if (EAX == BUTTON_COPY) EventCopyHex();
			if (EAX == BUTTON_PICK) pick_active = true;
			break;

		case evKey:
			@GetKeyScancode();
			if (AL == SCAN_CODE_ESC) @ExitProcess();
			if (AL == SCAN_CODE_KEY_C) EventCopyHex();
			break;
		 
		case evReDraw:
			DefineUnDragableWindow(215, 100, FORM_W, FORM_H);
			GetProcessInfo(#Form, SelfInfo);
			draw_window();
	}
}

void draw_window()
{
	DrawRectangle3D(0, 0, FORM_W, FORM_H, 0xCCCccc, 0x888888);
	DrawRectangle3D(1, 1, FORM_W-2, FORM_H-2, 0xCCCccc, 0x888888);
	DrawBar(2,2,FORM_W-3,FORM_H-3,0xFFFfff);

	DrawRectangle(PICKED_X-2, PICKED_PADDING-2, PICKED_SIZE+3, PICKED_SIZE+3, 0xCBC6C5);
	DefineHiddenButton(PICKED_X-1, PICKED_PADDING-1, PICKED_SIZE+1, PICKED_SIZE+1, BUTTON_PICK);

	DrawCopyButton(67, 11, 35, 14);

	EventUpdateWindowContent();
}

//copy of sprintf() => %A
void str2col(dword buf, number)
{
	byte s;
	strlcpy(buf,"000000",6);
	buf+=6;
	while(number)
	{
		$dec buf
		s=number&0xF;
		if(s>9)DSBYTE[buf]='A'+s-10;
		else DSBYTE[buf]='0'+s;
		number>>=4;
	}
}

void EventUpdateWindowContent()
{
	str2col(#picked_color_string, picked_color);
	rgb.DwordToRgb(picked_color);
	
	WriteTextWithBg(12,12, 0xD0, 0x000111, #picked_color_string, 0xFFFfff);
	
	WriteNumber(12,33, 0xD0, 0xff0000, 3<<16, rgb.r);
	WriteNumber(44,33, 0xD0, 0x008000, 3<<16, rgb.g);
	WriteNumber(75,33, 0xD0, 0x0000ff, 3<<16, rgb.b);

	DrawBar(PICKED_X, PICKED_PADDING, PICKED_SIZE, PICKED_SIZE, picked_color);
}

void DrawCopyButton(dword _x, _y, _w, _h)
{
	DefineHiddenButton(_x+1, _y+1, _w-2, _h-2, BUTTON_COPY);
	DrawRectangle(_x, _y, _w, _h, 0x777777);
	WriteText(_x+6, _h-8/2 + _y, 0x80, 0x555555, "Copy");
}

void EventCopyHex()
{
	Clipboard__CopyText(#picked_color_string);
}