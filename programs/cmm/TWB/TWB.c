#include "..\TWB\colors.h"
#include "..\TWB\anchors.h"
#include "..\TWB\parce_tag.h"
#include "..\TWB\special.h"
#include "..\TWB\img_cache.h"
#include "..\TWB\tag_list.h"
dword page_bg;
dword link_color_default;
dword link_color_active;
#include "..\TWB\links.h"

#define BODY_MARGIN 6
#define BASIC_LINE_H 18

struct _style {
	bool
	b, u, s, h,
	pre,
	blq,
	button,
	image;
	dword bg_color;
	LIST tag_list;
	dword tag_title;
};

struct TWebBrowser {
	llist list;
	_style style;
	DrawBufer DrawBuf;
	dword draw_y, stolbec;
	int zoom;
	dword o_bufpointer;
	int cur_encoding, custom_encoding;
	bool link, t_html, t_body;
	dword bufpointer;
	dword bufsize;

	void SetPageDefaults();
	void AddCharToTheLine();
	void ParseHtml();
	void SetStyle();
	void DrawStyle();
	void DrawPage();
	void DrawScroller();
	void NewLine();
	bool CheckForLineBreak();
	char line[500];
	char header[150];
};

scroll_bar scroll_wv = { 15,NULL,NULL,NULL,0,2,NULL,0,0,0xeeeeee,0xBBBbbb,0xeeeeee};

