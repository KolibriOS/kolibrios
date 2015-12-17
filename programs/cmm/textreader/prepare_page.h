void PreparePage() 
{
	char line[4096]=0;
	char char_width[255];
	dword line_start;
	byte ch;
	dword bufoff;
	dword line_length=30;
	dword stroka_y = 5;
	dword stroka=0;
	int i, srch_pos;
	
	font.changeSIZE();
	list.w = Form.cwidth-scroll.size_x-1;
	//get font chars width, need to increase performance
	for (i=0; i<256; i++) char_width[i] = font.symbol_size(i);
	//get font buffer height
	for (bufoff=io.buffer_data; ESBYTE[bufoff]; bufoff++)
	{
		ch = ESBYTE[bufoff];
		line_length += char_width[ch];
		if (line_length>=list.w) || (ch==10) {
			srch_pos = bufoff;
			loop()
			{
				if (__isWhite(ESBYTE[srch_pos])) { bufoff=srch_pos+1; break; } //normal word-break
				if (srch_pos == line_start) break; //no white space found in whole line
				srch_pos--;
			}
			line_start = bufoff;
			line_length = 30;
			stroka++;
		}
	}
	//draw text in buffer
	list.count = stroka+2;
	list.SetSizes(0, TOOLBAR_H, list.w, Form.cheight-TOOLBAR_H, font.size.text+1);
	if (list.count < list.visible) list.count = list.visible;

	font.size.height = list.count+1*list.item_h;
	font.buffer_size = 0;

	line_length = 30;
	line_start = io.buffer_data;
	for (bufoff=io.buffer_data; ESBYTE[bufoff]; bufoff++)
	{
		ch = ESBYTE[bufoff];
		line_length += char_width[ch];
		if (line_length>=list.w) || (ch==10)
		{
			//set word break
			srch_pos = bufoff;
			loop()
			{
				if (__isWhite(ESBYTE[srch_pos])) { bufoff=srch_pos+1; break; } //normal word-break
				if (srch_pos == line_start) break; //no white space found in whole line
				srch_pos--;
			}
			i = bufoff-line_start;
			strlcpy(#line, line_start, i);
			font.prepare_buf(8,stroka_y,list.w,font.size.height, #line);
			stroka_y += list.item_h;
			line_start = bufoff;
			line_length = 30;
		}
	}
	font.prepare_buf(8,stroka_y,list.w,font.size.height, line_start);
	SmoothFont(font.buffer, font.size.width, font.size.height);
	DrawPage();
}
