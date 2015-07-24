//list_box
#ifndef INCLUDE_LIST_BOX_H
#define INCLUDE_LIST_BOX_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

struct llist
{
	int x, y, w, h, min_h, line_h, text_y;
	int column_max;
	int count, visible, first, current; //visible = row_max
	int active;
	void ClearList();
	int ProcessKey(dword key);
	int MouseOver(int xx, yy);
	int ProcessMouse(int xx, yy);
	int KeyDown();
	int KeyUp();
	int KeyHome();
	int KeyEnd();
	void SetSizes(int xx, yy, ww, hh, min_hh, line_hh);
	int MouseScroll(dword scroll_state);
	int MouseScrollNoSelection(dword scroll_state);
	void debug_values();
}; 


void llist::debug_values()
{
	debug("current: ");
	debugi(current);
	debug("first: ");
	debugi(first);
	debug("visible: ");
	debugi(visible);
	debug("count: ");
	debugi(count);
}



void llist::ClearList()
{
	count = visible = first = current = 0;
}


void llist::SetSizes(int xx, yy, ww, hh, min_hh, line_hh)
{
	x = xx;
	y = yy;
	w = ww;
	h = hh;
	min_h = min_hh;
	line_h = line_hh;
	text_y = line_hh / 2 - 4;
	visible = h / line_h;
	column_max = w / 6;
	//if (visible > count) visible=count;
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
		case ASCII_KEY_DOWN: return KeyDown();
		case ASCII_KEY_UP:   return KeyUp();
		case ASCII_KEY_HOME: return KeyHome();
		case ASCII_KEY_END:  return KeyEnd();
	}
	return 0;
}

int llist::KeyDown()
{
	if (current-first+1<visible)
	{
		if (current+1>=count) return 0;
		current++;
	}
	else 
	{
		if (visible+first>=count) return 0;
		first++;
		current++;
	}
	return 1;
}

int llist::KeyUp()
{
	if (current>first) 
	{
		current--;
	}
	else
	{
		if (first==0) return 0;
		first--;
		current--;
	}
	return 1;
}

int llist::KeyHome()
{
	if (current==0) && (first==0) return 0;
	current=0;
	first=0;
	return 1;
}

int llist::KeyEnd()
{
	if (current==count-1) && (first==count-visible) return 0;
	current=count-1;
	first=count-visible;
	return 1;
}

#endif