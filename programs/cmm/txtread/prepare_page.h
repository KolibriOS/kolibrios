void ParseAndPaint()
{
	dword start_time = GetStartTime();
	search.clear();
	Parse();
	Paint();
	debugln("\nTextRead statistics in miliseconds...");
	debugval("Page generate time", GetStartTime() - start_time);
	if (list.count > list.visible * 10) DrawPage();
	start_time = GetStartTime();
	kfont.ApplySmooth();
	debugval("Smooth", GetStartTime() - start_time);
	DrawPage();
}


#define DRAW_PADDING 12

void Parse()
{
dword bufoff, buflen;
byte ch;
char line[4096]=0;
int srch_pos;
dword stroka_y=DRAW_PADDING-3;
dword line_length=30;
dword line_start=io.buffer_data;

	list.count=0;
	buflen = strlen(io.buffer_data) + io.buffer_data;
	for (bufoff=io.buffer_data; bufoff<buflen; bufoff++)
	{
		ch = ESBYTE[bufoff];
		line_length += kfont_char_width[ch];
		if (line_length>=list.w) || (ch==10) {
			srch_pos = bufoff;
			loop()
			{
				if (__isWhite(ESBYTE[srch_pos])) { bufoff=srch_pos+1; break; } //normal word-break
				if (srch_pos == line_start) break; //no white space found in whole line
				srch_pos--;
			}
			list.count++;
			strlcpy(#line, line_start, bufoff-line_start);
			search.add(stroka_y, #line);
			stroka_y += list.item_h;
			line_start = bufoff;
			line_length = 30;
		}
	}
	list.count+=2;
	list.visible = list.h / list.item_h;
	if (list.count < list.visible) list.count = list.visible;
	kfont.size.height = list.count+1*list.item_h;
	kfont.raw_size = 0;
	search.add(stroka_y, line_start);
}

void Paint()
{
	int i;
	int cur_pos;
	dword cur_line;
	for ( i=0; i < search.lines.count; i++)
	{
		cur_pos = atoi(search.pos.get(i));
		cur_line = search.lines.get(i);
		kfont.WriteIntoBuffer(DRAW_PADDING, cur_pos, list.w,
			kfont.size.height, bg_color, text_color, kfont.size.pt, cur_line);
	}
}

:void PaintVisible()
{
	int i;
	dword cur_pos;
	dword cur_line;
	for ( i=0; i < list.visible; i++)
	{
		cur_pos = atoi(search.pos.get(i + list.first));
		cur_line = search.lines.get(i + list.first);
		kfont.WriteIntoBuffer(DRAW_PADDING, cur_pos, list.w,
			kfont.size.height, bg_color, text_color, kfont.size.pt, cur_line);
	}
	kfont.ApplySmooth();
}