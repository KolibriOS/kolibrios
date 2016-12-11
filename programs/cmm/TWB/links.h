CustomCursor CursorPointer;
dword CursorFile = FROM "../TWB/pointer.cur";
#include "..\lib\collection.h"

#define NOLINE    0
#define UNDERLINE 1

#define MAXLINKS 400

struct array_link {
	dword link, text;
	int x,y,w,h;
	int underline, underline_h;
};

struct LinksArray {
	array_link links[MAXLINKS];
	collection page_links;
	dword buflen;
	int count, active;
	void Hover();
	void AddLink();
	void AddText();
	dword GetURL();
	void Clear();
} PageLinks;

void LinksArray::AddLink(dword lpath, int link_x, link_y)
{
	if (count>= MAXLINKS) return;
	links[count].x = link_x;
	links[count].y = link_y;

	page_links.add(lpath);
	links[count].link = page_links.get(page_links.count-1);
	count++;
}

void LinksArray::AddText(dword link_w, link_h, link_underline, _underline_h, new_text)
{
	if (count>= MAXLINKS) || (!count) return;
	links[count-1].w = link_w;
	links[count-1].h = link_h;
	links[count-1].underline = link_underline;
	links[count-1].underline_h = _underline_h;

	page_links.add(new_text);
	links[count-1].text = page_links.get(page_links.count-1);
}

dword LinksArray::GetURL(int id)
{
	return links[id].link;
}

void LinksArray::Clear()
{
	page_links.drop();
	page_links.realloc_size = 4096 * 32;
	count = 0;
	active = -1;
	CursorPointer.Restore();
}

char temp[sizeof(URL)];
PathShow_data status_text = {0, 17,250, 6, 250, 0, 0, 0x0, 0xFFFfff, 0, #temp, 0};

void LinksArray::Hover(dword mx, my, link_col_in, link_col_a, bg_col)
{
	int i;
	for (i=0; i<count; i++)
	{
		if (mx>links[i].x) && (my>links[i].y) && (mx<links[i].x+links[i].w) && (my<links[i].y+links[i].h)
		{
			if (mouse.down) DrawRectangle(links[active].x, -WB1.list.first + links[active].y, 
				links[active].w, links[active].h, 0);
			if (mouse.up) ClickLink();
			if (active==i) return;
			CursorPointer.Set();
			if (links[active].underline) DrawBar(links[active].x, -WB1.list.first + links[active].y
				+ links[active].h, links[active].w, links[i].underline_h, link_col_in);
			if (links[i].underline) DrawBar(links[i].x, -WB1.list.first + links[i].y
				+ links[i].h, links[i].w, links[i].underline_h, bg_col);
			active = i;
			status_text.start_x = wv_progress_bar.left + wv_progress_bar.width + 10;
			status_text.start_y = Form.cheight - STATUSBAR_H + 3;
			status_text.area_size_x = Form.cwidth - status_text.start_x -3;
			DrawBar(status_text.start_x, status_text.start_y, status_text.area_size_x, 9, col_bg);
			status_text.text_pointer = links[active].link;
			PathShow_prepare stdcall(#status_text);
			PathShow_draw stdcall(#status_text);
			return;
		}
	}
	if (active!=-1)
	{
		CursorPointer.Restore();
		if (links[active].underline) DrawBar(links[active].x, -WB1.list.first + links[active].y  + links[active].h,links[active].w, WB1.DrawBuf.zoom, link_col_in);
		DrawBar(status_text.start_x, status_text.start_y, status_text.area_size_x, 9, col_bg);
		active = -1;
	}
}
