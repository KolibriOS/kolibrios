enum {
	COUNT_BUF_HEIGHT,
	DRAW_BUF
};

#define DRAW_PADDING 12

void Parcer(byte mode)
{
dword bufoff, buflen;
byte ch;
char line[4096]=0;
int srch_pos;
dword stroka_y=DRAW_PADDING-3;
dword line_length=30;
dword line_start=io.buffer_data;

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
			if (mode==COUNT_BUF_HEIGHT) {
				line_start = bufoff;
				line_length = 30;
				list.count++;
			}
			if (mode==DRAW_BUF) {
				EBX = bufoff-line_start;
				strlcpy(#line, line_start, EBX);
				kfont.WriteIntoBuffer(DRAW_PADDING,stroka_y,list.w,kfont.size.height, bg_color, text_color, kfont.size.pt, #line);
				stroka_y += list.item_h;
				line_start = bufoff;
				line_length = 30;
			}
		}
	}
	if (mode==COUNT_BUF_HEIGHT) list.count+=2;
	if (mode==DRAW_BUF) kfont.WriteIntoBuffer(DRAW_PADDING,stroka_y,list.w,kfont.size.height, bg_color, text_color, kfont.size.pt, line_start);
}

void PreparePage() 
{
	list.w = Form.cwidth-scroll.size_x-1;
	list.count=0;
	Parcer(COUNT_BUF_HEIGHT);
	
	//draw text in buffer
	list.SetSizes(0, TOOLBAR_H, list.w, Form.cheight-TOOLBAR_H, kfont.size.pt+6);
	if (list.count < list.visible) list.count = list.visible;
	kfont.size.height = list.count+1*list.item_h;
	kfont.raw_size = 0;
	Parcer(DRAW_BUF);

	if (list.count > list.visible * 10) DrawPage();
	//draw result
	kfont.ApplySmooth();
	DrawPage();
}
