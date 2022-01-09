#include "TWB\colors.h"
#include "TWB\anchors.h"
#include "TWB\parse_tag.h"
#include "TWB\special.h"
#include "TWB\tag_list.h"
#define DEFAULT_BG_COL 0xffFBE5E4; //- peach; 0xffEBE8E9 - more grey
dword link_color_default;
dword link_color_active;
#include "TWB\links.h"

#define BODY_MARGIN 6
#define BASIC_CHAR_W 8
#define BASIC_LINE_H 18

struct STYLE {
	bool
	b, u, s, h,
	font,
	pre,
	blq,
	button,
	image,
	nav, header;
	LIST tag_list;
	dword title;
	dword cur_line_h;
	void reset();
};

void STYLE::reset()
{
	b = u = s = h = font = pre = blq = 
	title = button = image = nav = header =	false;
	cur_line_h = NULL;
	tag_list.reset();
}

struct TWebBrowser {
	llist list;
	STYLE style;
	dword draw_y, draw_x, draw_w, left_gap;
	dword o_bufpointer;
	int cur_encoding, custom_encoding;
	bool link, t_html, t_body;
	dword bufpointer;
	dword bufsize;
	dword is_html;
	collection img_url;
	char header[150];
	char linebuf[500];
	char redirect[URL_SIZE];

	bool secondrun;

	void SetStyle();
	void RenderTextbuf();
	void RenderLine();
	bool RenderImage();

	void SetPageDefaults();
	void ParseHtml();
	void Reparse();
	void NewLine();
	void ChangeEncoding();
	void AddCharToTheLine();
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
	void tag_table_reset();
	void tag_table();
	void tag_td();
	void tag_tr();
};

