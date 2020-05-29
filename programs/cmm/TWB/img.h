struct img
{
	collection src;
	collection_int data;
	collection_int xywh;
	void drop();
};

void img::drop()
{
	src.drop();
	data.drop();
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