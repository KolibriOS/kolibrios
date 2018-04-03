#define MEMSIZE 1024 * 420
#include "../lib/kolibri.h" 
#include "../lib/strings.h" 
#include "../lib/mem.h" 
#include "../lib/gui.h" 

#include "../lib/obj/libimg.h"

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

/* === DATA === */

proc_info Form;

dword b_screen,
      preview;

int b_screen_length,
    preview_width,
    preview_height,
    preview_length;

enum {
	BTN_MAKE_SCREENSHOT=10,
	BTN_SAVE
};

#define TOOLBAR_H 50;

/* === CODE === */


void main()
{	
	char id;
	b_screen_length = screen.width * screen.height * 3;
	preview_width  = screen.width / 2;
	preview_height = screen.height / 2;
	preview_length = b_screen_length / 2;	

	b_screen  = malloc(b_screen_length);
	preview = malloc(b_screen_length/2);

	loop() switch(WaitEvent())
	{
	case evButton:
		id = GetButtonID();
		if (id == CLOSE_BTN) ExitProcess();
		if (id == BTN_MAKE_SCREENSHOT) EventTakeScreenshot();
		if (id == BTN_SAVE) EventSaveFile();
		break;

	case evKey:
		GetKeys();
		if (SCAN_CODE_KEY_S == key_scancode) EventSaveFile();
		if (SCAN_CODE_ENTER == key_scancode) EventTakeScreenshot();
		break;
     
	case evReDraw:
		system.color.get();
		DefineAndDrawWindow(screen.width/4, screen.height/4, 
			preview_width + 9, preview_height + skin_height + TOOLBAR_H,
			0x74, 0, "EasyShot v0.3",0);
		GetProcessInfo(#Form, SelfInfo);
		if (Form.status_window>2) break;
		DrawBar(0, 0, Form.cwidth, TOOLBAR_H-4, system.color.work);
		DrawStandartCaptButton(10, 10, BTN_MAKE_SCREENSHOT, "Take a screenshot");
		_PutImage(0, Form.cheight - preview_height,  preview_width, preview_height, preview);
		if (ESDWORD[preview]==0) {
			WriteTextB(Form.cwidth/2 - 90, Form.cheight/2+10, 0x90, 0xFFFfff, "There will be a preview");
		}
		else {
			DrawStandartCaptButton(200, 10, BTN_SAVE, "Save");
		}
	}
}

void EventTakeScreenshot() {
	MinimizeWindow();
	pause(100);
	CopyScreen(b_screen, 0, 0, screen.width, screen.height);
	ZoomImageTo50percent();
	ActivateWindow(GetProcessSlot(Form.ID));
	//_PutImage(0, Form.cheight - preview_height,  preview_width, preview_height, preview);
}

void EventSaveFile()
{
	SaveFile(b_screen, screen.width, screen.height, "/tmp0/1/screen.png");
}

void SaveFile(dword _image, _w, _h, _path)
{
	dword encoded_data=0;
	dword encoded_size=0;
	dword image_ptr = 0;
	
	image_ptr = create_image(Image_bpp24, _w, _h);

	if (image_ptr == 0) {
		notify("'Error saving file, probably not enought memory!' -E");
	}
	else {
		EDI = image_ptr;
		memmov(EDI._Image.Data, _image, _w * _h * 3);

		encoded_data = encode_image(image_ptr, LIBIMG_FORMAT_PNG, 0, #encoded_size);

		img_destroy stdcall(image_ptr);

		if(encoded_data == 0) {
			notify("'Error saving file, incorrect data!' -E");
		}
		else {
			if (WriteFile(encoded_size, encoded_data, _path) == 0) {
				notify("'File saved as /rd/1/saved_image.png' -O");
			}
			else {
				notify("'Error saving file, probably not enought space on ramdisk!' -E");
			}
		}
	}
}

void ZoomImageTo50percent() {
	dword point_x,
	      item_h= screen.width * 3,
	      s_off = preview + 3,
	      b_off = b_screen + 6,
	      b_off_r,
	      b_off_g,
	      b_off_b,
	      rez_r, 
	      rez_g, 
	      rez_b;

	while( (s_off <= preview + preview_length) && (b_off <= b_screen + b_screen_length ) ) {
		
		if (b_off <= b_screen + item_h) || (b_off >= b_screen + b_screen_length - item_h)
		{
			ESBYTE[s_off]   = ESBYTE[b_off];
			ESBYTE[s_off+1] = ESBYTE[b_off+1];
			ESBYTE[s_off+2] = ESBYTE[b_off+2];
		}
		else
		{
			// line[x].R = (line[x+1].R + line[x].R + line[x-1].R + line1[x].R + line2[x].R) / 5;
			// line[x].G = (line[x+1].G + line[x].G + line[x-1].G + line1[x].G + line2[x].G) / 5;
			// line[x].B = (line[x+1].B + line[x].B + line[x-1].B + line1[x].B + line2[x].B) / 5
			b_off_r = b_off;
			b_off_g = b_off + 1;
			b_off_b = b_off + 2;
			rez_r = ESBYTE[b_off_r+3] + ESBYTE[b_off_r] + ESBYTE[b_off_r-3] + ESBYTE[b_off_r-item_h] + ESBYTE[b_off_r+item_h] / 5;
			rez_g = ESBYTE[b_off_g+3] + ESBYTE[b_off_g] + ESBYTE[b_off_g-3] + ESBYTE[b_off_g-item_h] + ESBYTE[b_off_g+item_h] / 5;
			rez_b = ESBYTE[b_off_b+3] + ESBYTE[b_off_b] + ESBYTE[b_off_b-3] + ESBYTE[b_off_b-item_h] + ESBYTE[b_off_b+item_h] / 5;
			ESBYTE[s_off] = rez_r;
			ESBYTE[s_off+1] = rez_g;
			ESBYTE[s_off+2] = rez_b;

		}
	
		s_off+=3;
		b_off+=6;

		point_x+=2;
		if (point_x >= screen.width) 
		{
			b_off += item_h;
			point_x = 0;
		}
	}
}


stop:
