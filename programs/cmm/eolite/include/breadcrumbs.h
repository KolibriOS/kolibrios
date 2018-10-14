
PathShow_data PathShow = {0, 17,250, 6, 250, 0, 0, 0x0, 0xFFFfff, #path, #temp, 0};
void DrawPathBar()
{
	PathShow.area_size_x = Form.cwidth-300;
	DrawBar(PathShow.start_x-3, PathShow.start_y-6, PathShow.area_size_x+3, 19, 0xFFFfff);
	DrawRectangle(PathShow.start_x-4,PathShow.start_y-7,PathShow.area_size_x+4,20,col_graph);
	DefineHiddenButton(PathShow.start_x-4+1,PathShow.start_y-7+1,PathShow.area_size_x+4-2,20-2,PATH_BTN);
	DrawBar(PathShow.start_x-4, PathShow.start_y+14, PathShow.area_size_x+5+18, 1, system.color.work_light);

	DrawFlatButtonSmall(PathShow.start_x+PathShow.area_size_x,PathShow.start_y-7,18,20, 61, "\26");

	PathShow_prepare stdcall(#PathShow);
	PathShow_draw stdcall(#PathShow);
}

void DrawBreadCrumbs()
 collection_int breadCrumb;
 char PathShow_path[4096];
 block btn;
 int i;
 unsigned text_line, area_w;
 {
	strcat(#PathShow_path, #path);
	for (i=0; i<50; i++) DeleteButton(i+BREADCRUMB_ID);
	breadCrumb.drop();
	for (i=0; PathShow_path[i]; i++) 
	{
		if (PathShow_path[i]=='/') {
			PathShow_path[i] = NULL;
			breadCrumb.add(i+1);
		}
	}
	breadCrumb.add(i+1);
	btn.set_size(246,10,NULL,20);
	area_w = Form.cwidth - btn.x - 20;
	for (i=0; i<breadCrumb.count-1; i++)
	{
		text_line = breadCrumb.get(i) + #PathShow_path;
		btn.w = strlen(text_line)*8+10;
		DrawBreadcrumbButton(btn.x, btn.y, btn.w, btn.h, i+BREADCRUMB_ID, text_line);
		btn.x += btn.w;
	}
	//DrawFavButton(btn.x);
	//btn.x+=20;
	btn.x++;
	DrawBar(btn.x,btn.y-1,Form.cwidth-btn.x-25,btn.h+3,col_work);
}


void ClickOnBreadCrumb(unsigned clickid)
{
	int i, slashpos = #path;
	for (i=0; i!=clickid+2; i++) {
		slashpos=strchr(slashpos,'/')+1;
	}
	ESBYTE[slashpos-1] = NULL;
	Open_Dir(#path,WITH_REDRAW);
}


dword col_palette_br[14] = {0xFFFfff,0xF3EDF0,0xF3EDF0,0xF3EDF0,0xF3EDF0,
	0xF2EBEF,0xF2EBEF,
	0xF0EAEC,0xEFE9EB,0xEDE6E9,0xEAE6E8,0xE9E4E6,0xE5E0E3,0xE3DDDE,0xE0DBDB,
	0xDFDADA,0xDBD9DA,0xD2D0D2,0xC2C6C6};
void DrawBreadcrumbButton(dword x,y,w,h,id,text)
{
	int i;
	DrawRectangle(x,y,w,h,col_graph);
	for (i=0; i<h-1; i++) DrawBar(x+1, y+i+1, w-1, 1, col_palette_br[i]);
	DefineHiddenButton(x+1,y+1,w-2,h-2,id);
	WriteText(-strlen(text)*8+w/2+x,h/2+y-7,0x90,0x444444,text);
	DrawBar(x, y+h+1, w+1, 1, MixColors(col_work,0xFFFfff,120));
}