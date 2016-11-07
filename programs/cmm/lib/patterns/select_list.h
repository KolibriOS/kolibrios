llist select_list;
scroll_bar scroll1 = { 18,200,398, 44,18,0,115,15,0,0xeeeeee,0xD2CED0,0x555555,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
dword select_list_count_offset;

?define T_SELECT_LIST_NO_DATA "No data to show"

void SelectList_Init(dword _x, _y, _w, _h, _no_selection)
{
	select_list.no_selection = _no_selection;
	select_list.SetFont(8, 14, 0x90);
	select_list.SetSizes(_x, _y, _w, _h, 20);
}

void SelectList_Draw()
{
	int i, list_last;

	select_list.CheckDoesValuesOkey();

	if (select_list.count > select_list.visible) list_last = select_list.visible; else list_last = select_list.count;

	for (i=0; i<list_last; i++;)
	{
		DrawBar(select_list.x,i*select_list.item_h+select_list.y,select_list.w, select_list.item_h, 0xFFFfff);
		SelectList_DrawLine(i); //NEED TO BE IMPLEMENTED IN APP
	}
	DrawBar(select_list.x,i*select_list.item_h+select_list.y, select_list.w, -i*select_list.item_h+ select_list.h, 0xFFFfff);
	if (!select_list.count) WriteText(-strlen(T_SELECT_LIST_NO_DATA)*select_list.font_w + select_list.w / 2 + select_list.x + 1, 
		select_list.h / 2 - 8 + select_list.y, select_list.font_type, 0x999999, T_SELECT_LIST_NO_DATA);
	SelectList_DrawScroller();
}

void SelectList_ProcessMouse()
{
	int mouse_clicked;
	mouse.get();
	scrollbar_v_mouse (#scroll1);
	if (select_list.first != scroll1.position)
	{
		select_list.first = scroll1.position;
		SelectList_Draw();
	}
	
	if (mouse.vert) && (select_list.MouseScroll(mouse.vert)) SelectList_Draw();

	if (mouse.up)&&(mouse_clicked)
	{
		if (mouse.lkm) && (select_list.ProcessMouse(mouse.x, mouse.y)) SelectList_LineChanged();
		mouse_clicked=false;
	}
	else if (mouse.down)&&(mouse.lkm) && (select_list.MouseOver(mouse.x, mouse.y)) mouse_clicked=true;
}

void SelectList_DrawBorder() {
	DrawRectangle3D(select_list.x-2, select_list.y-2,
		select_list.w+3+scroll1.size_x, select_list.h+3, 
		MixColors(system.color.work, system.color.work_dark, 125), 
		system.color.work_light);
	DrawRectangle(select_list.x-1, select_list.y-1, select_list.w+1+scroll1.size_x, select_list.h+1, system.color.work_graph);
}

void SelectList_DrawScroller()
{
	scroll1.bckg_col = MixColors(system.color.work, 0xBBBbbb, 80);
	scroll1.frnt_col = MixColors(system.color.work,0xFFFfff,120);
	scroll1.line_col = system.color.work_graph;

	scroll1.max_area = select_list.count;
	scroll1.cur_area = select_list.visible;
	scroll1.position = select_list.first;

	scroll1.all_redraw=1;
	scroll1.start_x = select_list.x + select_list.w;
	scroll1.start_y = select_list.y-1;
	scroll1.size_y = select_list.h+2;

	scrollbar_v_draw(#scroll1);
}
