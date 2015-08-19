dword bufpointer;
dword o_bufpointer;
dword bufsize;

scroll_bar scroll_wv = { 15,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

enum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT};

struct _style {
byte
	b, i, u, s,
	pre,
	blq,
	li,
	li_tab,
	align;
};

struct TWebBrowser {
	llist list;
	_style style;
	dword draw_line_width;
	DrawBufer DrawBuf;
	void Parse();
	void SetTextStyle();
	void DrawPage();
	void DrawScroller();
	void NewLine();
	void Perenos();
	byte end_parsing;
} WB1;



byte 	
	link,
	ignor_text,
	cur_encoding,
	t_html,
	t_body;
/*
struct _condition {
byte
	text_active,
	text_val,
	href,
	max
} condition;
*/

byte condition_text_active, condition_text_val, condition_href, condition_max;


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
char attr[1200];
char val[4096];
char anchor[256];

#include "..\TWB\history.h"
#include "..\TWB\links.h"
#include "..\TWB\colors.h"
#include "..\TWB\unicode_tags.h"
#include "..\TWB\img_cache.h"
#include "..\TWB\parce_tag.h"
#include "..\TWB\table.h"


//=======================================================================


void TWebBrowser::DrawPage()
{
	int start_x, start_y, line_length, stolbec_len, magrin_left=5;
	
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
	
	if (stroka >= 0) && (stroka - 2 < list.visible) && (line) && (!anchor)
	{
		start_x = stolbec * list.font_w + magrin_left * DrawBuf.zoom + list.x;
		start_y = stroka * list.line_h + magrin_left + list.y;
		stolbec_len = utf8_strlen(#line);
		line_length = stolbec_len * list.font_w * DrawBuf.zoom;

		WriteBufText(start_x, 0, list.font_type, text_colors[text_color_index], #line, buf_data);
		if (style.b)	WriteBufText(start_x+1, 0, list.font_type, text_colors[text_color_index], #line, buf_data);
		if (style.i) { stolbec++; DrawBuf.Skew(start_x, 0, line_length, list.line_h); } // bug with zoom>1
		if (style.s) DrawBuf.DrawBar(start_x, list.line_h / 2 - DrawBuf.zoom, line_length, DrawBuf.zoom, text_colors[text_color_index]);
		if (style.u) DrawBuf.DrawBar(start_x, list.line_h - DrawBuf.zoom - DrawBuf.zoom, line_length, DrawBuf.zoom, text_colors[text_color_index]);
		if (link) {
			DrawBuf.DrawBar(start_x, list.line_h - DrawBuf.zoom - DrawBuf.zoom, line_length, DrawBuf.zoom, text_colors[text_color_index]);
			UnsafeDefineButton(start_x-2, start_y-1, line_length + 3, DrawBuf.zoom * list.font_h, PageLinks.count + 400 + BT_HIDE, 0xB5BFC9);
			PageLinks.AddText(#line, line_length, list.line_h, UNDERLINE);
		}
		stolbec += stolbec_len;
	}
}
//=======================================================================


void TWebBrowser::Parse(){
	word bukva[2];
	int j;
	byte ignor_param;
	char temp[768];
	dword bufpos = bufpointer;
	int line_len;
	
	style.b = style.i = style.u = style.s = style.blq = t_html = t_body =
	style.li = link = ignor_text = text_color_index = text_colors[0] = 
	style.li_tab = condition_text_val = condition_text_active = 0; //обнуляем теги
	end_parsing = false;
	condition_max = 255;
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

	draw_line_width = list.w * DrawBuf.zoom;

	style.pre = 1;
	if (strstri(bufpointer, "html")) style.pre = 0;
	
	for ( ; (bufpointer+bufsize > bufpos) && (ESBYTE[bufpos]!=0); bufpos++;)
	{
		if (end_parsing) break;
		bukva = ESBYTE[bufpos];
		if (ignor_text) && (bukva!='<') continue;
		if (condition_text_active) && (condition_text_val != condition_href) && (bukva!='<') continue;
		switch (bukva)
		{
		case 0x0a:
			if (style.pre)
			{
				chrcat(#line, ' ');
				bukva = temp = NULL;
				Perenos();
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
			bufpos++; //промотаем символ <
			tag = attr = tagparam = ignor_param = NULL;
			if (ESBYTE[bufpos] == '!') //фильтрация внутри <!-- -->, дерзко
			{
				bufpos++;
				if (ESBYTE[bufpos] == '-')
				{
				HH_:
					do
					{
						bufpos++;
						if (bufpointer + bufsize <= bufpos) break 2;
					}
					while (ESBYTE[bufpos] <>'-');
					
					bufpos++;
					if (ESBYTE[bufpos] <>'-') goto HH_;
				}
			}
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

			if (condition_text_active) && (condition_text_val != condition_href) 
			{
				if (strcmp(#tag, "/condition")!=0) break;
			}
			if (tag[strlen(#tag)-1]=='/') tag[strlen(#tag)-1]=NULL; //for br/
			if (tagparam) GetNextParam();

			if (stolbec + utf8_strlen(#line) > list.column_max) Perenos();
			DrawPage();
			line = NULL;
			if (tag) SetTextStyle(WB1.DrawBuf.zoom * 5 + list.x, stroka * list.line_h + list.y + 5); //обработка тегов
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
			if (stolbec + line_len > list.column_max) Perenos();
		}
	}
	DrawPage();
	NewLine();
	DrawBar(list.x, stroka * list.line_h + list.y + 5, draw_line_width, -stroka * list.line_h + list.h - 5, bg_color);
	DrawBar(list.x, list.visible * list.line_h + list.y + 4, draw_line_width, -list.visible * list.line_h + list.h - 4, bg_color);
	if (list.first == 0) list.count = stroka;
	if (anchor) //если посреди текста появится новый якорь - будет бесконечный цикл
	{
		anchor=NULL;
		list.first=anchor_line_num;
		Parse();
	}
	DrawScroller();
}

void TWebBrowser::Perenos()
{
	int perenos_num;
	char new_line_text[4096];
	perenos_num = strrchr(#line, ' ');
	if (!perenos_num) && (utf8_strlen(#line)>list.column_max) perenos_num=list.column_max;
	strcpy(#new_line_text, #line + perenos_num);
	line[perenos_num] = 0x00;
	if (stroka-1 > list.visible) && (list.first <>0) end_parsing=true;
	DrawPage();
	strcpy(#line, #new_line_text);
	NewLine();
}


char oldtag[100];
void TWebBrowser::SetTextStyle(int left1, top1) {
	dword hr_color;
	byte opened;
	byte meta_encoding;
	//проверяем тег открывается или закрывается
	if (tag[0] == '/') 
	{
		 opened = 0;
		 strcpy(#tag, #tag+1);
	}
	else opened = 1;
		
	if (istag("html"))
	{
		t_html = opened;
		return;
	}

	if (istag("script")) || (istag("style")) || (istag("binary")) || (istag("select")) ignor_text = opened;
	if (istag("form")) if (!opened) ignor_text = false;

	if(istag("title"))
	{
		if (opened) header=NULL;
		else if (!stroka) DrawTitle(#header); //тег закрылся - вывели строку
		return;
	}

	if (ignor_text) return;
	
	IF(istag("q"))
	{
		if (opened)	strcat(#line, " \"");
		if (!opened) strcat(#line, "\" ");
	}

	if (anchor) && (isattr("id=")) //очень плохо!!! потому что если не последний тег, работать не будет
	{
		if (!strcmp(#anchor, #val))	anchor_line_num=list.first+stroka;
	}
	
	if (istag("body"))
	{
		t_body = opened;
		do{
			if (isattr("condition_max=")) condition_max = atoi(#val);
			if (isattr("link=")) link_color_inactive = GetColor(#val);
			if (isattr("alink=")) link_color_active = GetColor(#val);
			if (isattr("text=")) text_colors[0]=GetColor(#val);
			if (isattr("bgcolor="))
			{
				bg_color=GetColor(#val);
				DrawBuf.Fill(bg_color);
			}
		} while(GetNextParam());
		if (opened) && (cur_encoding==CH_NULL)
		{
			debugln("Document has no information about encoding, UTF will be used");
			//BufEncode(CH_UTF8);
		}
		return;
	}

	if (istag("a"))
	{
		if (opened)
		{
			if (link) IF(text_color_index > 0) text_color_index--; //если предыдущий тег а не был закрыт

			do{
				if (isattr("href="))
				{
					if (stroka - 1 > list.visible) || (stroka < -2) return;
					
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

	if (istag("font"))
	{
		if (opened)
		{
			text_color_index++;
			text_colors[text_color_index] = text_colors[text_color_index-1];
		
			do{
				if (strcmp(#attr, "color=") == 0) //&& (attr[1] == '#')
				{
					text_colors[text_color_index] = GetColor(#val);
				}
			} while(GetNextParam());
		}
		else
			if (text_color_index > 0) text_color_index--;
		return;
	}
	if (istag("br")) {
		NewLine();
		return;
	}
	if (istag("div")) || (istag("header")) || (istag("article")) || (istag("footer")) {
		IF(oldtag[0] <>'h') NewLine();
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

	if(istag("table")) {
		table.active = opened;
		NewLine();
		if (opened)	table.NewTable();
	}

	if(istag("td")) {
		if (opened)
		{
			table.cur_col++;
			table.row_h = 0;
			do {
				if (isattr("width="))
				{
					table.col_w[table.cur_col] = atoi(#val);
					// NewLine();
					// strcpy(#line, #val);
					// NewLine();
				}
			} while(GetNextParam());
		}
		else
		{
			if (table.row_h > table.row_max_h) table.row_max_h = table.row_h;
		}
	}

	if(istag("tr")) {
		if (opened)
		{
			table.cur_col = 0;
			table.row_max_h = 0;
			table.row_start = stroka;
		}
		else
		{
			NewLine();
			if (table.cur_row == 0) table.max_cols = table.cur_col;
			table.cur_row++;
			table.max_cols = table.cur_col;
		}
	}

	/*
	if (istag("center"))
	{
		if (opened) style.align = ALIGN_CENTER;
		if (!opened)
		{
			NewLine();
			style.align = ALIGN_LEFT;
		}
		return;
	}
	if (istag("right"))
	{
		if (opened) style.align = ALIGN_RIGHT;
		if (!opened)
		{
			NewLine();
			style.align = ALIGN_LEFT;
		}
		return;
	}
	*/
	if (istag("h1")) || (istag("h2")) || (istag("h3")) || (istag("h4")) {
		NewLine();
		if (opened) && (stroka>1) NewLine();
		strcpy(#oldtag, #tag);
		if (opened)
		{
			if (isattr("align=")) && (isval("center")) style.align = ALIGN_CENTER;
			if (isattr("align=")) && (isval("right")) style.align = ALIGN_RIGHT;
			style.b = 1;
		}
		if (!opened)
		{
			style.align = ALIGN_LEFT;
			style.b = 0;
		}
		return;
	}
	else
		oldtag=NULL;
		
	if (istag("b")) || (istag("strong")) || (istag("big")) {
		style.b = opened;
		return;
	}
	if(istag("i")) || (istag("em")) || (istag("subtitle")) {
		style.i = opened;
		return;
	}	
	if (istag("dt"))
	{
		style.li = opened;
		IF(opened == 0) return;
		NewLine();
		return;
	}
	if (istag("condition"))
	{
		condition_text_active = opened;
		if (opened) && (isattr("show_if=")) condition_text_val = atoi(#val);
		return;
	}
	if (istag("li")) || (istag("dt")) //надо сделать вложенные списки
	{
		style.li = opened;
		if (opened)
		{
			NewLine();
			if (stroka > -1) && (stroka - 2 < list.visible) 
				DrawBuf.DrawBar(style.li_tab * 5 * list.font_w * DrawBuf.zoom + list.x, list.line_h / 2 - DrawBuf.zoom - DrawBuf.zoom, DrawBuf.zoom*2, DrawBuf.zoom*2, 0x555555);
		}
		return;
	}
	if (istag("u")) || (istag("ins")) style.u = opened;
	if (istag("s")) || (istag("strike")) || (istag("del")) style.s = opened;
	if (istag("ul")) || (istag("ol")) IF(!opened)
	{
		style.li = opened;
		style.li_tab--;
		NewLine();
	} ELSE style.li_tab++;
	if (istag("dd")) stolbec += 5;
	if (istag("blockquote")) style.blq = opened;
	if (istag("pre")) || (istag("code")) style.pre = opened; 
	if (istag("hr"))
	{
		if (anchor) || (stroka < -1)
		{
			stroka+=2;
			return;
		}
		if (strcmp(#attr, "color=") == 0) hr_color = GetColor(#val); else hr_color = 0x999999;
		NewLine();
		DrawBuf.DrawBar(5, list.line_h/2, list.w-10, 1, hr_color);
		NewLine();
	}
	if (istag("img"))
	{
		ImgCache.Images( left1, top1, WB1.list.w);
		return;
	}
	if (istag("meta")) || (istag("?xml"))
	{
		do{
			if (isattr("charset=")) || (isattr("content=")) || (isattr("encoding="))
			{
				strcpy(#val, #val[strrchr(#val, '=')]); //поиск в content=
				strlwr(#val);
				meta_encoding = CH_NULL;
				if (isval("utf-8"))             || (isval("utf8"))        meta_encoding = CH_UTF8;
				else if (isval("koi8-r"))       || (isval("koi8-u"))      meta_encoding = CH_KOI8;
				else if (isval("windows-1251")) || (isval("windows1251")) meta_encoding = CH_CP1251;
				else if (isval("windows-1252")) || (isval("windows1252")) meta_encoding = CH_CP1252;
				else if (isval("iso-8859-5"))   || (isval("iso8859-5"))   meta_encoding = CH_ISO8859_5;
				else if (isval("dos"))          || (isval("cp-866"))      meta_encoding = CH_CP866;
				if (cur_encoding==CH_NULL) BufEncode(meta_encoding);
				return;
			}
		} while(GetNextParam());
		return;
	}
}

void BufEncode(int set_new_encoding)
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
	if (set_new_encoding == CH_CP1251)
	{
		 bufpointer = ChangeCharset("CP1251", "UTF-8", bufpointer);
	}
}


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


void TWebBrowser::NewLine()
{
	int onleft, ontop;

	onleft = list.x + 5;
	ontop = stroka * list.line_h + list.y + 5;
	if (!stroka) DrawBar(list.x, list.y, draw_line_width, 5, bg_color);
	if (t_html) && (!t_body) return;
	if (stroka * list.line_h + 5 >= 0) && ( stroka + 1 * list.line_h + 5 < list.h) && (!anchor)
	{
		if (style.align == ALIGN_CENTER) && (DrawBuf.zoom==1) DrawBuf.AlignCenter(onleft,ontop,list.w,list.line_h,stolbec * list.font_w);
		if (style.align == ALIGN_RIGHT) && (DrawBuf.zoom==1)  DrawBuf.AlignRight(onleft,ontop,list.w,list.line_h,stolbec * list.font_w);
		DrawBuf.bufy = ontop;
		DrawBuf.Show();
		DrawBuf.Fill(bg_color);
	}
	stroka++;
	if (style.blq) stolbec = 6; else stolbec = 0;
	if (style.li) stolbec = style.li_tab * 5;
}



int istag(dword text) { if (!strcmp(#tag,text)) return 1; else return 0; }
int isattr(dword text) { if (!strcmp(#attr,text)) return 1; else return 0; }
int isval(dword text) { if (!strcmp(#val,text)) return 1; else return 0; }

