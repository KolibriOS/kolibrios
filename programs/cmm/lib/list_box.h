//list_box
#ifndef INCLUDE_LIST_BOX_H
#define INCLUDE_LIST_BOX_H
#print "[include <list_box.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

struct llist
{
	int x, y, w, h, line_h, text_y;
	dword font_w, font_h, font_type;
	int count, visible, first, current, column_max; //visible = row_max
	int active;
	void ClearList();
	int MouseOver(int xx, yy);
	int ProcessMouse(int xx, yy);
	int ProcessKey(dword key);
	int KeyDown();
	int KeyUp();
	int KeyHome();
	int KeyEnd();
	int KeyPgDown();
	int KeyPgUp();
	void CheckDoesValuesOkey();
	void SetSizes(int xx, yy, ww, hh, line_hh);
	void SetFont(dword font_ww, font_hh, font_tt);
	int MouseScroll(dword scroll_state);
	int MouseScrollNoSelection(dword scroll_state);
	void debug_values();
}; 


void llist::debug_values()
{
	char yi[128];
	sprintf(#yi, "%s %d %s %d %s %d %s %d", "current:", current, "first:", first,
	"visible:", visible, "count:", count);
	debugln(#yi);
}



void llist::ClearList()
{
	count = visible = first = current = 0;
}


void llist::SetSizes(int xx, yy, ww, hh, line_hh)
{
	x = xx;
	y = yy;
	w = ww;
	h = hh;
	line_h = line_hh;
	text_y = line_h - font_h / 2;
	visible = h / line_h;
	//if (visible > count) visible=count;
}

void llist::SetFont(dword font_ww, font_hh, font_tt)
{
	font_w = font_ww;
	font_h = font_hh;
	font_type = font_tt;
}


int llist::MouseScroll(dword scroll_state)
{
	if (count<=visible) return 0;
	if (scroll_state == 65535)
	{
		if (first == 0) return 0;
		if (first > 3) first -= 2; else first=0;
		return 1;
	} 
	if (scroll_state == 1)
	{
		if (visible + first == count) return 0;
		if (visible+first+3 > count) first = count - visible; else first+=2;
		return 1;
	}
	return 0;
}

int llist::MouseScrollNoSelection(dword scroll_state)
{
	if (count<=visible) return 0;
	if (scroll_state == 65535)
	{
		if (current == 0) return 0;
		if (current > 3) current -= 2; else current=0;
		return 1;
	} 
	if (scroll_state == 1)
	{
		if (visible + current == count) return 0;
		if (visible+current+3 > count) current = count - visible; else current+=2;
		return 1;
	}
	return 0;
}

int llist::MouseOver(int xx, yy)
{
	if (xx>x) && (xx<x+w) && (yy>y) && (yy<y+h) return 1;
	return 0;
}

int llist::ProcessMouse(int xx, yy)
{
	int current_temp;
	if (MouseOver(xx, yy))
	{
		current_temp = yy - y / line_h + first;
		if (current_temp != current) && (current_temp<count)
		{
			current = current_temp;
			return 1;
		}
	}
	return 0;
}

int llist::ProcessKey(dword key)
{
	switch(key)
	{
		case SCAN_CODE_DOWN: return KeyDown();
		case SCAN_CODE_UP:   return KeyUp();
		case SCAN_CODE_HOME: return KeyHome();
		case SCAN_CODE_END:  return KeyEnd();
		case SCAN_CODE_PGUP: return KeyPgUp();
		case SCAN_CODE_PGDN: return KeyPgDown();
	}
	return 0;
}

int llist::KeyDown()
{
	if (current-first+1<visible)
	{
		if (current + 1 >= count) return 0;
		current++;
	}
	else 
	{
		if (visible + first >= count) return 0;
		first++;
		current++;
	}
	if (current < first) || (current > first + visible)
	{
		first = current;
		CheckDoesValuesOkey();
	}
	return 1;
}

int llist::KeyUp()
{
	if (current > first) 
	{
		current--;
	}
	else
	{
		if (first == 0) return 0;
		first--;
		current--;
	}
	if (current < first) || (current > first + visible)
	{
		first = current;
		CheckDoesValuesOkey();
	}
	return 1;
}

int llist::KeyHome()
{
	if (current==0) && (first==0) return 0;
	current = first = 0;
	return 1;
}

int llist::KeyEnd()
{
	if (current==count-1) && (first==count-visible) return 0;
	current = count-1;
	first = count - visible;
	return 1;
}

int llist::KeyPgUp()
{
	if (count <= visible) return KeyHome();
	if (first == 0) return 0;
	first -= visible;
	CheckDoesValuesOkey();
	return 1;
}

int llist::KeyPgDown()
{
	if (count <= visible) return KeyEnd();
	if (first == count - visible) return 0;
	first += visible;
	CheckDoesValuesOkey();
	return 1;
}

void llist::CheckDoesValuesOkey()
{
	if (first < 0) first = 0;
	if (visible + first > count) first = count - visible;
	if (current >= count) current = count - 1;
	if (current < 0) current = 0;
}

#endif