#ifndef INCLUDE_GUI_H
#define INCLUDE_GUI_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_STRING_H
#include "../lib/strings.h"
#endif

#ifndef INCLUDE_RGB_H
#include "../lib/patterns/rgb.h"
#endif

#ifndef INCLUDE_MATH_H
#include "../lib/math.h"
#endif

#include "../lib/gui/tabs.h"
#include "../lib/gui/sensor.h"
#include "../lib/gui/more_less_box.h"

#ifndef INCLUDE_CHECKBOX
#include "../lib/gui/checkbox.h"
#endif

#include "../lib/gui/child_window.h"
#include "../lib/gui/text_view_area.h"
#include "../lib/gui/reshare.h"

#ifndef INCLUDE_MENU_H
#include "../lib/gui/menu.h"
#endif

:int last_free_button_id = 1000;
:int GetFreeButtonId()
{
	last_free_button_id++;
	return last_free_button_id;
}

:void DrawRectangle(dword x,y,w,h,color1)
{
	DrawRectangle3D(x,y,w,h,color1,color1);
	/*
	DrawBar(x,y,w,1,color1);
	DrawBar(x,y+h,w,1,color1);
	DrawBar(x,y,1,h,color1);
	DrawBar(x+w,y,1,h+1,color1);
	*/
}

:void DrawWideRectangle(dword x,y,w,h,boder,color1)
{
	DrawBar(x, y, w, boder, color1);
	DrawBar(x, y+h-boder, w, boder, EDX);
	DrawBar(x, y+boder, boder, h-boder-boder, EDX);
	DrawBar(x+w-boder, y+boder, boder, h-boder-boder, EDX);
}

:void DrawRectangle3D(dword x,y,w,h,color1,color2)
{
	DrawBar(x,y,w+1,1,color1);
	DrawBar(x,y+1,1,h-1,EDX);
	DrawBar(x+w,y+1,1,h,color2);
	DrawBar(x,y+h,w,1,EDX);
}

:void DrawCaptButton(dword x,y,w,h,id,color_b, color_t,text)
{
	dword tx = -strlen(text)*8+w/2+x;
	dword ty = h/2-7+y;
	DefineButton(x,y,w,h,id,color_b);
	WriteText(tx+1,ty+1,0x90,MixColors(color_b,0,230),text);
	WriteText(tx,ty,0x90,color_t,text);
}

:int active_button_id = 0;
:int DrawStandartCaptButton(dword x, y, id, text)
{
	#define padding_v 5
	#define padding_h 15
	#define right_margin 12
	#define h padding_v + padding_v + 16 //16 font height
	int tw = strlen(text)*8;
	int w = tw + padding_h + padding_h;
	unsigned darker_color = MixColors(sc.button,0,230);

	DefineButton(x,y,w,h,id,sc.button);

	x += padding_h;
	y += padding_v+1;

	WriteText(x+1,y+1,0x90,darker_color,text);
	WriteText(x,y,0x90,sc.button_text,text);

	if (active_button_id==id) {
		
		DrawBar(x,y+15,tw,1, darker_color);
		DrawBar(x,y+14,tw,1, sc.button_text);
	}

	return w + right_margin;
}

/* UNSTABLE
:unsigned LightenDarkenColor(dword color, amt) {
	dword r = color >> 16 + amt << 16;
	dword b = color >> 8 & 0x00FF + amt << 8;
	dword g = color & 0x0000FF + amt;
	return g | b | r ;
}
*/

:void ActiveButtonSwitch(int min, max)
{
	active_button_id++;
	if (active_button_id>max) || (active_button_id<max) active_button_id=min;
}

:void WriteTextCenter(dword x,y,w,color_t,text)
{
	WriteText(-strlen(text)*6+w/2+x+1,y,0x80,color_t,text);
}

:void DrawCircle(int x, y, r, color)
{
	int i;
	float px=0, py=r, ii = r * 3.1415926 * 2;
	FOR (i = 0; i < ii; i++)
	{
        PutPixel(px + x, y - py, color);
        px = py / r + px;
        py = -px / r + py;
	}
}

