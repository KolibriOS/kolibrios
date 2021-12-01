void TWebBrowser::SetStyle() 
{
	if (tag.get_value_of("name")) || (tag.get_value_of("id")) {
		anchors.add(tag.value, draw_y);
		if (anchors.current) && (streq(tag.value, #anchors.current+1)) {
			list.first = draw_y;
			anchors.current = NULL;
		}
	}
	if (tag.is("a"))          { tag_a();                   return; }
	if (tag.is("p"))          { tag_p();                   return; } 
	if (tag.is("img"))        { tag_img();                 return; }
	if (tag.is("div"))        { tag_div();                 return; }
	if (tag.is("br"))         { /*draw_x++;*/NewLine();    return; }
	if (tag.is("nav"))        { style.nav = tag.opened;    return; }
	if (tag.is("header"))     { NewLine();                 return; }
	if (tag.is("article"))    { NewLine();                 return; }
	if (tag.is("footer"))     { NewLine();                 return; } 
	if (tag.is("figure"))     { NewLine();                 return; }
	if (tag.is("w:p"))        { NewLine();                 return; }
	if (tag.is("b"))          { style.b      = tag.opened; return; }
	if (tag.is("strong"))     { style.b      = tag.opened; return; }
	if (tag.is("big"))        { style.b      = tag.opened; return; }
	if (tag.is("w:b"))        { style.b      = tag.opened; return; }
	if (tag.is("u"))          { style.u      = tag.opened; return; }
	if (tag.is("ins"))        { style.u      = tag.opened; return; }
	if (tag.is("s"))          { style.s      = tag.opened; return; }
	if (tag.is("strike"))     { style.s      = tag.opened; return; }
	if (tag.is("del"))        { style.s      = tag.opened; return; }
	if (tag.is("pre"))        { style.pre    = tag.opened; return; }
	if (tag.is("blockquote")) { style.blq    = tag.opened; return; }
	if (tag.is("button"))     { style.button = tag.opened; return; }
	if (tag.is("dl"))         { if (tag.opened) NewLine(); return; }
	if (tag.is("w:r"))        { if (!tag.opened) style.b = false;      return; }
	if (tag.is("h1"))         { tag_h1234_caption();  return; }
	if (tag.is("h2"))         { tag_h1234_caption();  return; }
	if (tag.is("h3"))         { tag_h1234_caption();  return; }
	if (tag.is("h4"))         { tag_h1234_caption();  return; }
	if (tag.is("font"))       { tag_font();           return; }
	if (tag.is("dt"))         { tag_ol_ul_dt();       return; }
	if (tag.is("dd"))         { tag_ol_ul_dt();       return; }
	if (tag.is("ul"))         { tag_ol_ul_dt();       return; }
	if (tag.is("ol"))         { tag_ol_ul_dt();       return; }
	if (tag.is("li"))         { tag_li();             return; }
	if (tag.is("q"))          { tag_q();              return; }
	if (tag.is("hr"))         { tag_hr();             return; }
	if (tag.is("meta"))       { tag_meta_xml();       return; }
	if (tag.is("?xml"))       { tag_meta_xml();       return; }
	if (tag.is("code"))       { tag_code();           return; }
	if (tag.is("iframe"))     { tag_iframe();         return; }
	if (tag.is("caption"))    { tag_h1234_caption();  return; }
	if (tag.is("title"))      { tag_title();          return; }
	if (tag.is("body"))       { tag_body();           return; }
	if (tag.is("html"))       { t_html = tag.opened;  return; }
	if (tag.is("table"))      { tag_table();          return; }
	if (tag.is("tr"))         { tag_table();          return; }
	if (tag.is("th"))         { tag_table();          return; }
	if (tag.is("td"))         { tag_table();          return; }

	if (application_mode) {
	    if (tag.is("exit")) { ExitProcess(); return; }
	}
}

void TWebBrowser::tag_p()
{
	IF (tag.prior[0] == 'h') || (streq(#tag.prior,"td")) || (streq(#tag.prior,"p")) return;
	NewLine();
}

void TWebBrowser::tag_title()
{	
	style.title = tag.opened;
	if (!tag.opened) DrawTitle(#header);
}

void TWebBrowser::tag_font()
{
	style.font = tag.opened;
	if (tag.opened)
	{
		if (tag.get_value_of("bg")) {
			bg_colors.add(GetColor(tag.value));
		} else {
			bg_colors.add(bg_colors.get_last());
		}
		if (tag.get_value_of("color")) {
			text_colors.add(GetColor(tag.value));
		} else {
			text_colors.add(text_colors.get_last());
		}
	}
	else {
		if (bg_colors.count>1) text_colors.pop();
		if (bg_colors.count>1) bg_colors.pop(); //never pop the last color
	}
}

void TWebBrowser::tag_div()
{
	//if (streq(#tag.prior,"div")) && (tag.opened) return;
	if (streq(#tag.prior,"td")) return;
	if (streq(#tag.prior,"div")) return;
	if (!tag.opened) && (style.font) text_colors.pop();
	NewLine();
}

void TWebBrowser::tag_iframe()
{
	if (tag.get_value_of("src")) {
		NewLine();
		strcpy(#linebuf, "IFRAME: ");
		RenderTextbuf();
		link=true;
		links.add_link(tag.value);
		strncpy(#linebuf, tag.value, sizeof(TWebBrowser.linebuf)-1);
		RenderTextbuf();
		link=false;
		NewLine();
	}
}

void TWebBrowser::tag_a()
{
	if (tag.opened)
	{
		if (tag.get_value_of("href")) && (!strstr(tag.value,"javascript:"))
		{
			link = true;
			links.add_link(tag.value);
		}
	} else {
		link = false;
	}
}

void TWebBrowser::tag_meta_xml()
{
	if (custom_encoding == -1) if (tag.get_value_of("charset")) 
	|| (tag.get_value_of("content")) || (tag.get_value_of("encoding"))
	{
		EDX = strrchr(tag.value, '=') + tag.value; //search in content=
		if (ESBYTE[EDX] == '"') EDX++;
		strlwr(EDX);
		EAX = get_encoding_type_by_name(EDX);
		if (EAX!=-1) ChangeEncoding(EAX);
	}
	if (streq(tag.get_value_of("http-equiv"), "refresh")) && (tag.get_value_of("content")) {
		if (tag.value = strstri(tag.value, "url")) strcpy(#redirect, tag.value);
	}
	if (streq(tag.get_value_of("name"), "application")) {
	    if (application_mode) {
	        if (tag.get_number_of("left")) {
                MoveSize(tag.number,-1,-1,-1);
            }
            if (tag.get_number_of("top")) {
                MoveSize(-1,tag.number,-1,-1);
            }
            if (tag.get_number_of("width")) {
                MoveSize(-1,-1,tag.number,-1);
            }
            if (tag.get_number_of("height")) {
                MoveSize(-1,-1,-1,tag.number);
            }
	    }
    }
}

signed int get_encoding_type_by_name(dword name)
{
	EDX = name;
	if      (streqrp(EDX,"utf-8"))        || (streqrp(EDX,"utf8"))        return CH_UTF8;
	else if (streqrp(EDX,"windows-1251")) || (streqrp(EDX,"windows1251")) return CH_CP1251;
	else if (streqrp(EDX,"dos"))          || (streqrp(EDX,"cp-866"))      return CH_CP866;
	else if (streqrp(EDX,"iso-8859-5"))   || (streqrp(EDX,"iso8859-5"))   return CH_ISO8859_5;
	else if (streqrp(EDX,"koi8-r"))       || (streqrp(EDX,"koi8-u"))      return CH_KOI8;
	return -1;
}

void TWebBrowser::tag_code()
{
	if (style.pre = tag.opened) {
		bg_colors.add(0xe4ffcb);
	} else {
		if (bg_colors.count>1) bg_colors.pop(); //never pop the last color
	}
}

void TWebBrowser::tag_ol_ul_dt()
{
	char type = ESBYTE[#tag.name];
	switch(type)
	{
		case 'd': 
			if (ESBYTE[#tag.name+1]=='d') style.tag_list.upd_level(tag.opened, type);
			if (ESBYTE[#tag.name+1]=='t') style.tag_list.upd_level(false, 'd');
			if (tag.opened) NewLine(); 
			break;
		case 'u': 
		case 'o': 
			if (style.nav) && (style.tag_list.level) NewLine();
			style.tag_list.upd_level(tag.opened, type);
			if (!tag.opened) && (!style.pre) NewLine();
	}
}

void TWebBrowser::tag_li()
{
	if (style.nav) return;
	if (tag.opened) {
		if (!style.tag_list.level) style.tag_list.upd_level(1, 'u');
		if (!style.pre) NewLine();
		if (style.tag_list.order_type() == 'u') {
			strcpy(#linebuf, "\31 ");
			draw_x = style.tag_list.level * 5 - 2 * list.font_w + left_gap;
		} 
		if (style.tag_list.order_type() == 'o') {
			sprintf(#linebuf, "%i. ", style.tag_list.inc_counter());
			draw_x = style.tag_list.level * 5 - 1 - strlen(#linebuf) * list.font_w + left_gap;
		}
	}
}

void TWebBrowser::tag_hr()
{
	dword hrcol = 0x00777777;
	if (tag.get_value_of("color")) hrcol = GetColor(tag.value);
	if (draw_x != left_gap) NewLine();
	if (secondrun) canvas.DrawBar(5+left_gap, style.cur_line_h / 2 + draw_y - 1, draw_w-10, 1, hrcol);
	draw_x++;
	NewLine();
	return;
}

void TWebBrowser::tag_body()
{
	t_body = tag.opened;
	if (tag.get_value_of("link"))   link_color_default = GetColor(tag.value);
	if (tag.get_value_of("alink"))  link_color_active = GetColor(tag.value);
	if (tag.get_value_of("text"))   text_colors.set(0, GetColor(tag.value));
	if (tag.get_value_of("bgcolor")) {
		bg_colors.set(0, GetColor(tag.value));
		canvas.Fill(0, bg_colors.get(0));
	}
	// Autodetecting encoding if no encoding was set
	if (tag.opened) && (custom_encoding==-1) && (cur_encoding == CH_CP866) {
		if (strstr(bufpointer, "\208\190")) ChangeEncoding(CH_UTF8);
		else if (chrnum(bufpointer, '\246')>5) ChangeEncoding(CH_CP1251);
	}
}

void TWebBrowser::tag_q()
{
	chrncat(#linebuf, '\"', sizeof(TWebBrowser.linebuf));
}

void TWebBrowser::tag_h1234_caption()
{
	if (ESBYTE[#tag.name+1]=='4') {
		NewLine();
		NewLine();
		style.h = tag.opened;
		style.b = tag.opened;
	} else {
		style.h = tag.opened;
		if (tag.opened) {
			if (!style.pre) NewLine();
			draw_y += 10;
			list.SetFont(BASIC_CHAR_W*2, 14*2, 10011001b);
			list.item_h = BASIC_LINE_H * 2 - 2;
			if (tag.is("h1")) style.b = true;
		} else {
			if (tag.is("h1")) style.b = false;
			NewLine();
			list.SetFont(BASIC_CHAR_W, 14, 10011000b);
			style.cur_line_h = list.item_h = BASIC_LINE_H;
		}		
	}
}


void TWebBrowser::tag_img()
{
	char img_path[4096]=0;
	dword base64img;

	if (!tag.get_value_of("data-large-image")) 
		if (!tag.get_value_of("data-src")) 
			if (!tag.get_value_of("src")) return;

	if (streqrp(tag.value, "data:")) {
		if (!strstr(tag.value, "base64,")) goto NOIMG;
		EDX = EAX+7;
		if (ESBYTE[EDX]==' ') EDX++;
		base64img = malloc(strlen(EDX));
		base64_decode stdcall (EDX, base64img, strlen(EDX));
		img_decode stdcall (base64img, EAX, 0);
		$push eax
		free(base64img);
		$pop eax
		if (EAX) goto IMGOK; else goto NOIMG;
	} 

	if (!strcmp(tag.value + strrchr(tag.value, '.'), "svg")) goto NOIMG;
	if (!strcmp(tag.value + strrchr(tag.value, '.'), "webp")) goto NOIMG;

	strlcpy(#img_path, tag.value, sizeof(img_path)-1);
	replace_char(#img_path, ' ', '\0', sizeof(img_path));
	get_absolute_url(#img_path, history.current());

	if (check_is_the_adress_local(#img_path)) {
		img_from_file stdcall(#img_path);
		if (EAX) goto IMGOK; else goto NOIMG;
	}

	if (cache.has(#img_path)) {
		img_decode stdcall (cache.current_buf, cache.current_size, 0);
		if (EAX) goto IMGOK; else goto NOIMG;
	} else {
		img_url.add(#img_path);
		goto NOIMG;
	}

IMGOK:
	if (RenderImage(EAX)) return;

NOIMG:
	if (tag.get_value_of("title")) || (tag.get_value_of("alt")) {
		strncpy(#img_path, tag.value, sizeof(TWebBrowser.linebuf)-3);
		sprintf(#linebuf, "[%s]", #img_path);
	} else {
		if (streqrp(#img_path, "data:")) img_path=0;
		replace_char(#img_path, '?', NULL, strlen(#img_path));
		img_path[sizeof(TWebBrowser.linebuf)-3] = '\0'; //prevent overflow in sprintf
		sprintf(#linebuf, "[%s]", #img_path+strrchr(#img_path, '/'));
		linebuf[50]= NULL;
	}
	text_colors.add(0x9A6F29);
	style.image = true;
	RenderTextbuf();
	style.image = false;
	text_colors.pop();
}





struct TABLE {
	int count;
	int depth;
	int margin;
	collection_int cols;
	collection_int width;
} table;


int tr_pos, td_pos;
int row_start_y;
int colcount;
dword tallest_cell_in_row;

void TWebBrowser::tag_table_reset()
{
	table.depth = 0;
	table.count = 0;
	colcount = 0;
	tr_pos = 0;
	td_pos = 0;
}

void TWebBrowser::tag_table()
{
	if (!tag.opened) {
		if (style.font) tag_font();
		if (link) tag_a();
		style.b = false;
	}
	if (tag.is("table")) {
		if(tag.opened) {
			table.depth++;
			if (table.depth==1) {
				table.count++;
				colcount = 0;
				td_pos = 0;
				row_start_y = draw_y;
				if (tag.get_number_of("width")) {
					if (strchr(tag.value, '%')) tag.number = list.w * tag.number / 100;
					table.width.set(table.count, math.min(tag.number,list.w));
				} else {
					table.width.set(table.count, list.w);
				}
			}
		} else {
			if (table.depth>0) {
				table.depth--;
			}
			if (table.depth==0) {
				draw_x = left_gap = style.tag_list.level * 5 * list.font_w + BODY_MARGIN;
				draw_w = list.w;
				if (td_pos) draw_y = math.max(draw_y, tallest_cell_in_row);
				draw_y = math.max(draw_y+style.cur_line_h, tallest_cell_in_row);
				row_start_y = tallest_cell_in_row = draw_y;
				style.cur_line_h = list.item_h;
			}
		}
	}
	if (table.depth>1) {
		if (tag.is("tr")) && (tag.opened) NewLine();
		return;
	}

	if (!secondrun) {
		if (tag.is("tr")) {
			if (tag.opened) {
				table.cols.add(1);
			}
			colcount = 0;
		}
		if (tag.opened) if (tag.is("td")) || (tag.is("th")) {
			if (!table.cols.count) { table.cols.add(1); colcount = 0;}
			colcount++;
			if (colcount) table.cols.set(table.cols.count-1, colcount);
			//if (tag.get_number_of("colspan")) colcount += tag.number-1;
		}
	} else {
		if (tag.is("tr")) {
			if (tag.opened) {
				if (td_pos) draw_y = math.max(draw_y, tallest_cell_in_row);
				_TR_FIX:
				if (draw_x==left_gap) && (draw_y==BODY_MARGIN) {
					row_start_y = tallest_cell_in_row = draw_y;
				} else {
					row_start_y = tallest_cell_in_row = draw_y = draw_y + style.cur_line_h;
				}
				style.cur_line_h = list.item_h;
				tr_pos++;
				td_pos = 0;
			} else {
				draw_y = math.max(draw_y, tallest_cell_in_row);
				td_pos = 0;
			}
			draw_x = left_gap = style.tag_list.level * 5 * list.font_w + BODY_MARGIN;
			draw_w = list.w;
		}
		if (tag.is("td")) || (tag.is("th"))  {

			if (!tr_pos) goto _TR_FIX;

			/*
			if (tag.opened) {
				if (tag.get_value_of("bgcolor")) {
					bg_colors.add(GetColor(tag.value));
				} else {
					bg_colors.add(bg_colors.get(0));
				}
			} */

			tallest_cell_in_row = math.max(draw_y+style.cur_line_h-list.item_h, tallest_cell_in_row);
			style.cur_line_h = list.item_h;
			if (tag.opened) {
				
				if (!td_pos) {
					table.margin = list.w - table.width.get(table.count) / 2 + BODY_MARGIN;
					draw_x = left_gap = table.margin;
					draw_w = table.width.get(table.count) - BODY_MARGIN;
				} else {
					draw_x = left_gap = left_gap + draw_w;
					draw_w = table.width.get(table.count) - left_gap + table.margin - BODY_MARGIN;
				}

				if (EAX = table.cols.get(tr_pos-1)-td_pos) {
					draw_w /= EAX;
				} else {
					draw_y = row_start_y;
					draw_x = left_gap = table.margin;
				}
				if (table.cols.get(tr_pos-1)-td_pos>1) && (tag.get_number_of("width")) {
					if (strchr(tag.value, '%')) {
						tag.number = table.width.get(table.count) - table.margin - 23 - left_gap * tag.number / 100;
					}
					if (tag.number < draw_w) draw_w = tag.number;
				}
				draw_y = row_start_y;
				//canvas.WriteText(draw_x, draw_y, 10001001b, 0x0000FE, itoa(draw_x), NULL);
				//canvas.WriteText(draw_x, draw_y+20, 10001001b, 0xFF0000, itoa(draw_w), NULL);
				td_pos++;
			}
		}
	}
	if (draw_x > table.width.get(table.count)) {
		draw_x = left_gap = table.margin;
		draw_w = table.width.get(table.count) - table.margin - 23 - left_gap;
		table.depth = 0;
		NewLine();
		if (debug_mode) {
			debugln("anomaly draw_x");
			canvas.DrawBar(0, draw_y, 20, 20, 0xFF0000);
		}
	}
	/*
	if (left_gap + draw_w > list.w) {
		draw_w = list.w - left_gap;
		if (debug_mode) debugln("anomaly draw_W");
	}
	*/
}

