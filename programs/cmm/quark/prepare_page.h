
void ParseAndPaint()
{
	//search.clear();
	list.count=0;
	selection.cancel();

	Parse();

	list.visible = list.h / list.item_h;
	DrawBuf.Init(list.x, list.y, list.w, list.visible+1*list.item_h);
	DrawPage();	
}

void Parse()
{
dword off;
int line_end;
dword line_length=0;
dword line_start = io.buffer_data;
dword buflen = strlen(io.buffer_data) + io.buffer_data;

	lines.drop();
	lines.add(io.buffer_data);

	for (off = io.buffer_data; off < buflen; off++)
	{
		line_length += list.font_w;
		if (line_length + 30 >= list.w) || (ESBYTE[off] == 10)
		{
			//searching a 'white' for a normal word-break
			for(line_end = off; line_end != line_start; line_end--) 
			{
				if (__isWhite(ESBYTE[line_end])) { off=line_end+1; break; }
			}
			line_length = off - line_start * list.font_w;
			list.count++;
			lines.add(off);
			line_start = off;
			line_length = 0;
		}
	}
	lines.add(buflen);
	list.count++;
}

void PaintVisible()
{
	int i;
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
		DrawBuf.DrawBar(0, ydraw, list.w, list.item_h, line_bg);

		selection.draw(absolute_y);

		if (absolute_y<list.count) DrawBuf.WriteText(3, ydraw+3, list.font_type, theme.text, 
			lines.get(absolute_y), lines.len(absolute_y));
	}

	PutPaletteImage(buf_data+8, DrawBuf.bufw, list.h, list.x, list.y, 32, 0);

	if (swapped_selection) selection.swap_start_end();
}

