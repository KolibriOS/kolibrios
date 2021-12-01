CANVAS canvas;

void TWebBrowser::RenderLine(dword _line)
{
	unsigned px; //paint x coordinate
	unsigned pw; //paint y coordinate
	   dword pc; //paint color
	   int zoom;

	if (style.title)
	{
		strncpy(#header, _line, sizeof(TWebBrowser.header)-1);
		if (!application_mode) {
		    strncat(#header, " - ", sizeof(TWebBrowser.header)-1);
        	strncat(#header, #version, sizeof(TWebBrowser.header)-1);
		}
	}
	else if (t_html) && (!t_body) {
		//
	}
	else if (ESBYTE[_line])
	{
		pw = strlen(_line) * list.font_w;
		zoom = list.font_w / BASIC_CHAR_W;

		//there is some shit happens!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if (pw > draw_w) {
			//draw_w = pw;
			NewLine();
		}

		if (debug_mode) {
			canvas.DrawBar(draw_x, draw_y, pw, list.item_h, 0xCCCccc);
			debugln(_line);
		}

		style.cur_line_h = math.max(style.cur_line_h, list.item_h);

		if (!secondrun) goto _SKIP_DRAW;
		if (bg_colors.get_last() - bg_colors.get(0)) {
			canvas.DrawBar(draw_x, draw_y, pw, list.item_h, bg_colors.get_last());
		}

		if (style.image) {
			canvas.DrawBar(draw_x, draw_y, pw, list.item_h-1, 0xF9DBCB);
		}
		if (style.button) {
			canvas.DrawBar(draw_x, draw_y, pw, list.item_h - calc(zoom*2), 0xCCCccc);
			canvas.DrawBar(draw_x, draw_y + list.item_h - calc(zoom*2), pw, zoom, 0x999999);
		}

		pc = text_colors.get_last();
		if (link) && (pc == text_colors.get(0)) pc = link_color_default;

		canvas.WriteText(draw_x, draw_y, list.font_type, pc, _line, NULL);
		if (style.b) canvas.WriteText(draw_x+1, draw_y, list.font_type, pc, _line, NULL);
		if (style.s) canvas.DrawBar(draw_x, list.item_h / 2 - zoom + draw_y, pw, zoom, pc);
		if (style.u) canvas.DrawBar(draw_x, list.item_h - zoom - zoom + draw_y, pw, zoom, pc);
		if (link) {
			if (ESBYTE[_line]==' ') && (ESBYTE[_line+1]==NULL) {} else {
				canvas.DrawBar(draw_x, draw_y + list.item_h - calc(zoom*2)-1, pw, zoom, link_color_default);
				links.add_text(draw_x, draw_y + list.y, pw, list.item_h - calc(zoom*2)-1, zoom);				
			}
		}
		_SKIP_DRAW:
		draw_x += pw;
	}
	ESBYTE[_line] = NULL;
}

void TWebBrowser::RenderTextbuf()
{
	dword lbp = #linebuf;
	int br; //break position
	char nul = '\0';
	unsigned int len;

	//Do we need a line break?

	if (!linebuf[0]) return;

	while (len = strlen(lbp)) && (len * list.font_w + draw_x - left_gap >= draw_w) {
		//Yes, we do. Lets calculate where...
		br = len;

		//debugln("                \\n");

		//Is a new line fits in the current line?
		if (br * list.font_w + draw_x - left_gap >= draw_w) {
			br = draw_w - draw_x + left_gap /list.font_w;
			while(br) {
				if (ESBYTE[lbp + br]==' ') {
					br++;
					break;
				}
				br--;
			}
		}
		//Maybe a new line is too big for the whole new line? Then we have to split it
		if (!br) && (len * list.font_w >= draw_w) {
			br = draw_w - draw_x / list.font_w;
		}

		if (br) {
			ESBYTE[lbp + br] >< nul;
			RenderLine(lbp);
			ESBYTE[lbp + br] >< nul;
			lbp += br;
			while (ESBYTE[lbp]==' ') && (ESBYTE[lbp]) lbp++;
			if (ESBYTE[lbp]) NewLine();
		} else {
			NewLine();
			RenderLine(lbp);
		}
	}
	RenderLine(lbp);
}

void TWebBrowser::NewLine()
{
	static bool empty_line = true;

	if (draw_x==left_gap) && (draw_y==BODY_MARGIN) return;
	if (t_html) && (!t_body) return;
	
	if (draw_x == style.tag_list.level * 5 * list.font_w + left_gap) { 
		if (!empty_line) empty_line=true; else return;
	} else {
		empty_line = false;
	}

	draw_y += style.cur_line_h;
	style.cur_line_h = list.item_h;
	if (style.blq) draw_x = 6 * list.font_w; else draw_x = 0; //left_gap += style.tag_list.level * 5 * list.font_w ???
	draw_x += style.tag_list.level * 5 * list.font_w + left_gap;
}

bool TWebBrowser::RenderImage(dword cur_img)
{
	int img_x, img_y, img_w, img_h;
	dword imgbuf[44];

	if (!cur_img) return false;

	img_h = ESDWORD[cur_img+8];
	img_w = ESDWORD[cur_img+4];

	if (img_w + draw_x - left_gap >= draw_w) NewLine();
	img_y = draw_y;
	if (img_h < list.item_h) img_y += list.item_h - img_h / 2 - 1; else img_y -= 2;
	style.cur_line_h = math.max(style.cur_line_h, img_h);

	img_w = math.min(img_w, canvas.bufw - draw_x);

	if (link) links.add_text(draw_x + list.x, img_y + list.y, img_w, img_h, 0);

	if (img_y + img_h >= canvas.bufh) canvas.IncreaseBufSize();

	if (secondrun) 
	{	
		if (ESDWORD[cur_img+20] != IMAGE_BPP32) {
			img_convert stdcall(cur_img, 0, IMAGE_BPP32, 0, 0);
			$push eax
			img_destroy stdcall(cur_img);
			$pop eax
			cur_img = EAX;
			if (!EAX) return false;
		}
		imgbuf[04] = canvas.bufw;
		imgbuf[08] = canvas.bufh;
		imgbuf[20] = IMAGE_BPP32;
		imgbuf[24] = buf_data+8;
		img_blend stdcall(#imgbuf, cur_img, draw_x, img_y, 0, 0, img_w, img_h);
		img_destroy stdcall(cur_img);
	}

	draw_x += img_w;
	if (draw_x - left_gap >= draw_w) NewLine();
	return true;
}