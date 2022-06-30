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
char picked_rgb_string[12];

#define PICKED_SIZE 48
#define LABEL_RGB_W 97
#define PAD 11
#define PICKED_X PAD + LABEL_RGB_W + PAD

#define FORM_W PICKED_X + PICKED_SIZE + PAD + 1
#define FORM_H PAD + PICKED_SIZE + PAD + 3

#define BUTTON_CLOSE    1
#define BUTTON_PICK     2
#define BUTTON_COPY_HEX 3
#define BUTTON_COPY_RGB 4

#define COLOR_WIN_BG   0xD6D7DA
#define COLOR_PANE_BG  0xFFFfff
#define COLOR_3D_LIGHT 0xADAAA9
#define COLOR_3D_DARK  0x888888

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
			else if (EAX == BUTTON_COPY_HEX) EventCopyHex();
			else if (EAX == BUTTON_COPY_RGB) EventCopyRgb();
			else if (EAX == BUTTON_PICK) { pick_active = true; EventActivatePick(); }
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
	DrawRectangle(0, 0, FORM_W, FORM_H, COLOR_3D_DARK);
	DrawRectangle(1, 1, FORM_W-2, FORM_H-2, COLOR_PANE_BG);
	DrawRectangle(2, 2, FORM_W-4, FORM_H-4, COLOR_3D_LIGHT);
	DrawBar(3,3,FORM_W-5,FORM_H-5, COLOR_WIN_BG);
	DrawButton3D(PAD, PAD, 58, 21, BUTTON_COPY_HEX, COLOR_PANE_BG);
	DrawButton3D(PAD, 41, LABEL_RGB_W, 21, BUTTON_COPY_RGB, COLOR_PANE_BG);
	DrawButton3D(PICKED_X-2, PAD, PICKED_SIZE+3, PICKED_SIZE+3, BUTTON_PICK, -1);
	EventUpdateWindowContent();
}

int DrawButton3D(dword _x, _y, _w, _h, _id, _col)
{
	DefineHiddenButton(_x+1, _y+1, _w-2, _h-2, _id);
	DrawRectangle3D(_x, _y, _w, _h, COLOR_3D_LIGHT, COLOR_3D_DARK);
	DrawRectangle3D(_x+1, _y+1, _w-2, _h-2, COLOR_PANE_BG, COLOR_PANE_BG);
	if (_col) DrawBar(_x+2, _y+2, _w-2, _h-2, _col);
}

//copy of sprintf() => %A
void str2col(dword buf, number)
{
	byte s;
	strcpy(buf,"000000");
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
	
	WriteTextWithBg(PAD+5, PAD+4, 0xD0, 0x000111, #picked_color_string, COLOR_PANE_BG);
	
	WriteNumber(PAD+04, PAD+PAD+23, 0xD0, 0xff0000, 3<<16, rgb.r);
	WriteNumber(PAD+36, PAD+PAD+23, 0xD0, 0x008000, 3<<16, rgb.g);
	WriteNumber(PAD+67, PAD+PAD+23, 0xD0, 0x0000ff, 3<<16, rgb.b);

	DrawBar(PICKED_X, PAD+2, PICKED_SIZE, PICKED_SIZE, picked_color);
}

void EventActivatePick()
{
	DrawBar(PICKED_X, PAD+2, PICKED_SIZE, PICKED_SIZE, COLOR_PANE_BG);
	WriteTextWithBg(-4*8+PICKED_SIZE/2+PICKED_X, PICKED_SIZE/2+PAD-5, 0xD0, 0x000111, "Pick", COLOR_PANE_BG);
}

void EventCopyHex()
{
	Clipboard__CopyText(#picked_color_string);
	WriteTextWithBg(PAD+5, PAD+4, 0xD0, 0xD100C6, "Copied", COLOR_PANE_BG);
	pause(50);
	draw_window();
}

void EventCopyRgb()
{
	strcpy(#picked_rgb_string, "000,000,000");
	rgb2str(#picked_rgb_string, rgb.r);
	rgb2str(#picked_rgb_string+4, rgb.g);
	rgb2str(#picked_rgb_string+8, rgb.b);
	Clipboard__CopyText(#picked_rgb_string);
	WriteTextWithBg(PAD+1, PAD+PAD+23, 0xD0, 0xD100C6, "   Copied   ", COLOR_PANE_BG);
	pause(50);
	draw_window();
}

void rgb2str(int _str, _i)
{
	if (_i < 10) {
		ESBYTE[_str+2] = _i + '0';
	} else if (_i < 100) {
		//23
		ESBYTE[_str+1] = _i / 10 + '0';
		ESBYTE[_str+2] = _i % 10 + '0';
	} else {
		//123
		ESBYTE[_str+0] = _i / 100 + '0';
		ESBYTE[_str+1] = _i / 10 % 10 + '0';
		ESBYTE[_str+2] = _i % 100 % 10 + '0';
	}
}

