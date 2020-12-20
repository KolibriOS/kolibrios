#include "TWB\colors.h"
#include "TWB\anchors.h"
#include "TWB\parse_tag.h"
#include "TWB\special.h"
#include "TWB\tag_list.h"
#define DEFAULT_BG_COL 0xffEBE8E9;
dword link_color_default;
dword link_color_active;
#include "TWB\links.h"

#define BODY_MARGIN 6
#define BASIC_LINE_H 18

CANVAS canvas;
char line[500];

struct STYLE {
	bool
	b, u, s, h,
	font,
	pre,
	blq,
	button,
	image;
	LIST tag_list;
	dword title;
	dword cur_line_h;
	void reset();
};

void STYLE::reset()
{
	b = u = s = h = blq = pre = title = false;
	font = false;
	cur_line_h = NULL;
	tag_list.reset();
}

struct TWebBrowser {
	llist list;
	STYLE style;
	dword draw_y, draw_x;
	dword o_bufpointer;
	int cur_encoding, custom_encoding;
	bool link, t_html, t_body;
	dword bufpointer;
	dword bufsize;
	dword is_html;
	collection img_url;
	char header[150];
	char redirect[URL_SIZE];

	void Paint();
	void SetPageDefaults();
	void AddCharToTheLine();
	void ParseHtml();
	void SetStyle();
	bool CheckForLineBreak();
	void NewLine();
	void ChangeEncoding();
	void DrawPage();

	void tag_a();
	void tag_p();
	void tag_img();
	void tag_div();
	void tag_h1234_caption();
	void tag_ol_ul_dt();
	void tag_li();
	void tag_q();
	void tag_hr();
	void tag_code();
	void tag_meta_xml();
	void tag_body();
	void tag_iframe();
	void tag_title();
	void tag_font();
};

#include "TWB\set_style.h"