//============================================================================================
void TWebBrowser::DrawStyle()
{
	dword start_x, line_length, stolbec_len;
	
	if (style.tag_title)
	{
		strncpy(#header, #line, sizeof(TWebBrowser.header)-1);
		strncat(#header, " - ", sizeof(TWebBrowser.header)-1);
		strncat(#header, #version, sizeof(TWebBrowser.header)-1);
		line = 0;
		return;
	}
	if (t_html) && (!t_body) {
		line = 0;
		return;
	}
	
	if (line)
	{
		start_x = stolbec * list.font_w + BODY_MARGIN + list.x;
		stolbec_len = strlen(#line) * zoom;
		line_length = stolbec_len * list.font_w;

		if (debug_mode) {
			DrawBuf.DrawBar(start_x, draw_y, line_length, list.item_h, 0xDDDddd);
		}

		if (style.bg_color!=page_bg) {
			DrawBuf.DrawBar(start_x, draw_y, line_length, list.item_h, style.bg_color);
		}

		if (style.image) {
			DrawBuf.DrawBar(start_x, draw_y, line_length, list.item_h-1, 0xF9DBCB);
		}
		if (style.button) {
			DrawBuf.DrawBar(start_x, draw_y, line_length, list.item_h - calc(zoom*2), 0xCCCccc);
			DrawBuf.DrawBar(start_x, draw_y + list.item_h - calc(zoom*2), line_length, zoom, 0x999999);
		}

		DrawBuf.WriteText(start_x, draw_y, list.font_type, text_colors[text_color_index], #line);
		if (style.b) DrawBuf.WriteText(start_x+1, draw_y, list.font_type, text_colors[text_color_index], #line);
		if (style.s) DrawBuf.DrawBar(start_x, list.item_h / 2 - zoom + draw_y, line_length, zoom, text_colors[text_color_index]);
		if (style.u) DrawBuf.DrawBar(start_x, list.item_h - zoom - zoom + draw_y, line_length, zoom, text_colors[text_color_index]);
		if (link) {
			if (line[0]==' ') && (line[1]==NULL) {} else {
				DrawBuf.DrawBar(start_x, draw_y + list.item_h - calc(zoom*2)-1, line_length, zoom, link_color_default);
				PageLinks.AddText(start_x, draw_y + list.y, line_length, list.item_h - calc(zoom*2)-1, UNDERLINE, zoom);				
			}
		}
		stolbec += stolbec_len;
		if (debug_mode) debug(#line);
		line = NULL;
	}
}
//============================================================================================
void TWebBrowser::SetPageDefaults()
{
	style.b = style.u = style.s = style.h = style.blq = t_html = t_body = style.pre =
	link = text_color_index = text_colors[0] = style.tag_title = false;
	style.tag_list.reset();
	link_color_default = 0x0000FF;
	link_color_active = 0xFF0000;
	page_bg = 0xFFFFFF;
	style.bg_color = page_bg;
	DrawBuf.Fill(0, page_bg);
	PageLinks.Clear();
	anchors.clear();
	header = NULL;
	cur_encoding = CH_CP866;
	draw_y = BODY_MARGIN;
	stolbec = 0;
	line = 0;
	zoom = 1;
	//hold original buffer
	if (o_bufpointer) o_bufpointer=free(o_bufpointer);
	o_bufpointer = malloc(bufsize);
	memmov(o_bufpointer, bufpointer, bufsize);
	if (custom_encoding != -1) {
		cur_encoding = custom_encoding;
		bufpointer = ChangeCharset(cur_encoding*10+#charsets, "CP866", bufpointer);
	}
}
//============================================================================================
void TWebBrowser::AddCharToTheLine(unsigned char _char)
{
	dword line_len;
	if (_char<=15) _char=' ';
	line_len = strlen(#line);
	if (!style.pre) && (_char == ' ')
	{
		if (line[line_len-1]==' ') return; //no double spaces
		if (!stolbec) && (!line) return; //no paces at the beginning of the line
	}
	if (line_len < sizeof(TWebBrowser.line)) chrcat(#line, _char);
	CheckForLineBreak();
}
//============================================================================================
void TWebBrowser::ParseHtml(dword _bufpointer, _bufsize){
	word bukva[2];
	char unicode_symbol[10];
	dword unicode_symbol_result;
	dword j;
	bool ignor_param=false;
	int tab_len;
	dword bufpos;
	bufsize = _bufsize;
	bufpointer = malloc(bufsize);
	memmov(bufpointer, _bufpointer, bufsize);
	SetPageDefaults();
	if (strstri(bufpointer, "<body")==-1) {
		t_body = true;
		if (strstri(bufpointer, "<html")==-1)  style.pre = true; //show linebreaks for a plaint text
	} 
	for (bufpos=bufpointer ; (bufpos < bufpointer+bufsize) && (ESBYTE[bufpos]!=0) ; bufpos++;)
	{
		bukva = ESBYTE[bufpos];
		switch (bukva)
		{
		case 0x0a:
			if (style.pre) {
				DrawStyle();
				NewLine();
			} else {
				AddCharToTheLine(0x0a);
			}
			break;
		case 0x09:
			if (style.pre) {
				tab_len = strlen(#line) + stolbec % 4;
				if (!tab_len) tab_len = 4; else tab_len = 4 - tab_len;
				for (j=0; j<tab_len; j++;) chrcat(#line,' ');
			} else {
				AddCharToTheLine(0x09);
			}
			break;
		case '&': //&nbsp; and so on
			for (j=1, unicode_symbol=0; (ESBYTE[bufpos+j]<>';') && (j<8); j++)
			{
				bukva = ESBYTE[bufpos+j];
				chrcat(#unicode_symbol, bukva);
			}
			if (GetUnicodeSymbol(#line, #unicode_symbol, sizeof(TWebBrowser.line)-1)) {
				bufpos += j;
				CheckForLineBreak();
			} else {
				AddCharToTheLine('&');
			}
			break;
		case '<':
			bufpos++;
			if (!strncmp(bufpos,"!--",3))
			{
				bufpos+=3;
				while (strncmp(bufpos,"-->",3)!=0) && (bufpos < bufpointer + bufsize)
				{
					bufpos++;
				}
				bufpos+=2;
				break;
			}
			tag.reset();
			if (ESBYTE[bufpos] == '/') {
				tag.opened = false;
				bufpos++;
			}

			ignor_param=false;
			while (ESBYTE[bufpos] !='>') && (bufpos < bufpointer + bufsize) //ïîëó÷àåì òåã è åãî ïàðàìåòðû
			{
				bukva = ESBYTE[bufpos];
				if (bukva == '\9') || (bukva == '\x0a') || (bukva == '\x0d') bukva = ' ';
				if (!ignor_param) && (bukva <>' ')
				{
					if (strlen(#tag.name)+1<sizeof(tag.name)) chrcat(#tag.name, bukva);
				}
				else
				{
					ignor_param = true;
					if (strlen(#tag.params)+1<sizeof(tag.params)) strcat(#tag.params, #bukva);
					//	chrncat(#tag.params, bukva, sizeof(tag.params)-1);
				}
				bufpos++;
			}
			strlwr(#tag.name);

			// ignore text inside the next tags
			if (tag.is("script")) || (tag.is("style")) || (tag.is("binary")) || (tag.is("select"))  { 
				sprintf(#tag.params, "</%s>", #tag.name);
				j = strstri(bufpos, #tag.params);
				if (j!=-1) bufpos = j-1;
				break;
			}

			if (tag.name[strlen(#tag.name)-1]=='/') tag.name[strlen(#tag.name)-1]=NULL; //for br/ !!!!!!!!
			if (tag.params) tag.parse_params();

			if (tag.name) {
				CheckForLineBreak();
				DrawStyle();
				if (tag.name) SetStyle();
			}
			break;
		default:
			AddCharToTheLine(ESBYTE[bufpos]);
		}
	}
	DrawStyle();
	NewLine();
	list.count = draw_y;
	list.CheckDoesValuesOkey();
	anchors.current = NULL;
	custom_encoding = -1;
	if (!header) {
		strncpy(#header, #version, sizeof(TWebBrowser.header)-1);
		DrawTitle(#header);
	}
}
//============================================================================================
bool TWebBrowser::CheckForLineBreak()
{
	int line_break_pos;
	char new_line_text[4096];
	//Do we need a line break?
	if (strlen(#line)*zoom + stolbec < list.column_max) return false;
	//Yes, we do. Lets calculate where...
	line_break_pos = strrchr(#line, ' ');
	//Is a new line fits in the current line?
	if (line_break_pos*zoom + stolbec > list.column_max) {
		line_break_pos = list.column_max/zoom - stolbec;
		while(line_break_pos) && (line[line_break_pos]!=' ') line_break_pos--;
	}
	//Maybe a new line is too big for the whole new line? Then we have to split it
	if (!line_break_pos) && (style.tag_list.level*5 + strlen(#line) * zoom >= list.column_max) {
		line_break_pos = list.column_max/zoom - stolbec;
	}
	strcpy(#new_line_text, #line + line_break_pos);
	line[line_break_pos] = 0x00;		
	
	DrawStyle();

	strcpy(#line, #new_line_text);
	NewLine();
	//if (strlen(#line)*zoom + stolbec > list.column_max)CheckForLineBreak();
	return true;
}
//============================================================================================
void TWebBrowser::SetStyle() {
	char img_path[4096]=0;
	int meta_encoding;

	dword value;

	if (value = tag.get_value_of("name=")) || (value = tag.get_value_of("id=")) {
		anchors.add(value, draw_y);
		if (anchors.current) && (streq(value, #anchors.current+1)) {
			list.first = draw_y;
			anchors.current = NULL;
		}
	}
	if (tag.is("html")) {
		t_html = tag.opened;
		return;
	}
	if (tag.is("q"))
	{
		if (tag.opened)	{
			meta_encoding = strlen(#line);
			if (line[meta_encoding-1] != ' ') chrcat(#line, ' ');
			chrcat(#line, '\"');
		}
		if (!tag.opened) strcat(#line, "\" ");
		return;
	}
	if (tag.is("title")) {
		style.tag_title = tag.opened;
		if (!tag.opened) DrawTitle(#header);
		return;
	}
	if (tag.is("body")) {
		t_body = tag.opened;
		if (value = tag.get_value_of("link="))  link_color_default = GetColor(value);
		if (value = tag.get_value_of("alink=")) link_color_active = GetColor(value);
		if (value = tag.get_value_of("text="))  text_colors[0]=GetColor(value);
		if (value = tag.get_value_of("bgcolor=")) {
			style.bg_color = page_bg = GetColor(value);
			DrawBuf.Fill(0, page_bg);
		}
		return;
	}
	if (tag.is("a")) {
		if (tag.opened)
		{
			if (link) IF(text_color_index > 0) text_color_index--; //åñëè ïðåäûäóùèé òåã à íå áûë çàêðûò
			if (value = tag.get_value_of("href=")) && (!strstr(value,"javascript:"))
			{
				text_color_index++;
				text_colors[text_color_index] = text_colors[text_color_index-1];
				link = 1;
				text_colors[text_color_index] = link_color_default;
				PageLinks.AddLink(value);
			}
		} else {
			link = 0;
			IF(text_color_index > 0) text_color_index--;
		}
		return;
	}
	if (tag.is("font")) {
		style.bg_color = page_bg;
		if (tag.opened)
		{
			text_color_index++;
			text_colors[text_color_index] = text_colors[text_color_index-1];
			if (value = tag.get_value_of("color=")) text_colors[text_color_index] = GetColor(value);
			if (value = tag.get_value_of("bg=")) style.bg_color = GetColor(value);
		}
		else if (text_color_index > 0) text_color_index--;
		return;
	}
	if (tag.is("div")) {
		if (streq(#tag.prior,"div")) && (tag.opened) return;
		NewLine();
		return;
	}
	if (tag.is("header")) || (tag.is("article")) || (tag.is("footer")) || (tag.is("figure")) {
		NewLine();
		return;
	}
	if (tag.is("p")) {
		IF (tag.prior[0] == 'h') || (streq(#tag.prior,"td")) || (streq(#tag.prior,"p")) return;
		NewLine();
		return;
	}
	if (tag.is("br")) { NewLine(); return; }
	if (tag.is("td")) { if (tag.opened) AddCharToTheLine(' '); return; }
	if (tag.is("tr")) { if (tag.opened) NewLine(); return; }
	if (tag.is("b")) || (tag.is("strong")) || (tag.is("big")) { style.b = tag.opened; return; }
	if (tag.is("button")) { style.button = tag.opened; stolbec++; return; }
	if (tag.is("u")) || (tag.is("ins")) { style.u=tag.opened; return;}
	if (tag.is("s")) || (tag.is("strike")) || (tag.is("del")) { style.s=tag.opened; return; }
	if (tag.is("dd")) { stolbec += 5; return; }
	if (tag.is("blockquote")) { style.blq = tag.opened; return; }
	if (tag.is("pre")) { style.pre = tag.opened; return; }
	if (tag.is("code")) { 
		if (tag.opened) style.bg_color = 0xe4ffcb; else style.bg_color = page_bg;
		style.pre = tag.opened; return; 
	}
	if (tag.is("img")) {
		if (value = tag.get_value_of("src=")) strlcpy(#img_path, value, sizeof(img_path)-1);
		if (value = tag.get_value_of("title=")) && (strlen(value)<sizeof(TWebBrowser.line)-3) && (value) sprintf(#line, "[%s]", value); 
		if (value = tag.get_value_of("alt=")) && (strlen(value)<sizeof(TWebBrowser.line)-3) && (value) sprintf(#line, "[%s]", value); 
		if (!img_path) { line=0; return; }
		style.image = true;
		text_color_index++;
		text_colors[text_color_index] = 0x9A6F29;
		if (!line) {
			if (!strncmp(#img_path, "data:", 5)) img_path=0;
			replace_char(#img_path, '?', NULL, strlen(#img_path));
			sprintf(#line, "[%s]", #img_path+strrchr(#img_path, '/'));
			line[50]= NULL;
		}
		while (CheckForLineBreak()) {};
		DrawStyle();
		text_color_index--;
		style.image = false;
		//ImgCache.Images( list.x, draw_y, list.w); 
		return; 
	}
	if (tag.is("h4")) {
		NewLine();
		NewLine();
		style.h = tag.opened;
		style.b = tag.opened;
	}
	if (tag.is("h1")) || (tag.is("h2")) || (tag.is("h3")) || (tag.is("caption")) {
		style.h = tag.opened;
		if (tag.opened) {
			if (!style.pre) NewLine();
			draw_y += 10;
			zoom=2;
			list.font_type |= 10011001b;
			list.item_h = BASIC_LINE_H * 2 - 2;
			if (tag.is("h1")) style.b = true;
		} else {
			if (tag.is("h1")) style.b = false;
			NewLine();
			zoom=1;
			list.font_type = 10011000b;
			list.item_h = BASIC_LINE_H;
		}
		return;
	}
	if (tag.is("dt")) {
		style.tag_list.upd_level(tag.opened, DT);
		if (tag.opened) NewLine();
		return;
	}
	if (tag.is("ul")) {
		style.tag_list.upd_level(tag.opened, UL);
		if (!tag.opened) && (!style.pre) NewLine();
		return;
	}
	if (tag.is("ol")) {
		style.tag_list.upd_level(tag.opened, OL);	
		if (!tag.opened) && (!style.pre) NewLine();
		return;
	}
	if (tag.is("li")) && (tag.opened)
	{
		if (!style.tag_list.level) style.tag_list.upd_level(1, UL);
		if (!style.pre) NewLine();
		if (style.tag_list.get_order_type() == UL) {
			strcpy(#line, "\31 ");
			stolbec = style.tag_list.level * 5 - 2;
		} 
		if (style.tag_list.get_order_type() == OL) {
			sprintf(#line, "%i. ", style.tag_list.inc_counter());
			stolbec = style.tag_list.level * 5 - strlen(#line);
		}
		return;
	}
	if (tag.is("hr")) {
		if (value = tag.get_value_of("color=")) EDI = GetColor(value); else EDI = 0x999999;
		$push edi;
		NewLine();
		$pop edi;
		draw_y += 10;
		DrawBuf.DrawBar(5, draw_y - 1, list.w-10, 1, EDI);
		NewLine();
		draw_y += 10;
		return;
	}
	if (custom_encoding == -1) && (tag.is("meta")) || (tag.is("?xml")) {
		meta_encoding = CH_CP866;
		if (value = tag.get_value_of("charset=")) || (value = tag.get_value_of("content=")) || (value = tag.get_value_of("encoding="))
		{
			value += strrchr(value, '='); //search in content=
			strlwr(value);
			if      (streq(value,"utf-8"))        || (streq(value,"utf8"))        meta_encoding = CH_UTF8;
			else if (streq(value,"windows-1251")) || (streq(value,"windows1251")) meta_encoding = CH_CP1251;
			else if (streq(value,"dos"))          || (streq(value,"cp-866"))      meta_encoding = CH_CP866;
			else if (streq(value,"iso-8859-5"))   || (streq(value,"iso8859-5"))   meta_encoding = CH_ISO8859_5;
			else if (streq(value,"koi8-r"))       || (streq(value,"koi8-u"))      meta_encoding = CH_KOI8;
		}
		if (meta_encoding != CH_CP866) && (cur_encoding != meta_encoding) {
			cur_encoding = meta_encoding;
			bufpointer = ChangeCharset(cur_encoding*10+#charsets, "CP866", bufpointer);
			if (header) {
				ChangeCharset(cur_encoding*10+#charsets, "CP866", #header);
				DrawTitle(#header);
			}
		}
		return;
	}
}
//============================================================================================
void TWebBrowser::DrawScroller()
{
	scroll_wv.max_area = list.count;
	scroll_wv.cur_area = list.visible;
	scroll_wv.position = list.first;
	scroll_wv.all_redraw = 0;
	scroll_wv.start_x = list.x + list.w;
	scroll_wv.start_y = list.y;
	scroll_wv.size_y = list.h;
	scrollbar_v_draw(#scroll_wv);
}
//============================================================================================
void TWebBrowser::NewLine()
{
	dword onleft, ontop;
	static int empty_line=0;

	if (!stolbec) && (draw_y==BODY_MARGIN) return;
	
	if (style.tag_list.level) && (stolbec == style.tag_list.level * 5) { 
		if (empty_line<1) empty_line++;
		else return;
	} else if (!stolbec) { 
		if (empty_line<1) empty_line++;
		else return;
	} else {
		empty_line=0;
	}

	onleft = list.x + BODY_MARGIN;
	ontop = draw_y + list.y;
	if (t_html) && (!t_body) return;
	draw_y += list.item_h;
	if (style.blq) stolbec = 6; else stolbec = 0;
	stolbec += style.tag_list.level * 5;
	if (debug_mode) debugln(NULL);
}
//============================================================================================
void TWebBrowser::DrawPage()
{
	PutPaletteImage(list.first * DrawBuf.bufw * 4 + buf_data+8, DrawBuf.bufw, list.h, DrawBuf.bufx, DrawBuf.bufy, 32, 0);	
	DrawScroller();
}