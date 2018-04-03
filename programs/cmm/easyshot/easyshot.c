// TODO
// Settings: delay, savepath
// Icons and better UI

#define MEMSIZE 1024 * 20
#include "../lib/kolibri.h" 
#include "../lib/strings.h" 
#include "../lib/mem.h" 
#include "../lib/gui.h" 

#include "../lib/obj/libimg.h"

#ifndef AUTOBUILD
	#include "lang.h--"
#endif

/* === TRANSLATIONS === */

#ifdef LANG_RUS
	?define T_TAKE_SCREENSHOT "Сделать скриншот"
	?define T_SAVE "Сохранить"
	?define T_PREVIEW "Предпросмотр"
#else
	?define T_TAKE_SCREENSHOT "Take a screenshot"
	?define T_SAVE "Save"
	?define T_PREVIEW "Preview"
#endif

/* === DATA === */

proc_info Form;

dword screenshot,
      preview;

int screenshot_length,
    preview_width,
    preview_height,
    preview_length;

enum {
	BTN_MAKE_SCREENSHOT=10,
	BTN_SAVE
};

#define TOOLBAR_H 46;

/* === CODE === */


void main()
{	
	char id;
	int take_scr_btn_width;

	load_dll(libimg, #libimg_init, 1);

	screenshot_length = screen.width * screen.height * 3;
	preview_width  = screen.width / 2;
	preview_height = screen.height / 2;
	preview_length = screenshot_length / 2;	

	screenshot  = malloc(screenshot_length);
	preview = malloc(screenshot_length/2);

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
			preview_width + 9, preview_height + skin_height + TOOLBAR_H + 4,
			0x74, 0, "EasyShot v0.5",0);
		GetProcessInfo(#Form, SelfInfo);
		if (Form.status_window>2) break;
		DrawBar(0, 0, Form.cwidth, TOOLBAR_H, system.color.work);
		take_scr_btn_width = DrawStandartCaptButton(10, 10, BTN_MAKE_SCREENSHOT, T_TAKE_SCREENSHOT);
		if (ESDWORD[preview]==0) {
			DrawBar(0, TOOLBAR_H,  preview_width, preview_height, 0xEEEeee);
			WriteText(Form.cwidth-calc(strlen(T_PREVIEW)*8)/2, Form.cheight/2, 0x90, 0x777777, T_PREVIEW);
		}
		else {
			_PutImage(0, TOOLBAR_H,  preview_width, preview_height, preview);
			DrawStandartCaptButton(take_scr_btn_width + 10, 10, BTN_SAVE, T_SAVE);
		}
	}
}

void EventTakeScreenshot() {
	MinimizeWindow();
	pause(100);
	CopyScreen(screenshot, 0, 0, screen.width, screen.height);
	ZoomImageTo50percent();
	ActivateWindow(GetProcessSlot(Form.ID));
}

void EventSaveFile()
{
	int i=0;
	char save_file_name[4096];
	do {
		i++;
		sprintf(#save_file_name, "/tmp0/1/screen_%i.png", i);
	} while (file_exists(#save_file_name));
	SaveFile(screenshot, screen.width, screen.height, #save_file_name);
}

void SaveFile(dword _image, _w, _h, _path)
{
	char save_success_message[4096+200];
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
				sprintf(#save_success_message, "'File saved as %s' -O", _path);
				notify(#save_success_message);
			}
			else {
				notify("'Error saving file! Probably not enought space or file system is not writable!' -E");
			}
		}
	}
}

inline byte calc_rgb(dword B, item_h)
{
	return calc(ESBYTE[B+3] + ESBYTE[B] + ESBYTE[B-3]
		+ ESBYTE[B-item_h] + ESBYTE[B+item_h] / 5);
}

void ZoomImageTo50percent() {
	dword point_x = 0;
	dword item_h = screen.width * 3;
	dword small = preview;
	dword big = screenshot;

	while( (small <= preview + preview_length) && (big <= screenshot + screenshot_length ) ) {
		
		if (big <= screenshot + item_h) || (big >= screenshot + screenshot_length - item_h)
		{
			ESBYTE[small]   = ESBYTE[big];
			ESBYTE[small+1] = ESBYTE[big+1];
			ESBYTE[small+2] = ESBYTE[big+2];
		}
		else
		{
			ESBYTE[small]   = calc_rgb(big, item_h);
			ESBYTE[small+1] = calc_rgb(big+1, item_h);
			ESBYTE[small+2] = calc_rgb(big+2, item_h);
		}
	
		small+=3;
		big+=6;

		point_x+=2;
		if (point_x >= screen.width) 
		{
			big += item_h;
			point_x = 0;
		}
	}
}



stop:
