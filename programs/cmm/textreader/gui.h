void DrawToolbarButton(char image_id, int x)
{
	DefineButton(x+1, 7, TOOLBAR_ICON_WIDTH-2, TOOLBAR_ICON_HEIGHT-2, 10+image_id + BT_HIDE, 0);
	img_draw stdcall(skin.image, x, 6, TOOLBAR_ICON_WIDTH, TOOLBAR_ICON_HEIGHT, 0, image_id*TOOLBAR_ICON_HEIGHT);
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