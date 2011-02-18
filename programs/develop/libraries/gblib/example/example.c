
#include "system/kolibri.h"
#include "system/stdlib.h"
#include "system/string.h"

#include "gblib.h"


/// ===========================================================

void kol_main()
{

GB_BMP			b;
kol_struct_import	*imp_gblib;
unsigned		event;

imp_gblib = kol_cofflib_load("/sys/lib/gblib.obj"); 

gb_pixel_set = kol_cofflib_procload (imp_gblib, "gb_pixel_set");
gb_pixel_get = kol_cofflib_procload (imp_gblib, "gb_pixel_get");
gb_line = kol_cofflib_procload (imp_gblib, "gb_line");
gb_rect = kol_cofflib_procload (imp_gblib, "gb_rect");
gb_bar = kol_cofflib_procload (imp_gblib, "gb_bar");
gb_circle = kol_cofflib_procload (imp_gblib, "gb_circle");
gb_image_set = kol_cofflib_procload (imp_gblib, "gb_image_set");
gb_image_set_t = kol_cofflib_procload (imp_gblib, "gb_image_set_t");

b.w = 300; 
b.h = 200;
b.bmp = malloc (300*200*3);

gb_bar (&b, 4, 8, 4, 12, 0xff0000); // red
gb_bar (&b, 10, 8, 4, 12, 0x00ff00); // green
gb_bar (&b, 16, 8, 4, 12, 0x0000ff); // blue

gb_line(&b, 4, 30, 50, 30, 0xffffff); // white line
gb_line(&b, 55, 4, 120, 60, 0xf0f033); // another line

gb_rect(&b, 65, 24, 100, 60, 0x2065ff); // rectangle

gb_circle(&b, 55, 95, 40, 0x20ff20); // circle

for (;;)
	{
	event = kol_event_wait();

	switch (event)
		{
		case 1:
			kol_paint_start();
			kol_wnd_define(50, 50, 350, 240, 0x34f0f0f0);
			kol_paint_image(3, 3, 300, 200, b.bmp);
			kol_paint_end();
			break;

		case 2:
			kol_key_get();
			break;

		case 3:
			if ( 1 == (kol_btn_get() & 0xff00)>>8 )
				kol_exit();
			break;

		};


	}

}

/// ===========================================================
