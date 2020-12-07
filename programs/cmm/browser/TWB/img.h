
struct _img
{
	collection url;
	collection_int x,y,w,h;

	void clear();
	void add();
	
	void draw_all();
	bool draw();
};

void _img::clear()
{
	url.drop();
	x.drop();
	y.drop();
	w.drop();
	h.drop();
}

void _img::add(dword _path, _x, _y, _w, _h)
{
	url.add(_path);
	x.add(_x);
	y.add(_y);
	w.add(_w);
	h.add(_h);
}

/*
void _img::draw_all(int _x, _y, _w, _h, _start)
{
	int i, img_y;

	for (i=0; i<url.count; i++) 
	{
		img_y = y.get(i);

		if (img_y + h.get(i) > _start) && (img_y - _h < _start) 
		&& (cache.has(url.get(i))) draw(_x, _y, _w, _h, _start, i);
	}
}
*/

bool _img::draw(int _x, _y, _w, _h, _start, i)
{
	int img_x, img_y, img_w, img_h, invisible_h=0;
	img_decode stdcall (cache.current_buf, cache.current_size, 0);
	if (EAX) {
		EDI = EAX;

		img_x = x.get(i);
		img_y = y.get(i);
		img_w = math.min(w.set(i, ESDWORD[EDI+4]), _w - img_x);
		img_h = math.min(h.set(i, ESDWORD[EDI+8]), _h + _start - img_y);

		if (_start > img_y) {
			invisible_h = _start - img_y;
			img_y = _start;
		}

		img_draw stdcall(EDI, img_x + _x, img_y - _start + _y, img_w, img_h - invisible_h, 0, invisible_h);	
		free(EDI);
	}	
}

