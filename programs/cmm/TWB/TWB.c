
scroll_bar scroll_wv = { 15,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

enum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT};

struct _style {
bool
	b, u, s, h,
	pre,
	blq,
	li,
	li_tab,
	button,
	image,
	align;
dword 
	bg_color;
};

struct TWebBrowser {
	llist list;
	_style style;
	DrawBufer DrawBuf;
	int zoom;
	bool opened; //is this a "start tag" or "end tag"
	void SetPageDefaults();
	void Prepare();
	void SetStyle();
	void DrawStyle();
	void DrawPage();
	void DrawScroller();
	void NewLine();
	bool CheckForLineBreak();
	void BufEncode();
} WB1;


bool 	
	link,
	cur_encoding,
	t_html,
	t_body;

dword bufpointer=0;
dword o_bufpointer=0;
dword bufsize=0;

dword text_colors[300];
dword text_color_index;
dword link_color_inactive;
dword link_color_active;
dword page_bg;

int draw_y;
int stolbec;
int tab_len;

int body_magrin=5;
int basic_line_h=22;

char URL[10000];
char header[2048];
char line[500];
char tagparam[10000];
char tag[100];
char oldtag[100];
char attr[1200];
char val[4096];

#include "..\TWB\absolute_url.h"
#include "..\TWB\links.h"
#include "..\TWB\anchors.h"
#include "..\TWB\colors.h"
#include "..\TWB\unicode_tags.h"
#include "..\TWB\img_cache.h"
#include "..\TWB\parce_tag.h"