//============================================================================================
void TWebBrowser::Paint()
{
	unsigned px; //paint x coordinate
	unsigned pw; //paint y coordinate
	   dword pc; //paint color
	   int zoom;

	if (style.title)
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
		pw = strlen(#line) * list.font_w;
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

		canvas.WriteText(draw_x, draw_y, list.font_type, pc, #line, NULL);
		if (style.b) canvas.WriteText(draw_x+1, draw_y, list.font_type, pc, #line, NULL);
		if (style.s) canvas.DrawBar(draw_x, list.item_h / 2 - zoom + draw_y, pw, zoom, pc);
		if (style.u) canvas.DrawBar(draw_x, list.item_h - zoom - zoom + draw_y, pw, zoom, pc);
		if (link) {
			if (line[0]==' ') && (line[1]==NULL) {} else {
				canvas.DrawBar(draw_x, draw_y + list.item_h - calc(zoom*2)-1, pw, zoom, link_color_default);
				links.add_text(draw_x, draw_y + list.y, pw, list.item_h - calc(zoom*2)-1, zoom);				
			}
		}
		draw_x += pw;
		if (debug_mode) debugln(#line);
		line = NULL;
	}
}
//============================================================================================
void TWebBrowser::SetPageDefaults()
{
	t_html = t_body = link = false;
	style.reset();
	link_color_default = 0x0000FF;
	link_color_active = 0xFF0000;
	style.cur_line_h = list.item_h;
	links.clear();
	anchors.clear();
	img_url.drop();
	text_colors.drop();
	text_colors.add(0);
	bg_colors.drop();
	bg_colors.add(DEFAULT_BG_COL);
	canvas.Fill(0, DEFAULT_BG_COL);
	header = NULL;
	cur_encoding = CH_CP866;
	draw_y = BODY_MARGIN;
	draw_x = BODY_MARGIN;
	line = 0;
	redirect = '\0';
	//hold original buffer
	if (o_bufpointer) o_bufpointer=free(o_bufpointer);
	o_bufpointer = malloc(bufsize);
	memmov(o_bufpointer, bufpointer, bufsize);
	if (custom_encoding != -1) {
		cur_encoding = custom_encoding;
		bufpointer = ChangeCharset(cur_encoding, "CP866", bufpointer);
	}
}
//============================================================================================
void TWebBrowser::ParseHtml(dword _bufpointer, _bufsize){
	char unicode_symbol[10];
	dword j;
	int tab_len;
	dword bufpos;
	bufsize = _bufsize;

	if (list.w!=canvas.bufw) canvas.Init(list.x, list.y, list.w, 400*20);

	if (bufpointer != _bufpointer) {
		bufpointer = malloc(bufsize);
		memmov(bufpointer, _bufpointer, bufsize);
	} else {
		custom_encoding = CH_CP866;	
	}
	SetPageDefaults();
	is_html = true;
	if (!strstri(bufpointer, "<body")) {
		t_body = true;
		if (!strstri(bufpointer, "<html")) && (!strstr(bufpointer, "<?xml")) && (!strstr(bufpointer, "<xml")) {
			style.pre = true; //show linebreaks for a plaint text
			is_html = false;
		}
	} 
	for (bufpos=bufpointer ; (bufpos < bufpointer+bufsize) && (ESBYTE[bufpos]!=0) ; bufpos++;)
	{
		switch (ESBYTE[bufpos])
		{
		case 0x0a:
			if (style.pre) {
				Paint();
				NewLine();
			} else {
				AddCharToTheLine(0x0a);
			}
			break;
		case 0x09:
			if (style.pre) {
				tab_len = draw_x - BODY_MARGIN / list.font_w + strlen(#line) % 4;
				if (!tab_len) tab_len = 4; else tab_len = 4 - tab_len;
				for (j=0; j<tab_len; j++;) chrcat(#line,' ');
			} else {
				AddCharToTheLine(0x09);
			}
			break;
		case '&': //&nbsp; and so on
			for (j=1, unicode_symbol=0; (ESBYTE[bufpos+j]<>';') && (!__isWhite(ESBYTE[bufpos+j])) && (j<8); j++)
			{
				chrcat(#unicode_symbol, ESBYTE[bufpos+j]);
			}
			if (GetUnicodeSymbol(#line, #unicode_symbol, sizeof(line)-1)) {
				bufpos += j;
				CheckForLineBreak();
			} else {
				AddCharToTheLine('&');
			}
			break;
		case '<':
			if (!is_html) goto _DEFAULT;
			bufpos++;
			switch (ESBYTE[bufpos]) {
				case '!': case '/': case '?': 
				case 'a'...'z': case 'A'...'Z':
					goto _TAG;
				default:
					goto _DEFAULT;
			}
			_TAG:
			if (tag.parse(#bufpos, bufpointer + bufsize)) {
				CheckForLineBreak();
				Paint();
				$push cur_encoding
				SetStyle();
				$pop eax
				// The thing is that UTF if longer than other encodings.
				// So if encoding was changed from UTF to DOS than $bufpos position got wrong,
				// and we have to start parse from the very beginning
				if (EAX != cur_encoding) && (cur_encoding == CH_UTF8) {
					ParseHtml(bufpointer, bufsize);
					return;
				}
			}
			break;
		default:
			_DEFAULT:
			AddCharToTheLine(ESBYTE[bufpos]);
		}
	}
	Paint();
	NewLine();
	list.count = draw_y;

	canvas.bufh = math.max(list.visible, draw_y);
	buf_data = realloc(buf_data, canvas.bufh * canvas.bufw * 4 + 8);

	list.CheckDoesValuesOkey();
	anchors.current = NULL;
	custom_encoding = -1;
	if (!header) {
		strncpy(#header, #version, sizeof(TWebBrowser.header)-1);
		DrawTitle(#header);
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
		if (draw_x==BODY_MARGIN) && (!line) return; //no paces at the beginning of the line
		if (link) && (line_len==0) return;
	}
	if (line_len < sizeof(line)) chrcat(#line, _char);
	CheckForLineBreak();
}
//============================================================================================
bool TWebBrowser::CheckForLineBreak()
{
	int break_pos;
	char next_line[4096];
	int zoom = list.font_w / BASIC_CHAR_W;
	//Do we need a line break?
	if (strlen(#line) * list.font_w + draw_x < list.w) return false;
	//Yes, we do. Lets calculate where...
	break_pos = strrchr(#line, ' ');

	//Is a new line fits in the current line?
	if (break_pos * list.font_w + draw_x > list.w) {
		break_pos = list.w - draw_x /list.font_w;
		while(break_pos) && (line[break_pos]!=' ') break_pos--;
	}
	//Maybe a new line is too big for the whole new line? Then we have to split it
	if (!break_pos) && (style.tag_list.level*5 + strlen(#line) * zoom >= list.column_max) {
		break_pos = list.w  - draw_x / list.font_w;
	}

	strcpy(#next_line, #line + break_pos);
	line[break_pos] = 0x00;		
	
	Paint();

	strcpy(#line, #next_line);
	NewLine();
	return true;
}
//============================================================================================
void TWebBrowser::NewLine()
{
	static bool empty_line = true;

	if (draw_x==BODY_MARGIN) && (draw_y==BODY_MARGIN) return;
	if (t_html) && (!t_body) return;
	
	if (draw_x == style.tag_list.level * 5 * list.font_w + BODY_MARGIN) { 
		if (!empty_line) empty_line=true; else return;
	} else {
		empty_line = false;
	}

	draw_y += style.cur_line_h;
	style.cur_line_h = list.item_h;
	if (style.blq) draw_x = 6 * list.font_w; else draw_x = 0;
	draw_x += style.tag_list.level * 5 * list.font_w + BODY_MARGIN;
}
//============================================================================================
void TWebBrowser::ChangeEncoding(int _new_encoding)
{
	if (cur_encoding == _new_encoding) return;
	cur_encoding = _new_encoding;
	bufpointer = ChangeCharset(cur_encoding, "CP866", bufpointer);
	if (header) {
		ChangeCharset(cur_encoding, "CP866", #header);
		DrawTitle(#header);
	}
}
//============================================================================================
scroll_bar scroll_wv = 
{ 15,NULL,NULL,NULL,0,2,NULL,
  0,0,0xeeeeee,0xBBBbbb,0xeeeeee};

void TWebBrowser::DrawPage()
{
	if (list.w!=canvas.bufw) {
		ParseHtml(bufpointer, bufsize);
	}
	canvas.Show(list.first, list.h);

	scroll_wv.max_area = list.count;
	scroll_wv.cur_area = list.visible;
	scroll_wv.position = list.first;
	scroll_wv.all_redraw = 0;
	scroll_wv.start_x = list.x + list.w;
	scroll_wv.start_y = list.y;
	scroll_wv.size_y = list.h;
	scrollbar_v_draw(#scroll_wv);

}