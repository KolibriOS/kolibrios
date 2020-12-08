
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

bool _img::draw(int _x, _y, _w, _h, _start, i)
{
	int img_x, img_y, img_w, img_h, invisible_h=0;
	char* img_ptr;

	img_x = x.get(i);
	img_y = y.get(i);
	img_w = math.min(w.get(i), _w - img_x);
	img_h = math.min(h.get(i), _h + _start - img_y);

	if (_start > img_y) {
		invisible_h = _start - img_y;
		img_y = _start;
	}

	img_decode stdcall (cache.current_buf, cache.current_size, 0);
	img_ptr = EAX;
	img_draw stdcall(img_ptr, img_x + _x, img_y - _start + _y, img_w, img_h - invisible_h, 0, invisible_h);	
	img_destroy stdcall(img_ptr);
}

