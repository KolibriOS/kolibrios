#ifndef INCLUDE_LIST_BOX_H
#define INCLUDE_LIST_BOX_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

struct llist
{
	int x, y, w, h, item_h, item_w;
	int count, visible, first, column_max; //visible = row_max
	int cur_x, cur_y;
	int text_y;
	byte font_w, font_h, font_type;
	byte wheel_size;
	byte active;
	byte no_selection;
	byte horisontal_selelection;
	void ClearList();
	void SetSizes(int xx, yy, ww, hh, item_hh);
	void SetFont(dword font_ww, font_hh, font_tt);
	int ProcessKey(dword key);
	int ProcessMouse(int xx, yy);
	int MouseOver(int xx, yy);
	int MouseScroll(dword scroll_state);
	int KeyDown(); 
	int KeyUp(); 
	int KeyHome(); 
	int KeyEnd(); 
	int KeyPgDown(); 
	int KeyPgUp(); 
	int KeyLeft(); 
	int KeyRight();
	void CheckDoesValuesOkey();
	void debug();
};

:void llist::debug()
{
	char yi[128];
	sprintf(#yi, "%s %d %s %d %s %d %s %d %s %d %s %d", "first:", first, "visible:", visible, "count:", count, "col_max:", column_max, "cur_y:", cur_y, "cur_x:", cur_x);
	debugln(#yi);
}


:void llist::ClearList()
{
	count = visible = first = cur_y = cur_x = 0;
}


:void llist::SetSizes(int xx, yy, ww, hh, item_hh)
{
	x = xx;
	y = yy;
	w = ww;
	h = hh;
	item_h = item_hh;
	text_y = item_h - font_h / 2;
	visible = h / item_h;
	wheel_size = 3;
	CheckDoesValuesOkey();
}


:void llist::SetFont(dword font_ww, font_hh, font_tt)
{
	font_w = font_ww;
	font_h = font_hh;
	font_type = font_tt;
}


:int llist::MouseScroll(dword scroll_state)
{
	if (count<=visible) return 0;
	if (scroll_state == 65535)
	{
		if (first == 0) return 0;
		if (first > wheel_size+1) first -= wheel_size; else first=0;
		return 1;
	} 
	if (scroll_state == 1)
	{
		if (visible + first == count) return 0;
		if (visible+first+wheel_size+1 > count) first = count - visible; else first+=wheel_size;
		return 1;
	}
	return 0;
}


:int llist::MouseOver(int xx, yy)
{
	if (xx>x) && (xx<x+w) && (yy>y) && (yy<y+h) return 1;
	return 0;
}

:int llist::ProcessMouse(int xx, yy)
{
	int cur_y_temp, cur_x_temp, ret=0;
	if (MouseOver(xx, yy))
	{
		cur_y_temp = yy - y / item_h + first;
		if (cur_y_temp != cur_y) && (cur_y_temp<count)
		{
			cur_y = cur_y_temp;
			ret = 1;
		}
		if (horisontal_selelection) 
		{		
			cur_x_temp = xx - x / item_w;
			if (cur_x_temp != cur_x) && (cur_x_temp<column_max)
			{
				cur_x = cur_x_temp;
				ret = 1;
			}
		}
	}
	return ret;
}

:int llist::ProcessKey(dword key)
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
	if (horisontal_selelection) switch(key)
	{
		case SCAN_CODE_LEFT:  return KeyLeft();
		case SCAN_CODE_RIGHT: return KeyRight();
	}
	return 0;
}

:int llist::KeyDown()
{
	if (no_selection)
	{
		if (visible + first >= count) return 0;
		first++;
		return 1;		
	}

	if (cur_y-first+1<visible)
	{
		if (cur_y + 1 >= count) return 0;
		cur_y++;
	}
	else 
	{
		if (visible + first >= count) return 0;
		first++;
		cur_y++;
	}
	if (cur_y < first) || (cur_y > first + visible)
	{
		first = cur_y;
		CheckDoesValuesOkey();
	}
	return 1;
}

:int llist::KeyUp()
{
	if (no_selection)
	{
		if (first == 0) return 0;
		first--;
		return 1;
	}

	if (cur_y > first)
	{
		cur_y--;
	}
	else
	{
		if (first == 0) return 0;
		first--;
		cur_y--;
	}
	if (cur_y < first) || (cur_y > first + visible)
	{
		first = cur_y;
		CheckDoesValuesOkey();
	}
	return 1;
}

:int llist::KeyHome()
{
	if (cur_y==0) && (first==0) return 0;
	cur_y = first = 0;
	return 1;
}

:int llist::KeyEnd()
{
	if (cur_y==count-1) && (first==count-visible) return 0;
	cur_y = count-1;
	first = count - visible;
	return 1;
}

:int llist::KeyPgUp()
{
	if (count <= visible) return KeyHome();
	if (first == 0) return 0;
	first -= visible;
	cur_y = first;
	CheckDoesValuesOkey();
	return 1;
}

:int llist::KeyPgDown()
{
	if (count <= visible) return KeyEnd();
	if (first == count - visible) return 0;
	first += visible;
	cur_y = first + visible - 1;
	CheckDoesValuesOkey();
	return 1;
}

:void llist::CheckDoesValuesOkey()
{
	if (visible + first > count) first = count - visible;
	if (first < 0) first = 0;
	if (cur_y >= count) cur_y = count - 1;
	if (cur_y < 0) cur_y = 0;
	if (cur_x < 0) cur_x = 0;
}

:int llist::KeyRight()
{
	if (cur_x < column_max)
	{
		cur_x++;
	}
	else 
	{
		if (!KeyDown()) return 0;
		cur_x = 0;
	}
	return 1;
}

:int llist::KeyLeft()
{
	if (cur_x > 0)
	{
		cur_x--;
	}
	else 
	{
		if (!KeyUp()) return 0;
		cur_x = column_max;
	}
	return 1;
}


:void llist_copy(dword dest, src)
{
	EDI = dest;
	ESI = src;
	EDI.llist.x = ESI.llist.x;
	EDI.llist.y = ESI.llist.y;
	EDI.llist.w = ESI.llist.w;
	EDI.llist.h = ESI.llist.h;
	EDI.llist.item_h = ESI.llist.item_h;
	EDI.llist.text_y = ESI.llist.text_y;
	EDI.llist.font_w = ESI.llist.font_w;
	EDI.llist.font_h = ESI.llist.font_h;
	EDI.llist.font_type = ESI.llist.font_type;
	EDI.llist.count = ESI.llist.count;
	EDI.llist.visible = ESI.llist.visible;
	EDI.llist.first = ESI.llist.first;
	EDI.llist.cur_y = ESI.llist.cur_y;
	EDI.llist.column_max = ESI.llist.column_max;
	EDI.llist.active = ESI.llist.active;
}

#endif