:void DrawEditBox(dword edit_box_pointer)
{
	dword x,y,w,h,bg;
	ESI = edit_box_pointer;
	x = ESI.edit_box.left;
	y = ESI.edit_box.top;
	w = ESI.edit_box.width+1;
	h = 22;
	if (ESI.edit_box.flags & 100000000000b) bg = 0xCACACA; else bg = 0xFFFfff;
	edit_box_draw  stdcall (edit_box_pointer);
	DrawRectangle3D(x-1, y-1, w+1, h+1, 0xE7E7E7, bg);
	DrawRectangle(x-2, y-2, w+3, h+3, sc.line);
	DrawRectangle3D(x-3, y-3, w+5, h+5, sc.dark, sc.light);
}

#define DOT_W 37 
:void DrawFileBox(dword edit_box_pointer, title, btn)
{
	dword x,y,w,bg,t;
	ESI = edit_box_pointer;
	x = ESI.edit_box.left;
	y = ESI.edit_box.top;
	w = ESI.edit_box.width+1;
	#define box_h 22

	if (ESI.edit_box.flags & 100000000000b) bg = 0xCACACA; else bg = 0xFFFfff;
	edit_box_draw  stdcall (edit_box_pointer);
	DrawRectangle3D(x-1, y-1, w+1, box_h+1, 0xE7E7E7, bg);
	DrawRectangle(x-2, y-2, w+3, box_h+3, sc.line);
	DrawRectangle3D(x-3, y-3, w+DOT_W+5, box_h+5, sc.dark, sc.light);

	WriteText(x-2, y-19, 0x90, sc.work_text, title);
	DrawCaptButton(x+w+1, y-2, DOT_W, box_h+3, btn, sc.button, sc.button_text, "...");
}

:void DrawEditBoxPos(dword x,y, edit_box_pointer)
{
	ESI = edit_box_pointer;
	ESI.edit_box.left = x;
	ESI.edit_box.top = y;
	DrawEditBox(dword edit_box_pointer);
}

:void DrawProgressBar(dword st_x, st_y, st_w, st_h, col_fon, col_border, col_fill, progress_percent)
{
	int progress_w;
	static int fill_old;
	    
	//if (progress_percent<=0) {DrawBar(st_x,st_y, st_x + st_w + fill_old + 15,st_h+1, col_fon); fill_old=0; return;}
	if (progress_percent<=0) || (progress_percent>=100) return;
	
	DrawRectangle(st_x, st_y, st_w,st_h, col_border);
	DrawRectangle3D(st_x+1, st_y+1, st_w-2,st_h-2, 0xFFFfff, 0xFFFfff);

	if (progress_percent>0) && (progress_percent<=100)
	{
		progress_w = st_w - 3 * progress_percent / 100;
		DrawBar(st_x+2, st_y+2, progress_w, st_h-3, col_fill);
		DrawBar(st_x+2+progress_w, st_y+2, st_w-progress_w-3, st_h-3, 0xFFFfff);
	}
}

:void DrawLink(dword x,y,font_type,btn_id, inscription)
{
	int w;
	WriteText(x,y,font_type,0x4E00E7,inscription);
	if (font_type==0x80) w = strlen(inscription)*6; else w = strlen(inscription)*8;
	DefineButton(x-1,y-1,w,10,btn_id+BT_HIDE,0);
	DrawBar(x,y+8,w,1,0x4E00E7);
}