#include "TWB\render.h"
#include "TWB\set_style.h"
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
	draw_y = BODY_MARGIN;
	draw_x = left_gap = BODY_MARGIN;
	draw_w = list.w - BODY_MARGIN;
	linebuf = 0;
	redirect = '\0';
	list.SetFont(8, 14, 10011000b);
	tag_table_reset();
	is_html = true;
	if (!strstri(bufpointer, "<body")) {
		t_body = true;
		if (!strstri(bufpointer, "<html")) && (!strstri(bufpointer, "<head")) && (!strstri(bufpointer, "<title")) 
		&& (!strstr(bufpointer, "<?xml")) && (!strstr(bufpointer, "<xml")) {
			style.pre = true; //show linebreaks for a plaint text
			is_html = false;
		}
	} 
}
//============================================================================================
void TWebBrowser::Reparse()
{
	ParseHtml(bufpointer, bufsize);
}
//============================================================================================
void TWebBrowser::ParseHtml(dword _bufpointer, _bufsize){
	int tab_len;
	dword bufpos;
	bufsize = _bufsize;

	if (list.w!=canvas.bufw) canvas.Init(list.x, list.y, list.w, 400*20);

	if (bufpointer == _bufpointer) {
		custom_encoding = cur_encoding;	
	} else {
		bufpointer = malloc(bufsize);
		memmov(bufpointer, _bufpointer, bufsize);

		//hold original buffer
		o_bufpointer = malloc(bufsize);
		memmov(o_bufpointer, bufpointer, bufsize);

		cur_encoding = CH_CP866;
		if (custom_encoding != -1) {
			cur_encoding = custom_encoding;
			bufpointer = ChangeCharset(cur_encoding, CH_CP866, bufpointer);
			bufsize = strlen(bufpointer);
		}
	}


	table.cols.drop();
	secondrun = false;

	_PARSE_START_:

	SetPageDefaults();
	for (bufpos=bufpointer; bufpos < bufpointer+bufsize; bufpos++;)
	{
		if (style.pre) {
			if (ESBYTE[bufpos] == 0x0a) {
				RenderTextbuf();
				NewLine();
				continue;
			}
			if (ESBYTE[bufpos] == 0x09) {
				tab_len = draw_x - left_gap / list.font_w + strlen(#linebuf) % 4;
				if (!tab_len) tab_len = 4; else tab_len = 4 - tab_len;
				while (tab_len) {chrcat(#linebuf,' '); tab_len--;}
				continue;
			}
		}
		if (ESBYTE[bufpos] == '&')	{
			bufpos = GetUnicodeSymbol(#linebuf, sizeof(TWebBrowser.linebuf), bufpos+1, bufpointer+bufsize);
			continue;
		}
		if (ESBYTE[bufpos] == '<') && (is_html) {
			if (strchr("!/?abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ESBYTE[bufpos+1])) {
				bufpos++;
				if (tag.parse(#bufpos, bufpointer + bufsize)) {

					/*
					if (secondrun) 
					{
						if (streq(#tag.name, "tr")) debugln(" ");
						if (!tag.opened) {
							debugch('/');
						} else {
							debugch(' ');
						}
						debugln(sprintf(#param, "%s   x%i y%i %s", 
							#tag.name, draw_x, draw_y, #linebuf));
					}
					//*/
					RenderTextbuf();
					if (debug_mode) { debugch('<'); if(!tag.opened)debugch('/'); debug(#tag.name); debugln(">"); }
					$push cur_encoding
					SetStyle();
					$pop eax
					// The thing is that UTF if longer than other encodings.
					// So if encoding was changed from UTF to DOS than $bufpos position got wrong,
					// and we have to start parse from the very beginning
					if (EAX != cur_encoding) && (cur_encoding == CH_UTF8) {
						ParseHtml(bufpointer, strlen(bufpointer));
						return;
					}
				}
				continue;
			}
		}
		AddCharToTheLine(ESBYTE[bufpos]);
	}

	if (!secondrun) {
		secondrun = true;
		goto _PARSE_START_;
	}

	RenderTextbuf();
	list.count = draw_y + style.cur_line_h;

	canvas.bufh = math.max(list.visible, list.count);
	buf_data = realloc(buf_data, canvas.bufh * canvas.bufw * 4 + 8);

	list.CheckDoesValuesOkey();
	anchors.current = NULL;
	if (!header) {
		strncpy(#header, #version, sizeof(TWebBrowser.header)-1);
		DrawTitle(#header);
	}
}
//============================================================================================
void TWebBrowser::AddCharToTheLine(unsigned char _char)
{
	dword line_len = strlen(#linebuf);
	if (_char<=15) _char=' ';
	if (!style.pre) && (_char == ' ')
	{
		if (linebuf[line_len-1]==' ') return; //no double spaces
		if (draw_x==left_gap) && (!linebuf) return; //no paces at the beginning of the line
		if (link) && (line_len==0) return;
	}
	if (line_len < sizeof(TWebBrowser.linebuf)) {
		chrcat(#linebuf+line_len, _char);
	} else {
		RenderTextbuf();
	}
}
//============================================================================================
void TWebBrowser::ChangeEncoding(int _new_encoding)
{
	if (cur_encoding == _new_encoding) return;
	cur_encoding = _new_encoding;
	bufpointer = ChangeCharset(cur_encoding, CH_CP866, bufpointer);
	if (header) {
		ChangeCharset(cur_encoding, CH_CP866, #header);
		DrawTitle(#header);
	}
}
//============================================================================================
scroll_bar scroll_wv = { 15,NULL,NULL,NULL,
  0,2,NULL,0,0,0xeeeeee,0xBBBbbb,0xeeeeee};

void TWebBrowser::DrawPage()
{
	if (list.w!=canvas.bufw) Reparse();
	canvas.Show(list.first, list.h);

	scroll_wv.max_area = list.count;
	scroll_wv.cur_area = list.visible;
	scroll_wv.position = list.first;
	scroll_wv.all_redraw = 0;
	scroll_wv.start_x = list.x + list.w;
	scroll_wv.start_y = list.y;
	scroll_wv.size_y = list.h;

	if (list.count <= list.visible) {
		DrawBar(scroll_wv.start_x, scroll_wv.start_y, scroll_wv.size_x, 
		scroll_wv.size_y, bg_colors.get(0) & 0x00FFFFFF);
	} else {
		scrollbar_v_draw(#scroll_wv);		
	}
}