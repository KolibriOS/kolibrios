struct s_image
{
	dword *image;
	char path[4096];
};

s_image pics[100]; //pics = mem_Alloc( 100*sizeof(s_image) );

struct ImageCache {
	int pics_count;
	void Free();
	int GetImage();
	void Images();
};

void ImageCache::Free()
{
	for ( ; pics_count>0; pics_count--)
	{
		if (pics[pics_count].image) img_destroy stdcall (pics[pics_count].image);
		pics[pics_count].path = NULL;
	}
}

int ImageCache::GetImage(dword i_path)
{
	int i;
	for (i=0; i<=pics_count; i++) if (!strcmp(#pics[i].path, i_path)) return i; //image exists
	// Load image and add it to Cache
	pics_count++;
	pics[pics_count].image = load_image(i_path);
	strcpy(#pics[pics_count].path, i_path);
	return pics_count;
}


void ImageCache::Images(dword left1, top1, width1)
{
	dword image;
    char img_path[4096], alt[4096]=0;
    dword imgw=0, imgh=0, img_lines_first=0, cur_pic=0;
	
	do{
		if (isattr("src="))
		{
			strcpy(#img_path, #val);
			GetAbsoluteURL(#img_path);
			cur_pic = GetImage(#img_path);
		}
		if (isattr("alt="))
		{
			strcpy(#alt, "[");
			strcat(#alt, #val);
			strcat(#alt, "]");
		}

	} while(GetNextParam());
	
	if (!pics[cur_pic].image) 
	{
		if (alt) strcat(#line, #alt);
		//cur_pic = GetImage("/sys/network/noimg.png");
		return;
	}
	
	imgw = DSWORD[pics[cur_pic].image+4];
	imgh = DSWORD[pics[cur_pic].image+8];
	if (imgw > width1) imgw = width1;
	
	draw_y += imgh + 5;
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
	DrawBar(left1+imgw - 5, top1, WB1.list.w-imgw, imgh, bg_color);
	DrawBar(WB1.list.x, top1+imgh, WB1.list.w, -imgh % WB1.list.item_h + WB1.list.item_h, bg_color);
	if (link)
	{
		//UnsafeDefineButton(left1 - 5, top1, imgw, imgh-1, PageLinks.count + 400 + BT_HIDE, 0xB5BFC9);
		//PageLinks.AddText(0, imgw, imgh-1, NOLINE, 1);
		//WB1.DrawPage();
	} 
}

ImageCache ImgCache;