:void PutShadow(dword x,y,w,h,skinned, signed strength)
{
	proc_info wForm;
	dword shadow_buf = mem_Alloc(w*h*3);
 	GetProcessInfo(#wForm, SelfInfo);
	CopyScreen(shadow_buf, 5*skinned+x+wForm.left, GetSkinHeight()*skinned+y+wForm.top, w, h);
	ShadowImage(shadow_buf, w, h, strength);
	PutImage(x,y,w,h,shadow_buf);
	mem_Free(shadow_buf);
}

:void DrawPopup(dword x,y,w,h,skinned, col_work,col_border)
{
	DrawRectangle(x,y,w,h,col_border);
	DrawBar(x+1,y+1,w-1,1,0xFFFfff);
	DrawBar(x+1,y+2,1,h-2,EDX);
	if (col_work!=-1) DrawBar(x+2,y+2,w-2,h-2,col_work);
	DrawPopupShadow(x,y,w,h-1,skinned);
}

:void Draw3DPopup(dword x,y,w,h)
{
	DrawRectangle3D(x,y,w,h, sc.dark, sc.line);
	DrawBar(x+1,y+1,w-1,1,sc.light);
	DrawBar(x+1,y+2,1,h-2,sc.light);
	DrawPopupShadow(x,y,w,h-1,0);
}

:void DrawPopupShadow(dword x,y,w,h,skinned)
{
	PutShadow(w+x+1,y,1,h+2,skinned,2);
	PutShadow(w+x+2,y+1,1,h+2,skinned,1);
	PutShadow(x,y+h+2,w+2,1,skinned,2);
	PutShadow(x+1,y+h+3,w+1,1,skinned,1);
}

:dword GrayScaleImage(dword color_image, w, h)
{
	dword i,gray,to,rr,gg,bb;
	to = w*h*3 + color_image;
	for (i = color_image; i < to; i+=3)
	{
		rr = DSBYTE[i];
		gg = DSBYTE[i+1];
		bb = DSBYTE[i+2];
		gray = rr*rr;
		gray += gg*gg;
		gray += bb*bb;
		gray = sqrt(gray) / 3;
		DSBYTE[i] = DSBYTE[i+1] = DSBYTE[i+2] = gray;
	}
	return gray;
}

:void ShadowImage(dword color_image, w, h, signed strength)
{
	byte col;
	dword to;
	strength = 10 - strength;
	to = w*h*3 + color_image;
	for ( ; color_image < to; color_image++)
	{
		col = math.min(strength * DSBYTE[color_image] / 10, 255);
		DSBYTE[color_image] = col;
	}
}

:void WriteTextLines(dword x,y,byte fontType, dword color, text_pointer, line_h)
{
	dword next_word_pointer = strchr(text_pointer, '\n');
	if (next_word_pointer) WriteTextLines(dword x, y+line_h, byte fontType, dword color, next_word_pointer+2, line_h);
	ESBYTE[next_word_pointer] = NULL;
	WriteText(dword x, y, byte fontType, dword color, text_pointer);
	ESBYTE[next_word_pointer] = '\n';
}

:void DrawOvalBorder(dword x,y,w,h, light,dark,right,dots)
{
	DrawBar(x+1,    y,     w, 1,   light);
	DrawBar(x+1,    y+h+1, w, 1,   dark);
	DrawBar(x,      y+1,   1, h-1, light);
	DrawBar(x+w+1,  y+2,   1, h-2, right);

	PutPixel(x,     y,     dots);
	PutPixel(x+w+1, y+h+1, EDX);
	PutPixel(x,     y+h+1, EDX);
	PutPixel(x+w+1, y,     EDX);
	
	PutPixel(x,     y+h, dark);
	PutPixel(x+w+1, y+1, light);
	PutPixel(x+w+1, y+h, dark);	
}

:bool skin_is_dark()
{
	ESI = #sc.work;

	EDI = DSBYTE[ESI]*DSBYTE[ESI];
	EDI += DSBYTE[ESI+1]*DSBYTE[ESI+1];
	EDI += DSBYTE[ESI+2]*DSBYTE[ESI+2];

	if (sqrt(EDI) / 3 < 65) {
		return true; 
	} else {
		return false;
	}
}

//this function increase falue and return it
//useful for list of controls which goes one after one
:struct incn
{
	dword n;
	dword inc(dword _addition);
	dword set(dword _new_val);
};

:dword incn::inc(dword _addition)
{
	n += _addition;
	return n;
}

:dword incn::set(dword _new_val)
{
	n =_new_val;
	return n;
}

//block with hover
struct block {
	int x,y,w,h;
	bool hovered();
	void set_size();
};

:bool block::hovered() {
	if ((mouse.x>=x) && (mouse.y>=y) 
	&& (mouse.y<=y+h) && (mouse.x<=x+w)) 
		return true;
	return false;
}

:void block::set_size(dword _x, _y, _w, _h)
{
	x=_x; 
	y=_y;
	w=_w;
	h=_h;
}


#endif