
enum { BACK=300, FORWARD, REFRESH, HOME, NEWTAB, GOTOURL, SEARCHWEB, INPUT_CH, INPUT_BT };
enum { _WIN, _DOS, _KOI, _UTF };

#define ID1         178
#define ID2         177

                      

dword get_URL_part(int len) {
	char temp1[sizeof(URL)];
	strcpy(#temp1, #URL);
	temp1[len] = 0x00;
	return #temp1;
}

inline byte chTag(dword text) {return strcmp(#tag,text);}


void GetURLfromPageLinks(int id)
{
	int i, j = 0;
	for (i = 0; i <= id - 401; i++)
	{
		do
		{
			j++;
			if (j>=strlen(#page_links)) return;
		}
		while (page_links[j] <>'|');
	}
	page_links[j] = 0x00;
	strcpy(#URL, #page_links+strrchr(#page_links, '|'));
}


//У нас нет наклонных шрифтов, поэтому делаем костыль из
//палочек для мороженого и жевательной резинки:
//Снимаем область экрана и выводим её обратно полосками со смещением,
//что даёт перекос картинки
//При наличии фона и т.п. проявится вся костыльность решения :)

inline void Skew(dword x,y,w,h)
{
	dword italic_buf;
	int tile_height=2,
	shift=-2,
	i, skin_height;

	italic_buf = mem_Alloc(w*h*3);
 	skin_height = GetSkinHeight();
	CopyScreen(italic_buf, x+Form.left+2, y+Form.top+skin_height, w, h);

	FOR (i=0;i*tile_height<h;i++)
		_PutImage(x+shift-i+1,i*tile_height+y, w,tile_height, w*3*tile_height*i+italic_buf);
	
	mem_Free(italic_buf);
}
