struct _link
{
	int count;
	int x[4096], y[4096], w[4096], h[4096];
	collection text;
	collection url;
	void clear();
	void add();
	int hover();
	int active;
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

int _link::hover()
{
	//char tempp[4096];
	dword color;
	int i;
	active = 0;
	mouse.x = mouse.x - list.x;
	mouse.y = mouse.y - list.y;
	for (i=0; i<link.count; i++) {
		if(link.y[i]>list.first*list.item_h) && (link.y[i]<list.first*list.item_h+list.h) {
			// sprintf(#tempp, "mx:%i my:%i x[i]:%i y[i]:%i", mx, my, x[i], y[i]);
			// sprintf(#tempp);
			if (mouse.x>link.x[i]) 
			&& (-list.first*list.item_h+link.y[i]<mouse.y)
			&& (mouse.x<link.x[i]+link.w[i]) 
			&& (-list.first*list.item_h+link.y[i]+link.h[i]>mouse.y) 
				color = 0xFF0000;
			else
				color = 0xCCCccc;
			DrawRectangle(link.x[i]+list.x+1, -list.first*list.item_h+link.y[i]+list.y, link.w[i], link.h[i], color);
		}
	}
	return false;
}