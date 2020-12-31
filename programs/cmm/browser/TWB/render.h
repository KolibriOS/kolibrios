CANVAS canvas;

void TWebBrowser::RenderLine()
{
	unsigned px; //paint x coordinate
	unsigned pw; //paint y coordinate
	   dword pc; //paint color
	   int zoom;

	if (style.title)
	{
		strncpy(#header, #linebuf, sizeof(TWebBrowser.header)-1);
		strncat(#header, " - ", sizeof(TWebBrowser.header)-1);
		strncat(#header, #version, sizeof(TWebBrowser.header)-1);
		linebuf = 0;
		return;
	}
	if (t_html) && (!t_body) {
		linebuf = 0;
		return;
	}
	
	if (linebuf)
	{
		pw = strlen(#linebuf) * list.font_w;
		zoom = list.font_w / BASIC_CHAR_W;

		style.cur_line_h = math.max(style.cur_line_h, list.item_h);

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

		canvas.WriteText(draw_x, draw_y, list.font_type, pc, #linebuf, NULL);
		if (style.b) canvas.WriteText(draw_x+1, draw_y, list.font_type, pc, #linebuf, NULL);
		if (style.s) canvas.DrawBar(draw_x, list.item_h / 2 - zoom + draw_y, pw, zoom, pc);
		if (style.u) canvas.DrawBar(draw_x, list.item_h - zoom - zoom + draw_y, pw, zoom, pc);
		if (link) {
			if (linebuf[0]==' ') && (linebuf[1]==NULL) {} else {
				canvas.DrawBar(draw_x, draw_y + list.item_h - calc(zoom*2)-1, pw, zoom, link_color_default);
				links.add_text(draw_x, draw_y + list.y, pw, list.item_h - calc(zoom*2)-1, zoom);				
			}
		}
		draw_x += pw;
		if (debug_mode) debugln(#linebuf);
	}
	linebuf = NULL;
}


void TWebBrowser::RenderTextbuf()
{
	int break_pos;
	char next_line[sizeof(TWebBrowser.linebuf)];
	int zoom = list.font_w / BASIC_CHAR_W;

	//Do we need a line break?
	while (strlen(#linebuf) * list.font_w + draw_x >= draw_w) {
		//Yes, we do. Lets calculate where...
		break_pos = strrchr(#linebuf, ' ');

		//Is a new line fits in the current line?
		if (break_pos * list.font_w + draw_x > draw_w) {
			break_pos = draw_w - draw_x /list.font_w;
			while(break_pos) {
				if (linebuf[break_pos]==' ') {
					break_pos++;
					break;
				}
				break_pos--;
			}
		}
		//Maybe a new line is too big for the whole new line? Then we have to split it
		if (!break_pos) && (style.tag_list.level*5 + strlen(#linebuf) * zoom >= list.column_max) {
			break_pos = draw_w  - draw_x / list.font_w;
		}


		if (break_pos) {
			strlcpy(#next_line, #linebuf + break_pos, sizeof(next_line));
			linebuf[break_pos] = 0x00;
			RenderLine();
			strlcpy(#linebuf, #next_line, sizeof(TWebBrowser.linebuf));
			NewLine();
		} else {
			NewLine();
			RenderLine();
		}
	}
	RenderLine();
}

bool TWebBrowser::RenderImage(dword cur_img)
{
	int img_x, img_y, img_w, img_h;
	dword imgbuf[44];

	if (!cur_img) return false;

	img_h = ESDWORD[cur_img+8];
	img_w = ESDWORD[cur_img+4];

	if (img_w + draw_x >= draw_w) NewLine();
	img_y = draw_y;
	if (img_h < list.item_h) img_y += list.item_h - img_h / 2 - 1; else img_y -= 2;
	style.cur_line_h = math.max(style.cur_line_h, img_h);

	img_w = math.min(img_w, canvas.bufw - draw_x);

	if (link) links.add_text(draw_x + list.x, img_y + list.y, img_w, img_h, 0);

	if (img_y + img_h >= canvas.bufh) canvas.IncreaseBufSize();

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

	draw_x += img_w;
	if (draw_x >= draw_w) NewLine();
	return true;
}