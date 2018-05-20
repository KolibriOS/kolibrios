CustomCursor CursorPointer;
dword CursorFile = FROM "../TWB/pointer.cur";
#include "..\lib\collection.h"

#define NOLINE    0
#define UNDERLINE 1

#define MAXLINKS 400

struct array_link {
	dword link;
	int x,y,w,h;
	int underline, underline_h;
};

struct LinksArray {
	array_link links[MAXLINKS];
	collection page_links;
	int count;
	int active;
	bool HoverAndProceed();
	void AddLink();
	void AddText();
	dword GetURL();
	void Clear();
} PageLinks;

void LinksArray::AddLink(dword lpath)
{
	if (count>= MAXLINKS) return;
	page_links.add(lpath);
}

void LinksArray::AddText(dword _x, _y, _w, _h, _link_underline, _underline_h)
{
	if (count>= MAXLINKS) return;
	links[count].x = _x;
	links[count].y = _y;
	links[count].w = _w;
	links[count].h = _h;
	links[count].underline = _link_underline;
	links[count].underline_h = _underline_h;
	links[count].link = page_links.get(page_links.count-1);
	count++;
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

bool LinksArray::HoverAndProceed(dword mx, my)
{
	int i;
	for (i=0; i<count; i++)
	{
		if (mx>links[i].x) && (my>links[i].y) && (mx<links[i].x+links[i].w) && (my<links[i].y+links[i].h)
		{
			if (mouse.lkm) && (mouse.down) {
				DrawRectangle(links[active].x, -WB1.list.first + links[active].y, 
				links[active].w, links[active].h, 0);
				return false;
			}
			if (mouse.mkm) && (mouse.up) {
				open_in_a_new_window = true;
				ClickLink();
				return false;
			}
			if (mouse.lkm) && (mouse.up) { 
				CursorPointer.Restore();
				ClickLink();
				return false;
			}
			if (mouse.pkm) && (mouse.up) { 
				EventShowLinkMenu(mouse.x, mouse.y);
				return false;
			}
			if (active==i) return false;
			CursorPointer.Load(#CursorFile);
			CursorPointer.Set();
			if (links[active].underline) DrawBar(links[active].x, -WB1.list.first + links[active].y
				+ links[active].h, links[active].w, links[active].underline_h, link_color_inactive);
			if (links[i].underline) DrawBar(links[i].x, -WB1.list.first + links[i].y
				+ links[i].h, links[i].w, links[i].underline_h, bg_color);
			active = i;
			DrawStatusBar(links[active].link);
			return true;
		}
	}
	if (active!=-1)
	{
		CursorPointer.Restore();
		if (links[active].underline) {
			DrawBar(links[active].x, -WB1.list.first + links[active].y + links[active].h,links[active].w, 
				links[active].underline_h, link_color_inactive);
		}
		DrawBar(status_text.start_x, status_text.start_y, status_text.area_size_x, 9, col_bg);
		active = -1;
	}
}


