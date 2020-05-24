#ifndef INCLUDE_LIBIMG_LOAD_SKIN_H
#define INCLUDE_LIBIMG_LOAD_SKIN_H

#ifndef INCLUDE_LIBIMG_H
#include "../lib/obj/libimg.h"
#endif

:struct libimg_image {
	dword image, w, h, imgsrc;
	void load_as24b();
	void load();
	void replace_color();
	void fill_transparent();
} skin;

:void libimg_image::load_as24b(dword file_path)
{
	dword image_pointer = load_image(file_path);
	if (!image_pointer) notify("'Error: Image not loaded' -E");

	img_convert stdcall(image_pointer, 0, Image_bpp24, 0, 0);
	if (!EAX) {
		notify("'Error: Image can not be converted to 24b' -E");
	} else {
		image = image_pointer = EAX;
		w = DSWORD[image_pointer+4];
		h = DSWORD[image_pointer+8];
		imgsrc = ESDWORD[image_pointer+24];		
	}
}

:void libimg_image::load(dword file_path)
{
	dword image_pointer = load_image(file_path);
	if (!EAX) {
		notify("'Error: Image not loaded' -E");
	} else {
		image = image_pointer = EAX;
		w = DSWORD[image_pointer+4];
		h = DSWORD[image_pointer+8];
		imgsrc = ESDWORD[image_pointer+24];		
	}

}

:void libimg_image::replace_color(dword old_color, new_color)
{
	dword i, max_i;
	max_i =  w * h * 4 + imgsrc;
	for (i = imgsrc; i < max_i; i += 4)	if (DSDWORD[i]==old_color) DSDWORD[i] = new_color;
}

:void libimg_image::fill_transparent(new_color)
{
	if (new_color) replace_color(0, new_color);
}

:libimg_image icons32draw;
:void DrawIcon32(dword x,y, bg, icon_n) {
	//load_dll(libimg, #libimg_init,1);
	if (!icons32draw.image) {
		icons32draw.load("/sys/icons32.png");
		icons32draw.fill_transparent(bg);
	}
	if (icon_n>=0) img_draw stdcall(icons32draw.image, x, y, 32, 32, 0, icon_n*32);
}

:libimg_image icons16draw;
:void DrawIcon16(dword x,y, bg, icon_n) {
	//load_dll(libimg, #libimg_init,1);
	if (!icons16draw.image) {
		icons16draw.load("/sys/icons16.png");
		icons16draw.replace_color(0xffFFFfff, bg);
		icons16draw.replace_color(0xffCACBD6, MixColors(bg, 0, 220));
	}
	if (icon_n>=0) img_draw stdcall(icons16draw.image, x, y, 16, 16, 0, icon_n*16);
}

#endif