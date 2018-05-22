dword CursorFile = FROM "pointer.cur";

struct _link
{
	CustomCursor CursorPointer;
	int count;
	int x[4096], y[4096], w[4096], h[4096];
	collection text;
	collection url;
	void clear();
	void add();
	int hover();
	int active;
	void draw_underline();
} link;

void _link::clear()
{
	text.drop();
	url.drop();
	count = 0;
}

void _link::add(int _xx, _yy, _ww, _hh, dword _textt, _urll )
{
	if (count==4095) return;
	x[count] = _xx;
	y[count] = _yy;
	w[count] = _ww;
	h[count] = _hh;
	text.add(_textt);
	url.add(_urll);
	count++;
}

void _link::draw_underline(dword i, color)
{
	DrawBar(x[i]+list.x, -list.first*list.item_h+y[i]+list.y+h[i]-1, w[i], 1, color);
}

int _link::hover()
{
	int i;
	int new_active = -1;
	int link_start_y = list.first*list.item_h;
	mouse.x = mouse.x - list.x;
	mouse.y = mouse.y - list.y;
	for (i=0; i<count; i++) {
		if(y[i] > link_start_y) && (y[i] < link_start_y+list.h) {
			// debugln( sprintf(#param, "mx:%i my:%i x[i]:%i y[i]:%i", mx, my, x[i], y[i]) );
			if (mouse.x > x[i]) 
			&& (mouse.x < x[i]+w[i]) 
			&& (mouse.y > y[i]-link_start_y)
			&& (mouse.y < h[i]-link_start_y+link.y[i]) {
				new_active = i;
				break;
			}
		}
	}

	if (new_active != active) 
	{
		if (-1 == new_active) {
			draw_underline(active, 0x0000FF);
			CursorPointer.Restore();		
		}
		else {
			draw_underline(active, 0x0000FF);
			draw_underline(new_active, 0xFFFfff);
			CursorPointer.Load(#CursorFile);
			CursorPointer.Set();
		}
		active = new_active;
		return true;
	}

	return false;
}