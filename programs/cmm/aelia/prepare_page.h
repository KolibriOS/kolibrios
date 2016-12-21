
void PreparePage() 
{
	list.SetSizes(0, TOOLBAR_H, Form.cwidth-scroll.size_x-1, Form.cheight-TOOLBAR_H, label.size.pt+2);
	strcpy(#title, history.current()+strrchr(history.current(),'/'));
	//get font chars width, need to increase performance
	get_label_symbols_size();
	ChangeCharset(charsets[encoding], "CP866", io.buffer_data);
	link.clear();
	if (strstri(io.buffer_data, "<html")==-1) {
		debugln("no <html> found");
		DrawProgress(STEP_2_COUNT_PAGE_HEIGHT);     ParceTxt(false);   //get page height to calculate buffer size
		DrawProgress(STEP_3_DRAW_PAGE_INTO_BUFFER); ParceTxt(true);    //draw text in buffer
	} else {
		debugln("<html> tag found");
		DrawProgress(STEP_2_COUNT_PAGE_HEIGHT);     ParceHtml(false);  //get page height to calculate buffer size
		// debugln("DONE STEP 2");
		DrawProgress(STEP_3_DRAW_PAGE_INTO_BUFFER); ParceHtml(true);   //draw text in buffer
		// debugln("DONE STEP 3");
	}
	strcat(#title, " - Aelia");
	DrawTitle(#title);
	DrawProgress(STEP_4_SMOOTH_FONT);           label.ApplySmooth();
	DrawProgress(STEP_5_STOP);                  DrawPage();
}

void ParceTxt(byte draw)
{
byte ch, zeroch=0;
dword bufoff, buflen, line_start, srch_pos;
int stroka_y=5, line_length=0;

	line_start=io.buffer_data;
	buflen = strlen(io.buffer_data) + io.buffer_data;
	for (bufoff=io.buffer_data; bufoff<buflen; bufoff++)
	{
		ch = ESBYTE[bufoff];
		line_length += char_width[ch];
		if (line_length>=list.w-30) || (ch==10) {
			srch_pos = bufoff;
			loop()
			{
				if (__isWhite(ESBYTE[srch_pos])) { bufoff=srch_pos+1; break; } //normal word-break
				if (srch_pos == line_start) break; //no white space found in whole line
				srch_pos--;
			}
			if (draw==true) {
				ESBYTE[bufoff] >< zeroch; //set line end
				WriteTextIntoBuf(8, stroka_y, 0x000000, line_start);
				ESBYTE[bufoff] >< zeroch; //restore line
			}
			stroka_y += list.item_h;
			line_start = bufoff;
			line_length = 0;
		}
	}
	if (draw==false) {
		list.count = stroka_y/list.item_h+3;
		if (list.count < list.visible) list.count = list.visible;
		label.size.height = list.count+5*list.item_h;
		label.raw_size = 0;
	} 
	if (draw==true) WriteTextIntoBuf(8, stroka_y, 0x000000, line_start);
}


/*========================================================
=                                                        =
=                        HTML                            =
=                                                        =
========================================================*/

#define HTML_PADDING_X 8;
#define HTML_PADDING_Y 5;


void ParceHtml(byte draw)
{	
dword DOM_start, DOM_end, DOM_len, DOM_pos, aux2;
int stroka_x = HTML_PADDING_X;
int stroka_y = HTML_PADDING_Y;
int size_pt_change = 0;
dword line_break;
byte ch, zeroch;
_text text;
_tag tag;
	tag.clear();
	style.clear();
	/* Create DOM */
	debugln("creating DOM");
	DOM_len = strlen(io.buffer_data);
	DOM_start = malloc(DOM_len);
	DOM_end = DOM_start + DOM_len;
	strlcpy(DOM_start, io.buffer_data, DOM_len);

	/* Parce DOM */
	debugln("starting DOM parce...");
	text.start = DOM_start;
	for (DOM_pos=DOM_start; DOM_pos<DOM_end; DOM_pos++)
	{
		if (ESBYTE[DOM_pos]==0x0D) || (ESBYTE[DOM_pos]==0x0A) ESBYTE[DOM_pos]=' ';
		ch = ESBYTE[DOM_pos];
		if (ch=='<') {
			ESBYTE[DOM_pos] = '\0';
			tag.start = DOM_pos + 1;
			if (style.ignore) continue;
			if (tag.nameis("title")) {
				strcpy(#title, text.start);
				continue;
			}
			strtrim(text.start);
			// try to change the special symbols that may appear
			text.fixSpecial(text.start);

			while (get_label_len(text.start) + stroka_x + 30 > list.w)
			{
				zeroch = 0;
				for (line_break=tag.start-1; line_break>text.start; line_break--;)
				{
					ESBYTE[line_break] >< zeroch; //set line end
					if (get_label_len(text.start) + stroka_x + 30 <= list.w) break;
					ESBYTE[line_break] >< zeroch; //restore line
				}
				if (draw==true) {
					if (style.a) {
						link.add(stroka_x,stroka_y + size_pt_change,get_label_len(text.start),list.item_h,text.start," ");
						label_draw_bar(stroka_x, stroka_y+label.size.pt+1, get_label_len(text.start), style.color);
					}
					WriteTextIntoBuf(stroka_x, stroka_y, style.color, text.start);
				}
				stroka_x+=char_width[' '];
				ESBYTE[line_break] >< zeroch; //restore line
				text.start = line_break;
				stroka_x = HTML_PADDING_X;
				stroka_y += list.item_h;
			}
			if (draw==true) {
				if (style.a) {
					link.add(stroka_x,stroka_y + size_pt_change,get_label_len(text.start),list.item_h,text.start," ");	
					label_draw_bar(stroka_x, stroka_y+label.size.pt+1, get_label_len(text.start), style.color);
				}
				WriteTextIntoBuf(stroka_x, stroka_y, style.color, text.start);
			}
			stroka_x+=char_width[' '];
			stroka_x += get_label_len(text.start);
		}
		if (ch=='>') {
			ESBYTE[DOM_pos] = '\0';
			text.start = DOM_pos + 1;
			tag.parce();
			if (tag.nameis("br")) 
				|| (tag.nameis("p")) 
				|| (tag.nameis("div")) 
				|| (tag.nameis("tr")) {
				stroka_y+= list.item_h;
				stroka_x = HTML_PADDING_X;
				continue;
			}
			if 	(tag.nameis("h1")) || (tag.nameis("/h1")) ||
				(tag.nameis("h2")) || (tag.nameis("/h2")) ||
				(tag.nameis("h3")) || (tag.nameis("/h3")) {
				if (tag.nameis("h1")) {
					size_pt_change = 8;
				} else if (tag.nameis("/h1")) {
					size_pt_change = -8;
				} else if (tag.nameis("h2")) {
					size_pt_change = 6;
				} else if (tag.nameis("/h2")) {
					size_pt_change = -6;
				} else if (tag.nameis("h3")) {
					size_pt_change = 4;
				} else if (tag.nameis("/h3")) {
					size_pt_change = -4;
				}
				label.size.pt += size_pt_change;
				get_label_symbols_size();
				if (size_pt_change > 0) {
					stroka_y+= list.item_h;//что если будет очень длинная строка в теге?
				} else {//коммент выше и коммент ниже связаны
					stroka_y+= list.item_h - size_pt_change;//не очень понятна логика этого места
					size_pt_change = 0;
				}
				stroka_x = HTML_PADDING_X;
				continue;					
			}
			if (tag.nameis("script")) || (tag.nameis("style")) style.ignore = true;
			if (tag.nameis("/script")) || (tag.nameis("/style")) style.ignore = false;
			if (tag.nameis("a"))  { style.a = true;  style.color=0x0000FF; }
			if (tag.nameis("/a")) { style.a = false; style.color=0x000000; }
		}		
	}
	if (draw==false) {
		list.count = stroka_y/list.item_h+3;
		if (list.count < list.visible) list.count = list.visible;
		label.size.height = list.count+5*list.item_h;
		label.raw_size = 0;
	}
	free(DOM_start);
}