llist select_list;
scroll_bar scroll1 = { 18,200,398, 44,18,0,115,15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

?define T_SELECT_LIST_NO_DATA "No data to show"
?define SELECT_LIST_ITEMH 20

void SelectList_Init(dword _x, _y, _w, _h)
{
	select_list.SetFont(8, 14, 0x90);
	select_list.SetSizes(_x, _y, _w, _h, SELECT_LIST_ITEMH);
}

void SelectList_Draw()
{
	int i, list_last;

	select_list.CheckDoesValuesOkey();

	if (select_list.count > select_list.visible) list_last = select_list.visible; else list_last = select_list.count;

	for (i=0; i<list_last; i++;)
	{
		SelectList_DrawLine(i); //NEED TO BE IMPLEMENTED IN APP
	}
	DrawBar(select_list.x,i*select_list.item_h+select_list.y, select_list.w, -i*select_list.item_h+ select_list.h, 0xFFFfff);
	if (!select_list.count) WriteText(-15*8 + select_list.w / 2 + select_list.x + 1, 
		select_list.h / 2 - 8 + select_list.y, 0x90, 0x999999, T_SELECT_LIST_NO_DATA);
	SelectList_DrawScroller();
}

signed SelectList_ProcessMouse()
{
	mouse.get();
	scrollbar_v_mouse (#scroll1);
	if (select_list.first != scroll1.position)
	{
		select_list.first = scroll1.position;
		SelectList_Draw();
		return true;
	}
	
	if (mouse.vert) && (select_list.MouseScroll(mouse.vert)) {
		SelectList_Draw();
		return true;
	}

	if (mouse.up) && (mouse.click) 
		if (select_list.ProcessMouse(mouse.x, mouse.y)) {
			SelectList_LineChanged();
			return true;
		}
	return false;
}

void SelectList_DrawBorder() {
	DrawRectangle3D(select_list.x-2, select_list.y-2,
		select_list.w+3+scroll1.size_x, select_list.h+3, 
		sc.dark, sc.light);
	DrawRectangle(select_list.x-1, select_list.y-1, select_list.w+1+scroll1.size_x, select_list.h+1, sc.line);
}

void SelectList_DrawScroller()
{
	scroll1.bckg_col = MixColors(sc.work, 0xBBBbbb, 80);
	scroll1.frnt_col = MixColors(sc.work,0xFFFfff,120);
	scroll1.line_col = sc.line;

	scroll1.max_area = select_list.count;
	scroll1.cur_area = select_list.visible;
	scroll1.position = select_list.first;

	scroll1.all_redraw=1;
	scroll1.start_x = select_list.x + select_list.w;
	scroll1.start_y = select_list.y-1;
	scroll1.size_y = select_list.h+2;

	scrollbar_v_draw(#scroll1);
}
