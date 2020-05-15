
void ParseAndPaint()
{
	//search.clear();
	list.KeyHome();
	list.count=0;

	Parse();

	list.visible = list.h / list.item_h;
	if (list.count < list.visible) {
		DrawBuf.bufh = list.visible;
	} else {
		DrawBuf.bufh = list.count;
	}

	DrawBuf.Init(list.x, list.y, list.w, DrawBuf.bufh+1*list.item_h);
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
	selection.cancel();

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
	dword y;
	dword line_bg;
	bool swapped = false;

	list.column_max = lines.get(list.cur_y+1) - lines.get(list.cur_y);
	list.CheckDoesValuesOkey();
	if (selection.end_offset < selection.start_offset) {
		swapped = selection.swap_start_end();
	}

	for ( i=list.first; i < list.first+list.visible+1; i++)
	{
		y = i * list.item_h;
		line_bg = theme.bg;

		if (selection.start_y < i) && (selection.end_y > i) line_bg = selection.color;
		DrawBuf.DrawBar(0, y, list.w, list.item_h, line_bg);

		selection.draw(i);

		if (i<list.count) DrawBuf.WriteText(3, y+3, list.font_type, theme.text, 
			lines.get(i), lines.get(i+1) - lines.get(i));
	}

	PutPaletteImage(list.first * DrawBuf.bufw * list.item_h * 4 + buf_data+8, 
		DrawBuf.bufw, list.h, list.x, list.y, 32, 0);

	if (swapped) selection.swap_start_end();
}

