//list_box

struct llist
{
	int x, y, w, h, min_h, line_h;
	int count, visible, first, current;
	int current_temp;
	void ClearList();
	int KeyDown();
	int KeyUp();
	void SetSizes(int xx, yy, ww, hh, min_hh, line_hh);
	int MouseScroll(dword scroll_state);
}; 


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
	visible = h / line_h;
}


int llist::MouseScroll(dword scroll_state)
{
	if (scroll_state == 65535)
	{
		if (first == 0) return 0;
		if (first > 3) first -= 2; else first=0;
		return 1;
	} 
	if (scroll_state == 1)
	{
		if (visible+first+3 >= count) first = count - visible; else first+=2;
		return 1;
	}
	return 0;
}

int llist::KeyDown()
{
	if (current-first+1<visible)
	{
		if (current+1>=count) return -1;
		current++;
	}
	else 
	{
		if (visible+first>=count) return -1;
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
		if (first==0) return -1;
		first--;
		current--;
	}
	return 1;
}