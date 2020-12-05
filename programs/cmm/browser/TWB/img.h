
struct _img
{
	collection url;
	collection_int xywh;
	int getid;

	void clear();
	dword add_pos();
	bool set_size();

	dword current_url();
	bool next_url();
	
	void draw_all();
	bool draw();
};

void _img::clear()
{
	url.drop();
	xywh.drop();
	getid = 0;
}

dword _img::add_pos(dword _path, _x, _y)
{
	char full_path[URL_SIZE];
	strncpy(#full_path, _path, URL_SIZE);
	get_absolute_url(#full_path, history.current());

	url.add(#full_path);
	xywh.add(_x);
	xywh.add(_y);
	xywh.add(NULL);
	xywh.add(NULL);
	return #full_path;
}

bool _img::set_size(dword _id, _buf, _size)
{
	img_decode stdcall (_buf, _size, 0);
	if (EAX) {
		EDI = EAX;
		xywh.set(_id*4+2, ESDWORD[EDI+4]);
		xywh.set(_id*4+3, ESDWORD[EDI+8]);
		free(EDI);
		return true;
	}	
	return false;
}

//DELTE!!!!!11111111111111111111111111111111111111
dword _img::current_url()
{
	return url.get(getid);
}

//DELTE!!!!!11111111111111111111111111111111111111
bool _img::next_url()
{
	if (getid < url.count-1) {
		getid++;
		return 1;
	}
	return 0;
}

void _img::draw_all(int _x, _y, _w, _h, _start)
{
	int i, img_y;

	for (i=0; i<url.count; i++) 
	{
		img_y = xywh.get(i*4 + 1);

		if (img_y > _start) && (img_y < _start + _h) 
		&& (cache.has(url.get(i))) draw(_x, _y, _w, _h, _start, i);
	}
}

bool _img::draw(int _x, _y, _w, _h, _start, i)
{
	int img_x, img_y, img_w, img_h;
	img_decode stdcall (cache.current_buf, cache.current_size, 0);
	if (EAX) {
		EDI = EAX;

		img_x = xywh.get(i*4+0);
		img_y = xywh.get(i*4+1);
		img_w = math.min(xywh.set(getid*4+2, ESDWORD[EDI+4]), _w - img_x);
		img_h = math.min(xywh.set(getid*4+3, ESDWORD[EDI+8]), _h + _start - img_y);


		img_draw stdcall(EDI, img_x + _x, img_y - _start + _y, img_w, img_h, 0, 0);	
		free(EDI);
	}	
}