//============================================================================================
void TWebBrowser::DrawStyle()
{
	dword start_x, line_length, stolbec_len;
	
	if (!header)
	{
		strcpy(#header, #line);
		line = 0;
		return;
	}
	if (t_html) && (!t_body) return;
	
	if (line)
	{
		start_x = stolbec * list.font_w + body_magrin + list.x;
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
				DrawBuf.DrawBar(start_x, draw_y + list.item_h - calc(zoom*2), line_length, zoom, link_color_inactive);
				PageLinks.AddText(start_x, draw_y + list.y, line_length, list.item_h - calc(zoom*2), UNDERLINE, zoom);				
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
	style.li = link = text_color_index = text_colors[0] = style.li_tab = false;
	style.align = ALIGN_LEFT;
	link_color_inactive = 0x0000FF;
	link_color_active = 0xFF0000;
	page_bg = 0xFFFFFF;
	style.bg_color = page_bg;
	DrawBuf.Fill(0, page_bg);
	PageLinks.Clear();
	strcpy(#header, #version);
	cur_encoding = CH_NULL;
	draw_y = body_magrin;
	stolbec = 0;
	line = 0;
	zoom = 1;	
}
//============================================================================================
void TWebBrowser::Prepare(){
	word bukva[2];
	dword j;
	bool ignor_param;
	dword bufpos;
	dword line_len;
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
			if (style.pre)
			{
				DrawStyle();
				NewLine();
				break;
			}
			goto DEFAULT_MARK;
		case '\9':
			if (style.pre) //otherwise go to 0x0d	
			{
				tab_len = strlen(#line) % 4;
				if (!tab_len) tab_len = 4;
				for (j=0; j<tab_len; j++;) chrcat(#line,' ');
				break;
			}
			goto DEFAULT_MARK;
		case '&': //&nbsp; and so on
			for (j=1, tag=0; (ESBYTE[bufpos+j]<>';') && (j<8); j++)
			{
				bukva = ESBYTE[bufpos+j];
				chrcat(#tag, bukva);
			}
			if (bukva = GetUnicodeSymbol(#tag)) {
				bufpos += j;
				CheckForLineBreak();
			} else {
				bukva = '&';
				goto DEFAULT_MARK;
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
			tag = attr = tagparam = ignor_param = NULL;
			while (ESBYTE[bufpos] !='>') && (bufpos < bufpointer + bufsize) //ïîëó÷àåì òåã è åãî ïàðàìåòðû
			{
				bukva = ESBYTE[bufpos];
				if (bukva == '\9') || (bukva == '\x0a') || (bukva == '\x0d') bukva = ' ';
				if (!ignor_param) && (bukva <>' ')
				{
					if (strlen(#tag)+1<sizeof(tag)) chrcat(#tag, bukva);
				}
				else
				{
					ignor_param = true;
					if (strlen(#tagparam)+1<sizeof(tagparam)) strcat(#tagparam, #bukva);
					//	chrncat(#tagparam, bukva, sizeof(tagparam)-1);
				}
				bufpos++;
			}
			strlwr(#tag);

			// ignore text inside the next tags
			if (istag("script")) || (istag("style")) || (istag("binary")) || (istag("select"))  { 
				sprintf(#tagparam, "</%s>", #tag);
				j = strstri(bufpos, #tagparam);
				if (j!=-1) {
					bufpos = j-1;
				}
				tag = tagparam = NULL;
				break;
			}

			if (tag[strlen(#tag)-1]=='/') tag[strlen(#tag)-1]=NULL; //for br/
			if (tagparam) GetNextParam();

			if (tag[0] == '/') 
			{
				 opened = 0;
				 strcpy(#tag, #tag+1);
			}
			else opened = 1;

			if (tag) && (!istag("span")) && (!istag("i")) && (!istag("svg")) {
				CheckForLineBreak();
				DrawStyle();
				if (tag) SetStyle();
			}
			strlcpy(#oldtag, #tag, sizeof(oldtag)-1);
			tag = attr = tagparam = ignor_param = NULL;
			break;
		default:
			DEFAULT_MARK:
			if (bukva<=15) bukva=' ';
			line_len = strlen(#line);
			if (!style.pre) && (bukva == ' ')
			{
				if (line[line_len-1]==' ') break; //no double spaces
				if (!stolbec) && (!line) break; //no paces at the beginning of the line
			}
			if (line_len < sizeof(line)) chrcat(#line, bukva);
			CheckForLineBreak();
		}
	}
	DrawStyle();
	NewLine();
	if (list.first == 0) list.count = draw_y;
	DrawPage();
}
//============================================================================================
bool TWebBrowser::CheckForLineBreak()
{
	int line_break_pos;
	char new_line_text[4096];
	if (strlen(#line)*zoom + stolbec < list.column_max) return false;
	line_break_pos = strrchr(#line, ' ');
	if (line_break_pos*zoom + stolbec > list.column_max) {
		line_break_pos = list.column_max/zoom - stolbec;
		while(line_break_pos) && (line[line_break_pos]!=' ') line_break_pos--;
	}
	if (!line_break_pos) && (strlen(#line)*zoom>list.column_max) {
		line_break_pos=list.column_max/zoom; 
		if (!stolbec)&&(style.pre) draw_y-=list.item_h; //hack to fix https://prnt.sc/rk3kyt
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
	int left1 = body_magrin + list.x;
	int meta_encoding;
	if (istag("html")) {
		t_html = opened;
		return;
	}
	if(istag("title")) {
		if (opened) header=NULL;
		return;
	}
	
	IF(istag("q"))
	{
		if (opened)	{
			meta_encoding = strlen(#line);
			if (line[meta_encoding-1] != ' ') chrcat(#line, ' ');
			chrcat(#line, '\"');
		}
		if (!opened) strcat(#line, "\" ");
		return;
	}
	if (isattr("id=")) || (isattr("name=")) { // TO FIX: works only if the param is the last
		anchors.add(#val, draw_y);
	}	
	if (istag("body")) {
		t_body = opened;
		do{
			if (isattr("link=")) link_color_inactive = GetColor(#val);
			if (isattr("alink=")) link_color_active = GetColor(#val);
			if (isattr("text=")) text_colors[0]=GetColor(#val);
			if (isattr("bgcolor="))
			{
				style.bg_color = page_bg = GetColor(#val);
				DrawBuf.Fill(0, page_bg);
			}
		} while(GetNextParam());
		if (opened) && (cur_encoding==CH_NULL) {
			cur_encoding = CH_CP866; 
			//BufEncode(CH_UTF8);
			debugln("Document has no information about encoding!");
		}
		if (opened) {
			if (strcmp(#header, #version) != 0) {
				ChangeCharset(charsets[cur_encoding], "CP866", #header);
				sprintf(#header, "%s - %s", #header, #version);
			}
			DrawTitle(#header);
		}
		return;
	}
	if (istag("a")) {
		if (opened)
		{
			if (link) IF(text_color_index > 0) text_color_index--; //åñëè ïðåäûäóùèé òåã à íå áûë çàêðûò
			do{
				if (isattr("href=")) && (!strstr(#val,"javascript:"))
				{
					text_color_index++;
					text_colors[text_color_index] = text_colors[text_color_index-1];
					link = 1;
					text_colors[text_color_index] = link_color_inactive;
					PageLinks.AddLink(#val);
				}
			} while(GetNextParam());
		}
		else {
			link = 0;
			IF(text_color_index > 0) text_color_index--;
		}
		return;
	}
	if (istag("font")) {
		style.bg_color = page_bg;
		if (opened)
		{
			text_color_index++;
			text_colors[text_color_index] = text_colors[text_color_index-1];
			do{
				if (isattr("color=")) text_colors[text_color_index] = GetColor(#val);
				if (isattr("bg=")) style.bg_color = GetColor(#val);
			} while(GetNextParam());
		}
		else if (text_color_index > 0) text_color_index--;
		return;
	}
	if (istag("div")) {
		if (streq(#oldtag,"div")) && (opened) return;
		NewLine();
		//IF (oldtag[0] != 'h') 
		return;
	}
	if (istag("header")) || (istag("article")) || (istag("footer")) || (istag("figure")) {
		NewLine();
		return;
	}
	if (istag("p")) {
		IF (oldtag[0] == 'h') || (streq(#oldtag,"td")) || (streq(#oldtag,"p")) return;
		NewLine();
		//IF(opened) NewLine();
		return;
	}
	if (istag("br")) { NewLine(); return; }
	if (istag("tr")) { if (opened) NewLine(); return; }
	if (istag("b")) || (istag("strong")) || (istag("big")) { style.b = opened; return; }
	if (istag("button")) { style.button = opened; stolbec++; return; }
	if (istag("u")) || (istag("ins")) { style.u=opened; return;}
	if (istag("s")) || (istag("strike")) || (istag("del")) { style.s=opened; return; }
	if (istag("dd")) { stolbec += 5; return; }
	if (istag("blockquote")) { style.blq = opened; return; }
	if (istag("pre")) || (istag("code")) { style.pre = opened; return; }
	if (istag("img")) {
		do{
			if (isattr("src=")) strlcpy(#img_path, #val, sizeof(img_path)-1);
			if (isattr("title=")) && (strlen(#val)<sizeof(line)-3) && (val) sprintf(#line, "[%s]", #val); 
			if (isattr("alt=")) && (strlen(#val)<sizeof(line)-3) && (val) sprintf(#line, "[%s]", #val); 
		} while(GetNextParam());
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
		//ImgCache.Images( left1, draw_y, WB1.list.w); 
		return; 
	}
	if (istag("h1")) || (istag("h2")) || (istag("h3")) || (istag("caption")) {
		style.h = opened;
		if (opened)
		{
			NewLine();
			draw_y += 10;
			WB1.zoom=2;
			WB1.list.font_type |= 10011001b;
			if (isattr("align=")) && (isval("center")) style.align = ALIGN_CENTER;
			if (isattr("align=")) && (isval("right")) style.align = ALIGN_RIGHT;
			list.item_h = basic_line_h * 2;
			if (istag("h1")) style.b = true;
		}
		else
		{
			if (istag("h1")) style.b = false;
			NewLine();
			WB1.zoom=1;
			WB1.list.font_type = 10011000b;
			style.align = ALIGN_LEFT;
			list.item_h = basic_line_h;
		}
		return;
	}
	if (istag("dt")) {
		style.li = opened;
		if (opened) NewLine();
		return;
	}
	if (istag("li")) || (istag("dt"))
	{
		style.li = opened;
		if (opened)
		{
			NewLine();
			stolbec = style.li_tab * 5 - 2;
			strcpy(#line, "\31 ");
			//stolbec-=2;
		}
		return;
	}
	if (istag("ul")) || (istag("ol")) {
		if (!opened)
		{
			style.li = opened;
			style.li_tab--;
			NewLine();
		} 
		else style.li_tab++;
	}
	if (istag("hr")) {
		if (isattr("color=")) EDI = GetColor(#val); else EDI = 0x999999;
		$push edi;
		NewLine();
		$pop edi;
		draw_y += 10;
		DrawBuf.DrawBar(5, draw_y - 1, list.w-10, 1, EDI);
		NewLine();
		draw_y += 10;
		return;
	}
	if (istag("meta")) || (istag("?xml")) {
		meta_encoding = CH_NULL;
		do{
			if (isattr("charset=")) || (isattr("content=")) || (isattr("encoding="))
			{
				strcpy(#val, #val[strrchr(#val, '=')]); //search in content=
				strlwr(#val);
				if      (isval("utf-8"))        || (isval("utf8"))        meta_encoding = CH_UTF8;
				else if (isval("koi8-r"))       || (isval("koi8-u"))      meta_encoding = CH_KOI8;
				else if (isval("windows-1251")) || (isval("windows1251")) meta_encoding = CH_CP1251;
				else if (isval("iso-8859-5"))   || (isval("iso8859-5"))   meta_encoding = CH_ISO8859_5;
				else if (isval("dos"))          || (isval("cp-866"))      meta_encoding = CH_CP866;
			}
		} while(GetNextParam());
		if (meta_encoding!=CH_NULL) BufEncode(meta_encoding);
		return;
	}
}
//============================================================================================
void TWebBrowser::BufEncode(dword set_new_encoding)
{
	if (cur_encoding == set_new_encoding) return;
	if (o_bufpointer==0)
	{
		o_bufpointer = malloc(bufsize);
		strcpy(o_bufpointer, bufpointer);
	}
	else
	{
		strcpy(bufpointer, o_bufpointer);
	}
	//debugval("cur_encoding    ", cur_encoding);
	//debugval("set_new_encoding", set_new_encoding);
	cur_encoding = set_new_encoding;
	bufpointer = ChangeCharset(charsets[cur_encoding], "CP866", bufpointer);
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

	if (!stolbec) && (draw_y==body_magrin) return;
	
	if (style.li) && (stolbec == style.li_tab * 5) { 
		if (empty_line<1) empty_line++;
		else return;
	} else if (!stolbec) { 
		if (empty_line<1) empty_line++;
		else return;
	} else {
		empty_line=0;
	}

	onleft = list.x + body_magrin;
	ontop = draw_y + list.y;
	if (t_html) && (!t_body) return;
	draw_y += list.item_h;
	if (style.blq) stolbec = 6; else stolbec = 0;
	if (style.li) stolbec = style.li_tab * 5;
	if (debug_mode) debugln(NULL);
}
//============================================================================================
bool istag(dword text) { if (!strcmp(#tag,text)) return true; else return false; }
bool isattr(dword text) { if (!strcmp(#attr,text)) return true; else return false; }
bool isval(dword text) { if (!strcmp(#val,text)) return true; else return false; }
//============================================================================================
void TWebBrowser::DrawPage()
{
	PutPaletteImage(list.first * DrawBuf.bufw * 4 + buf_data+8, DrawBuf.bufw, list.h, DrawBuf.bufx, DrawBuf.bufy, 32, 0);	
	DrawScroller();
}