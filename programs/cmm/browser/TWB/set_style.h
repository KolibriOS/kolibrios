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
	if (tag.is("br"))         { NewLine();                 return; }
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

	//TO BE REWORKED
	//td_x = td_w = tr_y = highest_td = 0;
	//if (tag.is("table"))      { tag_table();          return; }
	//if (tag.is("td"))         { tag_td();             return; }
	//if (tag.is("tr"))         { tag_tr();             return; }

	if (tag.is("dd")) { 
		//NewLine();
		//if (tag.opened) stolbec += 5; //may overflow! 
		return; 
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
		text_colors.pop();
		bg_colors.pop();
	}
}

void TWebBrowser::tag_div()
{
	if (streq(#tag.prior,"div")) && (tag.opened) return;
	if (!tag.opened) && (style.font) text_colors.pop();
	NewLine();
}

void TWebBrowser::tag_iframe()
{
	if (tag.get_value_of("src")) {
		NewLine();
		strcpy(#line, "IFRAME: ");
		Render();
		link=true;
		links.add_link(tag.value);
		strncpy(#line, tag.value, sizeof(line)-1);
		while (CheckForLineBreak()) {};
		Render();
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
		if      (streqrp(EDX,"utf-8"))        || (streqrp(EDX,"utf8"))        ChangeEncoding(CH_UTF8);
		else if (streqrp(EDX,"windows-1251")) || (streqrp(EDX,"windows1251")) ChangeEncoding(CH_CP1251);
		else if (streqrp(EDX,"dos"))          || (streqrp(EDX,"cp-866"))      ChangeEncoding(CH_CP866);
		else if (streqrp(EDX,"iso-8859-5"))   || (streqrp(EDX,"iso8859-5"))   ChangeEncoding(CH_ISO8859_5);
		else if (streqrp(EDX,"koi8-r"))       || (streqrp(EDX,"koi8-u"))      ChangeEncoding(CH_KOI8);
	}
	if (streq(tag.get_value_of("http-equiv"), "refresh")) && (tag.get_value_of("content")) {
		if (tag.value = strstri(tag.value, "url")) strcpy(#redirect, tag.value);
	}
}

void TWebBrowser::tag_code()
{
	if (style.pre = tag.opened) {
		bg_colors.add(0xe4ffcb);
	} else {
		bg_colors.pop();
	}
}

void TWebBrowser::tag_ol_ul_dt()
{
	char type = ESBYTE[#tag.name];
	style.tag_list.upd_level(tag.opened, type);
	switch(type)
	{
		case 'd': 
			if (tag.opened) NewLine(); 
			break;
		case 'u': 
		case 'o': 
			if (!tag.opened) && (!style.pre) NewLine();
	}
}

void TWebBrowser::tag_li()
{
	if (tag.opened) {
		if (!style.tag_list.level) style.tag_list.upd_level(1, 'u');
		if (!style.pre) NewLine();
		if (style.tag_list.order_type() == 'u') {
			strcpy(#line, "\31 ");
			draw_x = style.tag_list.level * 5 - 2 * list.font_w + left_gap;
		} 
		if (style.tag_list.order_type() == 'o') {
			sprintf(#line, "%i. ", style.tag_list.inc_counter());
			draw_x = style.tag_list.level * 5 - 1 - strlen(#line) * list.font_w + left_gap;
		}
	}
}

void TWebBrowser::tag_hr()
{
	EAX = 0x999999;
	if (tag.get_value_of("color")) GetColor(tag.value);
	$push eax;
	NewLine();
	$pop edi;
	draw_y += 10;
	canvas.DrawBar(5, draw_y - 1, list.w-10, 1, EDI);
	NewLine();
	draw_y += 10;
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
	if (tag.opened)	{
		AddCharToTheLine(' ');
		AddCharToTheLine('\"');
	} else {
		AddCharToTheLine('\"');
		AddCharToTheLine(' ');
	}
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
		strncpy(#img_path, tag.value, sizeof(line)-3);
		sprintf(#line, "[%s]", #img_path);
	} else {
		if (streqrp(#img_path, "data:")) img_path=0;
		replace_char(#img_path, '?', NULL, strlen(#img_path));
		img_path[sizeof(line)-3] = '\0'; //prevent overflow in sprintf
		sprintf(#line, "[%s]", #img_path+strrchr(#img_path, '/'));
		line[50]= NULL;
	}
	while (CheckForLineBreak()) {};
	text_colors.add(0x9A6F29);
	style.image = true;
	Render();
	style.image = false;
	text_colors.pop();
}



unsigned tr_y;
unsigned td_x, td_w;
unsigned highest_td;
int table_c=0;

void TWebBrowser::tag_table()
{
	if (tag.opened) table_c++; else table_c--;
}

void TWebBrowser::tag_tr()
{
	if (table_c>1) return;
	//style.tr = tag.opened;
	NewLine();
	draw_w = list.w - left_gap;
	left_gap = BODY_MARGIN;
	td_w = 0;
	if (tag.opened) {
		tr_y = draw_y;
	} else {
		//draw_y = highest_td;
	}
}

void TWebBrowser::tag_td()
{
	if (table_c>1) return;
	if (tag.opened) {
		NewLine();
		//highest_td = math.max(draw_y, tr_y);
		if (tag.get_value_of("width")) td_w = atoi(tag.value);
		draw_y = tr_y;
		draw_x = left_gap;
		debugval("td_w", td_w);
		if (td_w > 20) draw_w = td_w; else draw_w = list.w - left_gap;
	} else {
		left_gap += td_w;
		draw_w = list.w - left_gap;
		/*
		draw_w -= left_gap;
		if (left_gap < 0) left_gap = BODY_MARGIN;
		if (draw_w < 0) {
			left_gap = BODY_MARGIN;
			draw_w = list.w - left_gap;
		}
		debugval("left_gap", left_gap);
		*/
	}
	debugval("td_w", td_w);
	debugval("left_gap", left_gap);
}