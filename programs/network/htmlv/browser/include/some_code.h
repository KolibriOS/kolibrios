//кнопки
#define BACK        300
#define FORWARD     301
#define REFRESH     302
#define HOME        303
#define NEWTAB      304
#define GOTOURL     305
#define SEARCHWEB   306
#define ID1         178
#define ID2         177

#define _WIN  0
#define _DOS  1
#define _KOI  2
#define _UTF  3
                      

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
		do j++;
		while (page_links[j] <>'|');
	}
	page_links[j] = 0x00;
	strcpy(#URL, #page_links[strrchr(#page_links, '|')]);
}


//У нас нет наклонных шрифтов, поэтому делаем костыль из
//палочек для мороженого и жевательной резинки:
//Снимаем область экрана и выводим её обратно полосками со смещением,
//что даёт перекос картинки
//При наличии фона и т.п. проявится вся костыльность решения :)

inline void Skew(dword x,y,w,h)
{
	dword italic_buf;
	int tile_height=2, //будем выводить двухпиксельными полосками
	shift=-2, //с двухпиксельным смещением
	i, skin_height;

	italic_buf = mem_Alloc(w*h*3);
 	skin_height = GetSkinHeight();
	CopyScreen(italic_buf, x+Form.left+2, y+Form.top+skin_height, w, h);

	FOR (i=0;i*tile_height<h;i++)
		PutImage(w*3*tile_height*i+italic_buf,w,tile_height,x+shift-i+1,i*tile_height+y);
	
	mem_Free(italic_buf);
}
