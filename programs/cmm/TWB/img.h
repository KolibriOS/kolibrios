
struct _img
{
	collection url;
	collection_int xywh;
	collection_int data;
	int getid;
	dword add();
	void clear();
	dword current_url();
	bool next_url();
	void set_data();
	void draw();
};

#ifndef NO_IMG

dword _img::add(dword _path, _x, _y)
{
	char full_path[URL_SIZE];
	strncpy(#full_path, _path, URL_SIZE);
	GetAbsoluteURL(#full_path, history.current());		

	url.add(#full_path);
	xywh.add(_x);
	xywh.add(_y);
	xywh.add(0);
	xywh.add(0);
	return full_path;
}

void _img::clear()
{
	url.drop();
	xywh.drop();
	data.drop();
	getid = 0;
}

dword _img::current_url()
{
	return url.get(getid);
}

bool _img::next_url()
{
	if (getid < url.count-1) {
		getid++;
		return 1;
	}
	return 0;
}

void _img::set_data(dword _data, _data_len)
{
	data.set(getid, _data);
}

void _img::draw(int _x, _y, _start, _height)
{
	int i, img_x, img_y;

	for (i=0; i<url.count; i++) 
	{
		img_x = xywh.get(i*4);
		img_y = xywh.get(i*4 + 1);

		if (img_y > _start) && (img_y < _start + _height) 
		{
			if (cache.has(url.get(i)))
			DrawLibimgImage(img_x + _x, img_y-_start + _y, cache.current_buf, cache.current_size);
		}
	}
}

void DrawLibimgImage(dword _x, _y, _data, _data_len)
{
	libimg_image im;
	img_decode stdcall (_data, _data_len, 0);
	$or      eax, eax
	$jz      __ERROR__
	
	im.image = EAX;
	im.set_vars();
	im.draw(_x, _y, im.w, im.h, 0, 0);	
__ERROR__:
}

/*

void ImageCache::Images(dword left1, top1, width1)
{
	dword image;
    dword imgw=0, imgh=0, img_lines_first=0, cur_pic=0;
	
	//GetAbsoluteURL(#img_path);
	//cur_pic = GetImage(#img_path);

	if (!pics[cur_pic].image) 
	{
		//cur_pic = GetImage("/sys/network/noimg.png");
		return;
	}
	
	imgw = DSWORD[pics[cur_pic].image+4];
	imgh = DSWORD[pics[cur_pic].image+8];
	if (imgw > width1) imgw = width1;
	
	draw_y += imgh + 5; TEMPORARY TURN OFF!!!
	
	if (top1+imgh<WB1.list.y) || (top1>WB1.list.y+WB1.list.h-10) return; //if all image is out of visible area
	if (top1<WB1.list.y) //if image partly visible (at the top)
	{
		img_lines_first=WB1.list.y-top1;
		imgh=imgh-img_lines_first;
		top1=WB1.list.y;
	}
	if (top1>WB1.list.y+WB1.list.h-imgh-5) //if image partly visible (at the bottom)
	{
		imgh=WB1.list.y+WB1.list.h-top1-5;
	}	
	if (imgh<=0) return;
	
	img_draw stdcall (pics[cur_pic].image, left1-5, top1, imgw, imgh,0,img_lines_first);
	DrawBar(left1+imgw - 5, top1, WB1.list.w-imgw, imgh, page_bg);
	DrawBar(WB1.list.x, top1+imgh, WB1.list.w, -imgh % WB1.list.item_h + WB1.list.item_h, page_bg);
	if (link)
	{
		UnsafeDefineButton(left1 - 5, top1, imgw, imgh-1, links.count + 400 + BT_HIDE, 0xB5BFC9);
		links.AddText(0, imgw, imgh-1, NOLINE, 1);
		WB1.DrawPage();
	} 
}

ImageCache ImgCache;

*/

#else
dword _img::add(dword _path, _x, _y) {};
void _img::clear() {};
dword _img::current_url() {};
bool _img::next_url() {};
void _img::set_data(dword _data, _data_len) {};
void _img::draw(int _x, _y, _start, _height) {};

#endif