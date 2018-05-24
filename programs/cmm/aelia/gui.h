void DrawToolbarButton(dword image_id, x)
{
	DefineButton(x+1, 7, TOOLBAR_ICON_WIDTH-2, TOOLBAR_ICON_HEIGHT-2, 10+image_id + BT_HIDE, 0);
	DrawLibImage(skin.image, x, 6, TOOLBAR_ICON_WIDTH, TOOLBAR_ICON_HEIGHT, 0, image_id*TOOLBAR_ICON_HEIGHT);
}


void DrawScroller()
{
	scroll.max_area = list.count;
	scroll.cur_area = list.visible;
	scroll.position = list.first;
	scroll.all_redraw = 0;
	scroll.start_x = list.x + list.w;
	scroll.start_y = list.y;
	scroll.size_y = list.h;
	scroll.start_x = list.x + list.w;
	scrollbar_v_draw(#scroll);
}


dword MakePageWithHistory()
{
	int i;
	static dword history_page;

	if (history_page) free(history_page);
	history_page = malloc(history.items.data_size+256);
	strcat(history_page, "<html><head><title>History</title></head><body>\n<h1>History</h1>\n");
	strcat(history_page, "<h2>Visited pages</h2><br>\n");
	for (i=0; i<history.items.count; i++)
	{
		strcat(history_page, "<a href='");
		strcat(history_page, history.items.get(i));
		strcat(history_page, "'>");
		strcat(history_page, history.items.get(i));
		strcat(history_page, "</a><br>\n");
	}
	return history_page;
}

enum {
	STEP_1_DOWNLOAD_PAGE         =   0,
	STEP_2_COUNT_PAGE_HEIGHT     =  35,
	STEP_3_DRAW_PAGE_INTO_BUFFER =  60,
	STEP_4_SMOOTH_FONT           =  88,
	STEP_5_STOP                  = 100,
};

void DrawProgress(dword percent)
{
	dword progress_width;
	if (percent<100) {
		progress_width = address_box.width+5*percent/100;
		DrawBar(address_box.left-3, address_box.top+16, progress_width, 2, 0x72B7EA);
	}
	else {
		progress_width = address_box.width+5;
		DrawBar(address_box.left-3, address_box.top+16, progress_width, 2, 0xFFFfff);
	}
}
