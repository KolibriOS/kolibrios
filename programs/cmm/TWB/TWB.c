
scroll_bar scroll_wv = { 15,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

enum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT};

struct _style {
byte
	b, u, s, h,
	pre,
	blq,
	li,
	li_tab,
	align;
};

struct TWebBrowser {
	llist list;
	_style style;
	DrawBufer DrawBuf;
	int zoom;
	void Prepare();
	void SetStyle();
	void DrawStyle();
	void DrawPage();
	void DrawScroller();
	void LoadInternalPage();
	void NewLine();
	void Perenos();
	void BufEncode();
} WB1;


byte 	
	link,
	ignor_text,
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
dword bg_color;

int draw_y;
int stolbec;
int tab_len;
int anchor_y;

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

		WriteBufText(start_x, draw_y, list.font_type, text_colors[text_color_index], #line, buf_data);
		if (style.b) WriteBufText(start_x+1, draw_y, list.font_type, text_colors[text_color_index], #line, buf_data);
		if (style.s) DrawBuf.DrawBar(start_x, list.item_h / 2 - zoom + draw_y, line_length, zoom, text_colors[text_color_index]);
		if (style.u) DrawBuf.DrawBar(start_x, list.item_h - zoom - zoom + draw_y, line_length, zoom, text_colors[text_color_index]);
		if (link) {
			DrawBuf.DrawBar(start_x, draw_y + list.item_h - calc(zoom*2), line_length, zoom, text_colors[text_color_index]);
			PageLinks.AddText(start_x, draw_y + list.y, line_length, list.item_h - calc(zoom*2), UNDERLINE, zoom); //TODO: set bigger underline_h for style.h
		}
		stolbec += stolbec_len;
	}
}
//============================================================================================
void TWebBrowser::LoadInternalPage(dword bufpos, in_filesize){
	bufsize = in_filesize;
	bufpointer = bufpos;
	Prepare();
}
//============================================================================================
void TWebBrowser::Prepare(){
	word bukva[2];
	dword j;
	byte ignor_param;
	dword bufpos;
	dword line_len;	
	style.b = style.u = style.s = style.h = style.blq = t_html = t_body =
	style.li = link = ignor_text = text_color_index = text_colors[0] = style.li_tab = 0;
	style.align = ALIGN_LEFT;
	link_color_inactive = 0x0000FF;
	link_color_active = 0xFF0000;
	bg_color = 0xFFFFFF;
	DrawBuf.Fill(bg_color);
	PageLinks.Clear();
	strcpy(#header, #version);
	draw_y = body_magrin;
	stolbec = 0;
	line = 0;
	zoom = 1;
	//for plaint text use CP866 for other UTF
	if (strstri(bufpointer, "html")!=-1) 
	{
		debugln("<html> found");
		style.pre = false;
		cur_encoding = CH_NULL;
	}
	else
	{
		debugln("no <html>");
		style.pre = true;
		cur_encoding = CH_NULL;
	}
	for (bufpos=bufpointer ; (bufpos < bufpointer+bufsize) && (ESBYTE[bufpos]!=0) ; bufpos++;)
	{
		if (ignor_text) && (ESBYTE[bufpos]!='<') continue;
		bukva = ESBYTE[bufpos];
		switch (bukva)
		{
		case 0x0a:
			if (style.pre)
			{
				DrawStyle();
				line = NULL;
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
			bufpos++;
			tag=0;
			for (j=0; (ESBYTE[bufpos]<>';') && (j<7);   j++, bufpos++;)
			{
				bukva = ESBYTE[bufpos];
				chrcat(#tag, bukva);
			}
			if (bukva = GetUnicodeSymbol()) goto DEFAULT_MARK;
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
				bufpos+=3;
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
					if (!ignor_text) && (strlen(#tagparam)+1<sizeof(tagparam)) strcat(#tagparam, #bukva);
				}
				bufpos++;
			}
			strlwr(#tag);

			if (tag[strlen(#tag)-1]=='/') tag[strlen(#tag)-1]=NULL; //for br/
			if (tagparam) GetNextParam();
			Perenos();
			DrawStyle();
			line = NULL;
			if (tag) SetStyle();
			strlcpy(#oldtag, #tag, sizeof(oldtag));
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
			Perenos();
		}
	}
	DrawStyle();
	NewLine();
	if (list.first == 0) list.count = draw_y;
	DrawPage();
}
//============================================================================================
void TWebBrowser::Perenos()
{
	int perenos_num;
	char new_line_text[4096];
	if (strlen(#line)*zoom + stolbec < list.column_max) return;
	perenos_num = strrchr(#line, ' ');
	if (!perenos_num) && (strlen(#line)*zoom>list.column_max) perenos_num=list.column_max/zoom;
	strcpy(#new_line_text, #line + perenos_num);
	line[perenos_num] = 0x00;
	DrawStyle();
	strcpy(#line, #new_line_text);
	NewLine();
}
//============================================================================================
void TWebBrowser::SetStyle() {
	int left1 = body_magrin + list.x;
	byte opened;
	byte meta_encoding;
	if (tag[0] == '/') 
	{
		 opened = 0;
		 strcpy(#tag, #tag+1);
	}
	else opened = 1;
	if (istag("html")) {
		t_html = opened;
		return;
	}
	if (istag("script")) || (istag("style")) || (istag("binary")) || (istag("select")) { ignor_text = opened; return; }
	if (istag("form")) if (!opened) ignor_text = false;
	if(istag("title")) {
		if (opened) header=NULL;
		return;
	}
	if (ignor_text) return;
	
	IF(istag("q"))
	{
		if (opened)	strcat(#line, " \"");
		if (!opened) strcat(#line, "\" ");
		return;
	}
	//if (isattr("id=")) || (isattr("name=")) { //very bad: if the tag is not the last it wound work
		//add anchor
	//}	
	if (istag("body")) {
		t_body = opened;
		do{
			if (isattr("link=")) link_color_inactive = GetColor(#val);
			if (isattr("alink=")) link_color_active = GetColor(#val);
			if (isattr("text=")) text_colors[0]=GetColor(#val);
			if (isattr("bgcolor="))
			{
				bg_color = GetColor(#val);
				DrawBuf.Fill(bg_color);
			}
		} while(GetNextParam());
		if (opened) && (cur_encoding==CH_NULL) {
			cur_encoding = CH_UTF8; 
			debugln("Document has no information about encoding, UTF will be used");
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
		if (opened)
		{
			text_color_index++;
			text_colors[text_color_index] = text_colors[text_color_index-1];
			do{
				if (isattr("color=")) text_colors[text_color_index] = GetColor(#val);
			} while(GetNextParam());
		}
		else if (text_color_index > 0) text_color_index--;
		return;
	}
	if (istag("div")) || (istag("header")) || (istag("article")) || (istag("footer")) {
		IF(oldtag[0] != 'h') NewLine();
		return;
	}
	if (istag("p")) {
		IF(oldtag[0] == 'h') return;
		NewLine();
		IF(opened) NewLine();
		return;
	}
	if (istag("br")) { NewLine(); return; }
	if (istag("tr")) { if (opened) NewLine(); return; }
	if (istag("b")) || (istag("strong")) || (istag("big")) { style.b = opened; return; }
	if (istag("u")) || (istag("ins")) { style.u=opened; return;}
	if (istag("s")) || (istag("strike")) || (istag("del")) { style.s=opened; return; }
	if (istag("dd")) { stolbec += 5; return; }
	if (istag("blockquote")) { style.blq = opened; return; }
	if (istag("pre")) || (istag("code")) { style.pre = opened; return; }
	if (istag("img")) { ImgCache.Images( left1, draw_y, WB1.list.w); return; }
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
		}
		else
		{
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
			strcpy(#line, "\31 \0");
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
		DrawBuf.DrawBar(5, draw_y - 1, list.w-10, 1, EDI);
		NewLine();
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
	debugval("cur_encoding    ", cur_encoding);
	debugval("set_new_encoding", set_new_encoding);
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

	if (!stolbec) && (draw_y==body_magrin) return;

	onleft = list.x + body_magrin;
	ontop = draw_y + list.y;
	if (t_html) && (!t_body) return;
	draw_y += list.item_h;
	if (style.blq) stolbec = 6; else stolbec = 0;
	if (style.li) stolbec = style.li_tab * 5;
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