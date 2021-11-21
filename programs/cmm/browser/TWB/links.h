CustomCursor CursorPointer;
dword CursorFile = FROM "TWB/pointer.cur";
#include "..\lib\collection.h"

struct PAGE_LINKS {
	collection unic_links;
	collection element_links;
	
	collection_int x;
	collection_int y;
	collection_int w;
	collection_int h;
	collection_int id;
	collection_int underline_h;

	signed int active;
	dword active_url;
	bool hover();
	void add_link();
	void add_text();
	void clear();
	void draw_underline();
} links;

void PAGE_LINKS::add_link(dword lpath)
{
	unic_links.add(lpath);
}

void PAGE_LINKS::add_text(dword _x, _y, _w, _h, _underline_h)
{
	x.add(_x);
	y.add(_y);
	w.add(_w);
	h.add(_h);
	underline_h.add(_underline_h);
	element_links.add(unic_links.get_last());
	id.add(unic_links.count);
}

void PAGE_LINKS::clear()
{
	x.drop();
	y.drop();
	w.drop();
	h.drop();
	underline_h.drop();
	element_links.drop();
	unic_links.drop();
	id.drop();

	active = -1;
	active_url = 0;
	CursorPointer.Restore();
}

void PAGE_LINKS::draw_underline(signed _id, dword list_first, list_y, color)
{
	int i;
	if (_id == -1) return;
	for (i=0; i<id.count; i++) 
	{
		if (id.get(i) - id.get(_id) == 0) 
		&& (y.get(i) + h.get(i) - list_first > list_y) {
			DrawBar(x.get(i), y.get(i) + h.get(i) - list_first, 
				w.get(i), underline_h.get(i), color);
		}		
	}
}

bool PAGE_LINKS::hover(dword list_y, list_first)
{
	int i;
	int mx = mouse.x;
	int my = mouse.y + list_first;
	if (!id.count) return false;

	//Here we check is any link hovered
	for (i=0; i<id.count; i++)
	{
		if (mx>x.get(i)) && (my>y.get(i)) 
		&& (mx<x.get(i)+w.get(i)) && (my<y.get(i)+h.get(i))
		&& (my>list_y+list_first)
		{
			if (active!=i) {
				CursorPointer.Load(#CursorFile);
				CursorPointer.Set();

				draw_underline(active, list_first, list_y, link_color_default);			
				draw_underline(i, list_first, list_y, DEFAULT_BG_COL);

				active_url = element_links.get(i);
				active = i;
				DrawStatusBar(active_url);
			}
			if (mouse.lkm) && (mouse.down) {
				DrawRectangle(x.get(active), -list_first + y.get(active), 
					w.get(active), h.get(active), 0);
			}
			return true;
		}
	}
	if (active_url) {
		CursorPointer.Restore();
		draw_underline(active, list_first, list_y, link_color_default);
		active_url = 0;
		active = -1;
		DrawStatusBar(NULL);
	}
	return false;
}

