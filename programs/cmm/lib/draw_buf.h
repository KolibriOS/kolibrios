#ifndef INCLUDE_DRAW_BUF_H
#define INCLUDE_DRAW_BUF_H
#print "[include <draw_buf.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

dword buf_data=0;


struct DrawBufer {
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
"'DrawBufer requested %i MB more memory than the system has.
Application could be unstable.' -E";

bool DrawBufer::Init(dword i_bufx, i_bufy, i_bufw, i_bufh)
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

void DrawBufer::Fill(dword start_pointer, i_fill_color)
{
	dword i;
	dword max_i = bufw * bufh * 4 + buf_data + 8;
	fill_color = i_fill_color;
	MEMSETD(buf_data+start_pointer+8, max_i-buf_data-start_pointer-8/4, fill_color);
}

void DrawBufer::DrawBar(dword x, y, w, h, color)
{
	dword i, j;
	if (y + h >= bufh) IncreaseBufSize();
	for (j=0; j<h; j++)	{
		for (i = y+j*bufw+x<<2+8+buf_data; i<y+j*bufw+x+w<<2+8+buf_data; i+=4) {
			ESDWORD[i] = color;
		}
	}
}

void DrawBufer::WriteText(dword x, y, byte fontType, dword color, str_offset, strlen)
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

void DrawBufer::PutPixel(dword x, y, color)
{
	dword pos = y*bufw+x*4+8+buf_data;
	ESDWORD[pos] = color;
}

void DrawBufer::AlignRight(dword x,y,w,h, content_width)
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

void DrawBufer::AlignCenter(dword x,y,w,h, content_width)
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

/*
void DrawBufer::Zoom2x(int zoom)
{
	int i, s;
	dword point_x, max_i, zline_w, s_inc;

	point_x = 0;
	max_i = bufw * bufh * 4 + buf_data+8;
	s_inc = zoom * 4;
	zline_w = zbufw * 4;

	for (i=buf_data+8, s=zbuf_data+8; i<max_i; i+=4, s+= s_inc) {
		ESDWORD[s] = ESDWORD[i];
		ESDWORD[s+4] = ESDWORD[i];
		ESDWORD[s+zline_w] = ESDWORD[i];
		ESDWORD[s+zline_w+4] = ESDWORD[i];
		if (zoom==3)
		{
			ESDWORD[s+8] = ESDWORD[i];
			ESDWORD[zline_w+s+8] = ESDWORD[i];
			ESDWORD[zline_w*2+s] = ESDWORD[i];
			ESDWORD[zline_w*2+s+4] = ESDWORD[i];
			ESDWORD[zline_w*2+s+8] = ESDWORD[i];
		}

		point_x++;
		if (point_x >= bufw) 
		{
			s += zoom - 1 * zline_w;
			point_x = 0;
		}
	}
}
*/


void DrawBufer::Show()
{
	PutPaletteImage(buf_data+8, bufw, bufh, bufx, bufy, 32, 0);	
}

void DrawBufer::IncreaseBufSize()
{
	static dword alloc_counter;
	static dword bufh_initial;
	dword alloc_size;
	dword free_ram_size;
	char error_str[256];

	if (!buf_data) {
		alloc_counter = 1;
		bufh_initial = bufh;
		alloc_size = bufw * bufh * 4 + 8;
		buf_data = malloc(alloc_size);
	}
	else {
		alloc_counter++;
		bufh = bufh_initial * alloc_counter;
		alloc_size = bufw * bufh * 4 + 8;
		buf_data = realloc(buf_data, alloc_size);
		Fill(alloc_counter - 1 * bufw * bufh_initial * 4 + 8, fill_color);
	}

	free_ram_size = GetFreeRAM() * 1024;
	if (alloc_size > free_ram_size) {
		sprintf(#error_str, #draw_buf_not_enaught_ram, alloc_size - free_ram_size/1048576);
		notify(#error_str);
	}
}



#endif