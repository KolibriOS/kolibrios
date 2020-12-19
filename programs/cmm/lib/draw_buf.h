#ifndef INCLUDE_DRAW_BUF_H
#define INCLUDE_DRAW_BUF_H
#print "[include <draw_buf.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

dword buf_data=0;


struct CANVAS {
	dword bufx, bufy, bufw, bufh;
	dword fill_color;

	bool Init();
	void Show();
	void Fill();
	void DrawBar();
	void WriteText();
	void PutPixel();
	void AlignCenter();
	void AlignRight();
	void IncreaseBufSize();
};

char draw_buf_not_enaught_ram[] = 
"'CANVAS requested %i MB more memory than the system has.
Application could be unstable.' -E";

bool CANVAS::Init(dword i_bufx, i_bufy, i_bufw, i_bufh)
{
	bufx = i_bufx;
	bufy = i_bufy;
	bufw = i_bufw; 
	bufh = i_bufh;
	if (buf_data) buf_data = free(buf_data);
	IncreaseBufSize();
	if (!buf_data) return false;
	ESDWORD[buf_data] = bufw;
	ESDWORD[buf_data+4] = bufh;
	return true;
}

void CANVAS::Fill(dword start_pointer, i_fill_color)
{
	dword max_i = bufw * bufh * 4 - start_pointer/4;
	fill_color = i_fill_color | 0xFF000000; //set background color non-transparent
	@MEMSETD(buf_data+start_pointer+8, max_i, fill_color);
}

void CANVAS::DrawBar(dword x, y, w, h, color)
{
	dword i, j;
	if (y + h >= bufh) IncreaseBufSize();
	for (j=0; j<h; j++)	{
		for (i = y+j*bufw+x<<2+8+buf_data; i<y+j*bufw+x+w<<2+8+buf_data; i+=4) {
			ESDWORD[i] = color;
		}
	}
}

void CANVAS::WriteText(dword x, y, byte fontType, dword color, str_offset, strlen)
{
	#define BUGFIX_32000 32000
	dword ydiv=0;
	dword reserve_data_1, reserve_data_2;
	dword new_buf_offset;
	if (y + 30 >= bufh) IncreaseBufSize();
	if (y < BUGFIX_32000) {
		ESI = strlen;
		WriteBufText(x, y, fontType, color, str_offset, buf_data);
	}
	else {
		ydiv = y / BUGFIX_32000 * BUGFIX_32000;
		y -= ydiv;
		new_buf_offset = ydiv * bufw * 4 + buf_data;

		reserve_data_1 = ESDWORD[new_buf_offset];
		reserve_data_2 = ESDWORD[new_buf_offset+4];

		ESDWORD[new_buf_offset] = bufw;
		ESDWORD[new_buf_offset+4] = bufh - y;
		ESI = strlen;
		WriteBufText(x, y, fontType, color, str_offset, new_buf_offset);

		ESDWORD[new_buf_offset] = reserve_data_1;
		ESDWORD[new_buf_offset+4] = reserve_data_2;
	}
}

void CANVAS::PutPixel(dword x, y, color)
{
	dword pos = y*bufw+x*4+8+buf_data;
	ESDWORD[pos] = color;
}

void CANVAS::AlignRight(dword x,y,w,h, content_width)
{
	dword i, j, l;
	dword content_left = w - content_width / 2;
	for (j=0; j<h; j++)
	{
		for (i=j*w+w-x*4, l=j*w+content_width+x*4; (i>=j*w+content_left*4) && (l>=j*w*4); i-=4, l-=4)
		{
			ESDWORD[buf_data+8+i] >< ESDWORD[buf_data+8+l];
		}
	}
}

void CANVAS::AlignCenter(dword x,y,w,h, content_width)
{
	dword i, j, l;
	dword content_left = w - content_width / 2;
	for (j=0; j<h; j++)
	{
		for (i=j*w+content_width+content_left*4, l=j*w+content_width+x*4; (i>=j*w+content_left*4) && (l>=j*w*4); i-=4, l-=4)
		{
			ESDWORD[buf_data+8+i] >< ESDWORD[buf_data+8+l];
		}
	}
}

void CANVAS::Show(dword _y_offset, _h)
{
	PutPaletteImage(_y_offset * bufw * 4 + buf_data+8, bufw, _h, bufx, bufy, 32, 0);
}

void CANVAS::IncreaseBufSize()
{
	static dword bufh_initial;
	dword alloc_size;
	dword free_ram_size;
	char error_str[256];

	if (!buf_data) {
		alloc_size = bufh * bufw * 4 + 8;
		buf_data = malloc(alloc_size);
	} else {
		if (bufh_initial != bufh) bufh_initial = bufh;
		bufh += 4096*1600/bufw; //+50 Mb
		alloc_size = bufh * bufw * 4 + 8;
		buf_data = realloc(buf_data, alloc_size);
		Fill(bufh_initial * bufw * 4 + 8, fill_color);
	}
	bufh_initial = bufh;
	free_ram_size = GetFreeRAM() * 1024;
	if (alloc_size > free_ram_size) {
		sprintf(#error_str, #draw_buf_not_enaught_ram, alloc_size - free_ram_size/1048576);
		notify(#error_str);
	}
}


#endif