struct s_image
{
	dword *image;
	char path[4096];
};
s_image pics[100]; //pics = mem_Alloc( 100*sizeof(s_image) );
int num_of_pics;

int GetOrSetPicNum(dword i_path)
{
	int i;
	for (i=0; i<num_of_pics; i++)
	{
		if (!strcmp(#pics[i].path, i_path)) return i;
	}
	num_of_pics++;
	return num_of_pics;
}

void FreeImgCache()
{
	for ( ; num_of_pics>0; num_of_pics--)
	{
		if (pics[num_of_pics].image) img_destroy stdcall (pics[num_of_pics].image);
		pics[num_of_pics].path = NULL;
	}
}


void Images(int left1, top1, width1)
{
	dword image;
    char img_path[4096], alt[4096];
    int w=0, h=0, img_lines_first=0, cur_pic=0;
	
	do{
		if (!strcmp(#parametr,"src="))   //надо объединить с GetNewUrl()
		{
			if (downloader_id) strcpy(#img_path, #history_list[history_current-1].Item);
				else strcpy(#img_path, BrowserHistory.CurrentUrl()); //достаём адрес текущей страницы
			
			if (strcmpn(#img_path, "http:", 5)!=0) || (strcmpn(#options, "http:", 5)!=0)
			{
				img_path[strrchr(#img_path, '/')] = '\0'; //обрезаем её урл до последнего /
				strcat(#img_path, #options);
				
				cur_pic=GetOrSetPicNum(#img_path);
				if (!pics[cur_pic].path)
				{
					pics[cur_pic].image=load_image(#img_path);
					strcpy(#pics[cur_pic].path, #img_path);
				}
			}
		}
			if (!strcmp(#parametr,"alt="))
		{
			strcpy(#alt, "[");
			strcat(#alt, #options);
			strcat(#alt, "]");
		}

	} while(GetNextParam());
	
	if (!pics[cur_pic].image) 
	{
		if (alt) && (link) strcat(#line, #alt);
		return;
	}
	
	w = DSWORD[pics[cur_pic].image+4];
	h = DSWORD[pics[cur_pic].image+8];
	if (w > width1) w = width1;
	
	if (stroka==0) DrawBar(WB1.left, WB1.top, WB1.width-15, 5, bg_color); //закрашиваем первую строку
	stroka+=h/10;
	if (top1+h<WB1.top) || (top1>WB1.top+WB1.height-10) return; //если ВСЁ изображение ушло ВЕРХ или ВНИЗ
	if (top1<WB1.top) //если часть изображения сверху
	{
		img_lines_first=WB1.top-top1;
		h=h-img_lines_first;
		top1=WB1.top;
	}
	if (top1>WB1.top+WB1.height-h-5) //если часть изображения снизу
	{
		h=WB1.top+WB1.height-top1-5;
	}	
	if (h<=0) return;
	if (anchor) return;
	
	img_draw stdcall (pics[cur_pic].image, left1-5, top1, w, h,0,img_lines_first);
	DrawBar(left1+w - 5, top1, WB1.width-w, h, bg_color);
	IF (link) UnsafeDefineButton(left1 - 5, top1, w, h-1, blink + BT_HIDE, 0xB5BFC9);
}