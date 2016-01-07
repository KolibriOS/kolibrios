char char_width[255];

void PreparePage() 
{
int i;
	list.SetSizes(0, TOOLBAR_H, Form.cwidth-scroll.size_x-1, Form.cheight-TOOLBAR_H, label.size.pt+1);
	//get font chars width, need to increase performance
	//if (strstri(io.buffer_data, "<html>")==-1) {
		debugln("no <html> found");
		label.changeSIZE();
		for (i=0; i<256; i++) char_width[i] = label.symbol_size(i);
		ChangeCharset(charsets[encoding], "CP866", io.buffer_data);
		DrawProgress(STEP_2_COUNT_PAGE_HEIGHT);     ParceTxt(false);        //get page height to calculate buffer size
		DrawProgress(STEP_3_DRAW_PAGE_INTO_BUFFER); ParceTxt(true);         //draw text in buffer
		DrawProgress(STEP_4_SMOOTH_FONT);           label.apply_smooth();
		DrawProgress(STEP_5_STOP);                  DrawPage();
	/*}
	else {
		debugln("<html> tag found");
		label.changeSIZE();
		for (i=0; i<256; i++) char_width[i] = label.symbol_size(i);
		ChangeCharset(charsets[encoding], "CP866", io.buffer_data);
		DrawProgress(STEP_2_COUNT_PAGE_HEIGHT);     ParceHtml(false);        //get page height to calculate buffer size
		DrawProgress(STEP_3_DRAW_PAGE_INTO_BUFFER); ParceHtml(true);         //draw text in buffer
		DrawProgress(STEP_4_SMOOTH_FONT);           label.apply_smooth();
		DrawProgress(STEP_5_STOP);                  DrawPage();
	}*/
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
				label.write_buf(8,stroka_y,list.w,label.size.height, 0xFFFFFF, 0, label.size.pt, line_start);
				ESBYTE[bufoff] >< zeroch; //restore line
				DrawProgressWhileDrawing(bufoff, buflen);
				if (stroka_y/list.item_h-list.first==list.visible) DrawPage();
			}
			stroka_y += list.item_h;
			line_start = bufoff;
			line_length = 0;
		}
	}
	if (draw==false) {
		list.count = stroka_y/list.item_h+2;
		if (list.count < list.visible) list.count = list.visible;
		label.size.height = list.count+1*list.item_h;
		label.raw_size = 0;
	} 
	if (draw==true) label.write_buf(8,stroka_y,list.w,label.size.height, 0xFFFFFF, 0, label.size.pt, line_start);
}


/*========================================================
=                                                        =
=                        HTML                            =
=                                                        =
========================================================*/

/*
HTML parcer tags:
<title>
<meta encoding>
<a hrf="">
<img src="" alt="">
<h1> ... <h6>
<b>
<u>
<s>
<pre>
*/

struct _DOM {
	dword start;
	dword end;
	dword len;
};

struct _style {
	bool b, u, i, s;
	bool h1, h2, h3, h4, h5, h6;
	bool a;
	bool pre;
	bool title;
	bool br;
} style;

struct _tag {
	dword start;
	dword name;
	dword param[10];
	dword value[10];
	void parce();
	void get_param_value();
};

void _tag::parce()
{
	bool closed_status = false;
	if (start) debugln(start);
	/*
	if (strncmp(start, "/", 1)==0) {
		start++;
		closed_status = true;
	}
	if (!strcmp(start, "title")) style.title = closed_status;
	if (!strcmp(start, "br")) style.br = closed_status;
	*/
}

struct _text {
	dword start;
	int x, y;
	void draw();	
};

void _text::draw()
{
	if (start) debugln(start);
	/*
	if (style.title) {
		strlcpy(#title, start, sizeof(title));
		DrawTitle(#title);
		return;
	}
	if (style.br) {
		y += list.item_h;
		style.br = false;
	}
	*/
}


void ParceHtml(byte draw)
{
byte ch;
_DOM DOM;
_text text;
_tag tag;
dword DOM_pos;

	/* Create DOM */
	debugln("starting DOM parce");
	DOM.len = strlen(io.buffer_data);
	DOM.start = malloc(DOM.len);
	DOM.end = DOM.start + DOM.len;
	strlcpy(DOM.start, io.buffer_data, DOM.len);

	/* Parce DOM */
	text.start = DOM_pos;
	for (DOM_pos=DOM.start; DOM_pos<DOM.end; DOM_pos++)
	{
		ch = ESBYTE[DOM_pos];
		if (ch=='<') {
			ESBYTE[DOM_pos] = NULL;
			tag.start = DOM_pos + 1;
			text.draw();
		}
		if (ch=='>') {
			ESBYTE[DOM_pos] = NULL;
			text.start = DOM_pos + 1;
			tag.parce();
		}		
	}
	free(DOM.start);
	ExitProcess();
}