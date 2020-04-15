CustomCursor CursorPointer;
dword CursorFile = FROM "../TWB/pointer.cur";
#include "..\lib\collection.h"

#define NOLINE    0
#define UNDERLINE 1

#define MAXLINKS 2000

bool open_new_window=false;
bool open_new_tab=false;

struct array_link {
	dword link;
	unsigned int x,y,w,h;
	unsigned int unic_id;
	int underline, underline_h;
};

struct LinksArray {
	array_link links[MAXLINKS];
	collection page_links;
	unsigned int count;
	unsigned int unic_count;
	unsigned int active;
	bool HoverAndProceed();
	void AddLink();
	void AddText();
	dword GetURL();
	void Clear();
	void DrawUnderline();
} PageLinks;

void LinksArray::AddLink(dword lpath)
{
	if (count>= MAXLINKS) return;
	page_links.add(lpath);
	unic_count++;
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
	links[count].unic_id = unic_count;
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
	unic_count = 0;
	CursorPointer.Restore();
	open_new_window = false;
}

void LinksArray::DrawUnderline(dword und_id, list_first, list_y, color)
{
	int i;
	for (i=0; i<count; i++) 
	{
		if (links[i].unic_id==links[und_id].unic_id) && (links[i].y + links[i].h - list_first > list_y) {
			DrawBar(links[i].x, links[i].y + links[i].h - list_first, links[i].w, links[i].underline_h, color);
		}		
	}
}

PathShow_data status_text = {0, 17,250, 6, 250};

bool LinksArray::HoverAndProceed(dword mx, my, list_y, list_first)
{
	int i;
	if (!count) return true;
	for (i=0; i<count; i++)
	{
		if (mx>links[i].x) && (my>links[i].y) 
		&& (mx<links[i].x+links[i].w) && (my<links[i].y+links[i].h)
		&& (my>list_y+list_first)
		{
			if (mouse.lkm) && (mouse.down) {
				DrawRectangle(links[active].x, -list_first + links[active].y, 
				links[active].w, links[active].h, 0);
				return false;
			}
			if (mouse.mkm) && (mouse.up) {
				if (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) {
					ProcessEvent(IN_NEW_TAB);
				} else {
					ProcessEvent(IN_NEW_WINDOW);
				}
				return false;
			}
			if (mouse.lkm) && (mouse.up) { 
				CursorPointer.Restore();
				EventClickLink(PageLinks.GetURL(PageLinks.active));
				return false;
			}
			if (mouse.pkm) && (mouse.up) { 
				EventShowLinkMenu();
				return false;
			}
			if (active==i) return false;
			CursorPointer.Load(#CursorFile);
			CursorPointer.Set();

			if (links[active].underline) {
				DrawUnderline(active, list_first, list_y, link_color_default);			
			}

			if (links[i].underline) {
				DrawUnderline(i, list_first, list_y, page_bg);
			}

			active = i;
			DrawStatusBar(links[active].link);
			return true;
		}
	}
	if (active!=-1)
	{
		CursorPointer.Restore();
		if (links[active].underline) {
			DrawUnderline(active, list_first, list_y, link_color_default);
		}
		DrawStatusBar(NULL);
		active = -1;
	}
}

