

scroll_bar scroll_wv = { 15,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

enum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT};

struct _style {
byte
	b, i, u, s, h,
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

int stroka;
int stolbec;
int tab_len;
int anchor_line_num;

char URL[10000];
char header[2048];
char line[500];
char tagparam[10000];
char tag[100];
char oldtag[100];
char attr[1200];
char val[4096];
char anchor[256]=0;

#include "..\TWB\history.h"
#include "..\TWB\links.h"
#include "..\TWB\colors.h"
#include "..\TWB\unicode_tags.h"
#include "..\TWB\img_cache.h"
#include "..\TWB\parce_tag.h"


//============================================================================================
void TWebBrowser::DrawStyle()
{
	int start_x, start_y, line_length, stolbec_len, body_magrin=5;
	
	if (!header)
	{
		ChangeCharset("UTF-8", "CP866", #line);
		strcpy(#header, #line);
		strcat(#header, " -");
		strcat(#header, #version);
		line = 0;
		return;
	}
	if (t_html) && (!t_body) return;
	
	if (line) && (!anchor)
	{
		start_x = stolbec * list.font_w + body_magrin * DrawBuf.zoom + list.x;
		start_y = stroka * list.item_h + body_magrin;
		stolbec_len = utf8_strlen(#line) * DrawBuf.zoom;
		line_length = stolbec_len * list.font_w;

		if (style.h) stroka++;
		WriteBufText(start_x, start_y, list.font_type, text_colors[text_color_index], #line, buf_data);
		if (style.b) WriteBufText(start_x+1, start_y, list.font_type, text_colors[text_color_index], #line, buf_data);
		if (style.i) { stolbec++; DrawBuf.Skew(start_x, start_y, line_length, list.item_h); } // bug with zoom>1
		if (style.s) DrawBuf.DrawBar(start_x, list.item_h / 2 - DrawBuf.zoom + start_y, line_length, DrawBuf.zoom, text_colors[text_color_index]);
		if (style.u) DrawBuf.DrawBar(start_x, list.item_h - DrawBuf.zoom - DrawBuf.zoom + start_y, line_length, DrawBuf.zoom, text_colors[text_color_index]);
		if (link) {
			DrawBuf.DrawBar(start_x, list.item_h*style.h + list.item_h - DrawBuf.zoom - DrawBuf.zoom + start_y, line_length, DrawBuf.zoom, text_colors[text_color_index]);
			PageLinks.AddText(#line, line_length, list.item_h*style.h + list.item_h, UNDERLINE);
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
	int j;
	byte ignor_param;
	dword bufpos;
	int line_len;	
	style.b = style.i = style.u = style.s = style.h = style.blq = t_html = t_body =
	style.li = link = ignor_text = text_color_index = text_colors[0] = style.li_tab = 0;
	style.align = ALIGN_LEFT;
	link_color_inactive = 0x0000FF;
	link_color_active = 0xFF0000;
	bg_color = 0xFFFFFF;
	DrawBuf.Fill(bg_color);
	PageLinks.Clear();
	strcpy(#header, #version);
	stroka = -list.first;
	stolbec = 0;
	line = 0;
	//for plaint text use CP866 for other UTF
	if (strstri(bufpointer, "html")) 
	{
		style.pre = 0;
		cur_encoding = CH_UTF8;
	}
	else
	{
		style.pre = 1;
		cur_encoding = CH_CP866;
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
				NewLine();
				break;
			}
		case '\9':
			if (style.pre) //иначе идём на 0x0d	
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
				if (!strncmp(bufpos,"-->",3)) || (bufpointer + bufsize <= bufpos) break;
				bufpos++;
			}
			tag = attr = tagparam = ignor_param = NULL;
			while (ESBYTE[bufpos] !='>') && (bufpos < bufpointer + bufsize) //получаем тег и его параметры
			{
				bukva = ESBYTE[bufpos];
				if (bukva == '\9') || (bukva == '\x0a') || (bukva == '\x0d') bukva = ' ';
				if (!ignor_param) && (bukva <>' ')
				{
					if (strlen(#tag)<sizeof(tag)) chrcat(#tag, bukva);
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
			if (tag) SetStyle(); //обработка тегов
			strcpy(#oldtag, #tag);
			tag = attr = tagparam = ignor_param = NULL;
			break;
		default:
			DEFAULT_MARK:
			if (bukva<=15) bukva=' ';
			line_len = utf8_strlen(#line);
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
	DrawPage();
	if (list.first == 0) list.count = stroka;
	if (anchor) //если посреди текста появится новый якорь - будет бесконечный цикл
	{
		anchor=NULL;
		list.first=anchor_line_num;
		Prepare();
	}
}
//============================================================================================
void TWebBrowser::Perenos()
{
	int perenos_num;
	char new_line_text[4096];
	if (utf8_strlen(#line)*DrawBuf.zoom + stolbec < list.column_max) return;
	perenos_num = strrchr(#line, ' ');
	if (!perenos_num) && (utf8_strlen(#line)*DrawBuf.zoom>list.column_max) perenos_num=list.column_max/DrawBuf.zoom;
	strcpy(#new_line_text, #line + perenos_num);
	line[perenos_num] = 0x00;
	DrawStyle();
	strcpy(#line, #new_line_text);
	NewLine();
}
//============================================================================================
void TWebBrowser::SetStyle() {
	int left1 = 5 + list.x;
	int top1 = stroka * list.item_h + list.y + 5;
	byte opened;
	byte meta_encoding;
	//проверяем тег открывается или закрывается
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
		else if (!stroka) DrawTitle(#header); //тег закрылся - вывели строку
		return;
	}
	if (ignor_text) return;
	
	IF(istag("q"))
	{
		if (opened)	strcat(#line, " \"");
		if (!opened) strcat(#line, "\" ");
		return;
	}
	if (anchor) && (isattr("id=")) { //очень плохо!!! потому что если не последний тег, работать не будет
		if (!strcmp(#anchor, #val))	anchor_line_num=list.first+stroka;
	}	
	if (istag("body")) {
		t_body = opened;
		do{
			if (isattr("link=")) link_color_inactive = GetColor(#val);
			if (isattr("alink=")) link_color_active = GetColor(#val);
			if (isattr("text=")) text_colors[0]=GetColor(#val);
			if (isattr("bgcolor="))
			{
				bg_color=GetColor(#val);
				DrawBuf.Fill(bg_color);
			}
		} while(GetNextParam());
		if (opened) && (cur_encoding==CH_NULL) debugln("Document has no information about encoding, UTF will be used");
		return;
	}
	if (istag("a")) {
		if (opened)
		{
			if (link) IF(text_color_index > 0) text_color_index--; //если предыдущий тег а не был закрыт
			do{
				if (isattr("href="))
				{
					text_color_index++;
					text_colors[text_color_index] = text_colors[text_color_index-1];
					link = 1;
					text_colors[text_color_index] = link_color_inactive;
					PageLinks.AddLink(#val, DrawBuf.zoom * stolbec * list.font_w + left1, top1-DrawBuf.zoom);
				}
				if (anchor) && (isattr("name="))
				{
					if (!strcmp(#anchor, #val))
					{
						anchor_line_num=list.first+stroka;
					}
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
		if (isattr("bgcolor="))
		{
			bg_color=GetColor(#val);
			DrawBuf.Fill(bg_color);
		}
		return;
	}
	if (istag("p")) {
		IF(oldtag[0] == 'h') return;
		NewLine();
		IF(opened) NewLine();
		return;
	}
	if (istag("br")) { NewLine(); return; }
	if (istag("tr")) { if (opened) { NewLine(); strcat(#line, "| "); } return; }
	if (istag("td")) || (istag("th")) { if (!opened) strcat(#line, " | "); return; }
	if (istag("b")) || (istag("strong")) || (istag("big")) { style.b = opened; return; }
	if (istag("i")) || (istag("em")) || (istag("subtitle")) { style.i=opened; return; }
	if (istag("u")) || (istag("ins")) { style.u=opened; return;}
	if (istag("s")) || (istag("strike")) || (istag("del")) { style.s=opened; return; }
	if (istag("dd")) { stolbec += 5; return; }
	if (istag("blockquote")) { style.blq = opened; return; }
	if (istag("pre")) || (istag("code")) { style.pre = opened; return; }
	if (istag("img")) { ImgCache.Images( left1, top1, WB1.list.w); return; }
	if (istag("h1")) || (istag("h2")) || (istag("h3")) || (istag("h4")) || (istag("caption")) {
		style.h = opened;
		NewLine();
		if (opened)
		{
			WB1.DrawBuf.zoom=2;
			WB1.list.font_type |= 10111001b;
			if (isattr("align=")) && (isval("center")) style.align = ALIGN_CENTER;
			if (isattr("align=")) && (isval("right")) style.align = ALIGN_RIGHT;
			if (stroka>1) NewLine();
		}
		else
		{
			WB1.DrawBuf.zoom=1;
			WB1.list.font_type = 10111000b;
			style.align = ALIGN_LEFT;
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
			DrawBuf.DrawBar(style.li_tab * 5 * list.font_w * DrawBuf.zoom + list.x, stroka +1 * list.item_h - 3
			 - DrawBuf.zoom - DrawBuf.zoom, DrawBuf.zoom*2, DrawBuf.zoom*2, 0x454545);
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
		DrawBuf.DrawBar(5, list.item_h*stroka+4, list.w-10, 1, EDI);
		NewLine();
		return;
	}
	if (istag("meta")) || (istag("?xml")) {
		do{
			if (isattr("charset=")) || (isattr("content=")) || (isattr("encoding="))
			{
				strcpy(#val, #val[strrchr(#val, '=')]); //поиск в content=
				strlwr(#val);
				if      (isval("utf-8"))        || (isval("utf8"))        meta_encoding = CH_UTF8;
				else if (isval("koi8-r"))       || (isval("koi8-u"))      meta_encoding = CH_KOI8;
				else if (isval("windows-1251")) || (isval("windows1251")) meta_encoding = CH_CP1251;
				else if (isval("iso-8859-5"))   || (isval("iso8859-5"))   meta_encoding = CH_ISO8859_5;
				else if (isval("dos"))          || (isval("cp-866"))      meta_encoding = CH_CP866;
				if (cur_encoding!=meta_encoding) BufEncode(meta_encoding);
				return;
			}
		} while(GetNextParam());
		return;
	}
}

void TWebBrowser::BufEncode(int set_new_encoding)
{
	int bufpointer_realsize;
	cur_encoding = set_new_encoding;
	if (o_bufpointer==0)
	{
		o_bufpointer = malloc(bufsize);
		strcpy(o_bufpointer, bufpointer);
	}
	else
	{
		strcpy(bufpointer, o_bufpointer);
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
	scroll_wv.start_x = list.w * DrawBuf.zoom + list.x;

	scrollbar_v_draw(#scroll_wv);
}
//============================================================================================
void TWebBrowser::NewLine()
{
	int onleft, ontop;

	onleft = list.x + 5;
	ontop = stroka * list.item_h + list.y + 5;
	if (t_html) && (!t_body) return;
	if (stroka * list.item_h + 5 >= 0) && ( stroka + 1 * list.item_h + 5 < list.h) && (!anchor)
	{
		if (style.align == ALIGN_CENTER) && (DrawBuf.zoom==1) DrawBuf.AlignCenter(onleft,ontop,list.w,list.item_h,stolbec * list.font_w);
		if (style.align == ALIGN_RIGHT) && (DrawBuf.zoom==1)  DrawBuf.AlignRight(onleft,ontop,list.w,list.item_h,stolbec * list.font_w);
	}
	stroka++;
	if (style.blq) stolbec = 6; else stolbec = 0;
	if (style.li) stolbec = style.li_tab * 5;
}
//============================================================================================
int istag(dword text) { if (!strcmp(#tag,text)) return 1; else return 0; }
int isattr(dword text) { if (!strcmp(#attr,text)) return 1; else return 0; }
int isval(dword text) { if (!strcmp(#val,text)) return 1; else return 0; }
//============================================================================================
void TWebBrowser::DrawPage()
{
	PutPaletteImage(list.first * list.item_h * DrawBuf.bufw * 4 + buf_data+8, DrawBuf.bufw, list.h, DrawBuf.bufx, DrawBuf.bufy, 32, 0);	
	DrawScroller();
}