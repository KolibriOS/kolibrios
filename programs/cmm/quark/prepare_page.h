
void ParseAndPaint()
{
	Parse();
	DrawPage();
}

void Parse()
{
dword ptr;
int line_end;

	list.count=0;
	selection.cancel();
	if (list.w != canvas.bufw) canvas.Init(list.x, list.y, list.w, screen.h);

	lines.drop();
	lines.add(textbuf.p);

	for (ptr = textbuf.p;    ptr < textbuf.p + textbuf.len;    ptr++)
	{
		if (ptr - lines.get_last() * list.font_w + 16 >= list.w)
		{
			//searching a 'white' for a normal word-break
			for(line_end = ptr; line_end != lines.get_last(); line_end--) 			{
				if (__isWhite(ESBYTE[line_end])) { 
					ptr = line_end + 1; 
					break;
				}
			}
			list.count++;
			lines.add(ptr);
		} else if (ESBYTE[ptr] == '\x0D') {
			if (ESBYTE[ptr+1] == '\x0A') ptr++;
			list.count++;
			lines.add(ptr+1);
		} else if (ESBYTE[ptr] == '\x0A') {
			list.count++;
			lines.add(ptr+1);
		}
	}
	lines.add(ptr);
	list.count++;
}

void DrawPage()
{
	char t[64];
	scroll.max_area = list.count;
	scroll.cur_area = list.visible;
	scroll.position = list.first;
	scroll.all_redraw = 0;
	scroll.start_x = list.x + list.w;
	scroll.start_y = list.y;
	scroll.size_y = list.h;

	if (list.count <= list.visible) {
		DrawBar(scroll.start_x, scroll.start_y, scroll.size_x,
		scroll.size_y, theme.bg);
	} else {
		scrollbar_v_draw(#scroll);
	}
	DrawRectangle(scroll.start_x, scroll.start_y, scroll.size_x,
		scroll.size_y-1, scroll.bckg_col);

	PaintVisible();

	sprintf(#t, #chars_selected, math.abs(selection.end_offset - selection.start_offset));
	if (selection.is_active()) DrawStatusBar(#t); else DrawStatusBar(" ");
}

void PaintVisible()
{
	int i, ff;
	signed s1, s2;
	dword ydraw, absolute_y;
	dword line_bg;
	bool swapped_selection = false;

	list.column_max = lines.len(list.cur_y);
	list.CheckDoesValuesOkey();
	if (selection.end_offset < selection.start_offset) {
		swapped_selection = selection.swap_start_end();
	}

	for ( i=0; i < list.visible+1; i++)
	{
		ydraw = i * list.item_h;
		absolute_y = i + list.first;
		line_bg = theme.bg;

		if (selection.start_y < absolute_y) && (selection.end_y > absolute_y) line_bg = selection.color;
		canvas.DrawBar(0, ydraw, list.w, list.item_h, line_bg);

		selection.draw(absolute_y);

		if (search.visible) || (search_next) for (ff=0; ff<search.found.count; ff++) {
			s1 = search.found.get(ff) - lines.get(absolute_y);
			s2 = search.found.get(ff) - lines.get(absolute_y+1);

			if (s2 > 0) break;

			if (s1 > 0) && (s2 < 0) {
				canvas.DrawBar(search.found.get(ff) - lines.get(absolute_y) * list.font_w + 3,
					ydraw, strlen(#found_text) * list.font_w, list.item_h, theme.found);
				search_next = false;
			}
		}

		if (absolute_y<list.count) canvas.WriteText(3, ydraw+3, list.font_type, theme.text,
			lines.get(absolute_y), lines.len(absolute_y));
	}

	PutPaletteImage(buf_data+8, canvas.bufw, list.h, list.x, list.y, 32, 0);

	if (swapped_selection) selection.swap_start_end();
}
