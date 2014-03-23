dword bufpointer;
dword o_bufpointer;
dword bufsize;

#define URL param

scroll_bar scroll_wv = { 15,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

char header[2048];

struct TWebBrowser {
	llist list;
	DrawBufer DrawBuf;
	void Prepare();
	void Parse();
	void SetTextStyle();
	void DrawPage();
	void DrawScroller();
	void NewLine();
} WB1;

byte b_text, i_text, u_text, s_text, pre_text, blq_text, li_text, li_tab, 
	link, ignor_text, cur_encoding, text_align, t_html, t_body;
byte condition_text_active, condition_text_val, condition_href, condition_max;

enum { _WIN, _DOS, _KOI, _UTF, _DEFAULT };

enum { ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT};

dword text_colors[300];
dword text_color_index;
dword link_color_inactive;
dword link_color_active;
dword bg_color;

int stroka;
int stolbec;
int tab_len;
int anchor_line_num;

char line[500];
char tag[100];
char tagparam[10000];
char parametr[1200];
char options[4096];
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
	int start_x, start_y, line_length, magrin_left=5;
	
	if (!header)
	{
		strcpy(#header, #line);
		strcat(#header, " -");
		strcat(#header, #version);
		line = 0;
		return;
	}
	if (t_html) && (!t_body) return;
	
	if (stroka >= 0) && (stroka - 2 < list.visible) && (line) && (!anchor)
	{
		start_x = stolbec * 6 + list.x + magrin_left;
		start_y = stroka * 10 + list.y + magrin_left;
		line_length = strlen(#line) * 6;

		WriteBufText(start_x, 0, 0x88, text_colors[text_color_index], #line, buf_data);
		IF (b_text)	WriteBufText(start_x+1, 0, 0x88, text_colors[text_color_index], #line, buf_data);
		IF (i_text) { stolbec++; DrawBuf.Skew(start_x, 0, line_length, list.line_h); }
		IF (s_text) DrawBuf.DrawBar(start_x, 4, line_length, 1, text_colors[text_color_index]);
		IF (u_text) DrawBuf.DrawBar(start_x, 8, line_length, 1, text_colors[text_color_index]);
		IF (link) {
			UnsafeDefineButton(start_x-2, start_y, line_length + 3, 9, PageLinks.count + 400 + BT_HIDE, 0xB5BFC9);
			DrawBuf.DrawBar(start_x, 8, line_length, 1, text_colors[text_color_index]);
			PageLinks.AddText(#line, line_length, list.line_h, UNDERLINE);
		}
		stolbec += strlen(#line);
	}
}
//=======================================================================


void BufEncode(int set_new_encoding)
{
	int bufpointer_realsize;
	cur_encoding = set_new_encoding;
	if (o_bufpointer==0)
	{
		bufpointer_realsize = strlen(bufpointer);
		if (bufpointer_realsize > bufsize)
		{
			debug("bufsize: ");
			debugi(bufsize);
			debug("bufpointer_realsize: ");
			debugi(bufpointer_realsize);
			bufsize = bufpointer_realsize;
		}
		o_bufpointer = malloc(bufsize);
		strcpy(o_bufpointer, bufpointer);
	}
	else
	{
		strcpy(bufpointer, o_bufpointer);
	}
	if (set_new_encoding==_WIN) wintodos(bufpointer);
	if (set_new_encoding==_UTF) utf8rutodos(bufpointer);
	if (set_new_encoding==_KOI) koitodos(bufpointer);
}

void TWebBrowser::Prepare(dword bufpos, in_filesize){
	bufsize = in_filesize;
	bufpointer = bufpos;
	Parse();
}


void TWebBrowser::Parse(){
	word bukva[2];
	int j, perenos_num;
	byte ignor_param;
	char temp[768];
	dword bufpos = bufpointer;
	
	b_text = i_text = u_text = s_text = blq_text = t_html = t_body =
	li_text = link = ignor_text = text_color_index = text_colors[0] = li_tab = 
	condition_text_val = condition_text_active = 0; //обнуляем теги
	condition_max = 255;
	text_align = ALIGN_LEFT;
	link_color_inactive = 0x0000FF;
	link_color_active = 0xFF0000;
	bg_color = 0xFFFFFF;
	DrawBuf.Fill(bg_color);
	PageLinks.Clear();
	strcpy(#header, #version);
	stroka = -list.first;
	stolbec = 0;
	line = 0;

	if (pre_text<>2)
	{
		pre_text=0;
		if (!strcmp(#URL + strlen(#URL) - 4, ".txt")) pre_text = 1;
		if (!strcmp(#URL + strlen(#URL) - 4, ".mht")) ignor_text = 1;
	}
	
	for ( ; bufpointer+bufsize > bufpos; bufpos++;)
	{
		bukva = ESBYTE[bufpos];
		if (ignor_text) && (bukva!='<') continue;
		if (condition_text_active) && (condition_text_val != condition_href) && (bukva!='<') continue;
		switch (bukva)
		{
		case 0x0a:
			if (pre_text)
			{
				chrcat(#line, ' ');
				bukva = temp = NULL;
				goto NEXT_MARK;
			}
		case '\9':
			if (pre_text) //иначе идём на 0x0d	
			{
				tab_len=strlen(#line)/8;
				tab_len=tab_len*8;
				tab_len=8+tab_len-strlen(#line);
				for (j=0; j<tab_len; j++;) chrcat(#line,' ');
				break;
			}
			goto DEFAULT_MARK;		
		case '=': //quoted printable
			if (strcmp(#URL + strlen(#URL) - 4, ".mht")<>0) goto DEFAULT_MARK;

			temp[0] = ESBYTE[bufpos+1];
			temp[1] = ESBYTE[bufpos+2];
			temp[2] = '\0';
			if (bukva = Hex2Symb(#temp))
			{
				bufpos+=2;
				goto DEFAULT_MARK;
			}
			break;
			
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
			tag = parametr = tagparam = ignor_param = NULL;
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
					if (strlen(#tag)<sizeof(tag)) strcat(#tag, #bukva);
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
			if (tagparam) && (strlen(#tagparam) < 4000) GetNextParam();

			if (stolbec + strlen(#line) > list.column_max) //============the same as NEXT_MARK
			{
				perenos_num = strrchr(#line, ' ');
				if (!perenos_num) && (strlen(#line)>list.column_max) perenos_num=list.column_max;
				strcpy(#temp, #line + perenos_num); //перенос по словам
				line[perenos_num] = 0x00;
				if (stroka-1 > list.visible) && (list.first <>0) break 1; //уходим...
				DrawPage();
				strcpy(#line, #temp);				
				NewLine(list.x + 5, stroka * 10 + list.y + 5); //закрашиваем следущую строку
			}
			DrawPage();
			line = NULL;
			if (tag) SetTextStyle(list.x + 5, stroka * 10 + list.y + 5); //обработка тегов
			tag = parametr = tagparam = ignor_param = NULL;
			break;
		default:
			DEFAULT_MARK:
			if (bukva<=15) bukva=' ';
			if (!pre_text) && (bukva == ' ')
			{
				if (line[strlen(#line)-1]==' ') break; //убрать 2 пробела подряд
				if (!stolbec) && (!line) break; //строка не может начинаться с пробела
			}
			if (strlen(#line)<sizeof(line)) chrcat(#line, bukva);

			if (stolbec + strlen(#line) > list.column_max)
			{
			NEXT_MARK:
				perenos_num = strrchr(#line, ' ');
				if (!perenos_num) && (strlen(#line)>list.column_max) perenos_num=list.column_max;
				strcpy(#temp, #line + perenos_num);
				line[perenos_num] = 0x00;
				if (stroka-1 > list.visible) && (list.first <>0) break 1;
				DrawPage();
				strcpy(#line, #temp);
				NewLine(list.x + 5, stroka * 10 + list.y + 5);
			}
		}
	}

	DrawPage(); //рисует последнюю строку, потом это надо убрать, оптимизировав код
	NewLine(list.x + 5, stroka * 10 + list.y + 5); //закрашиваем следущую строку

	if (list.visible * 10 + 25 <= list.h)
		DrawBar(list.x, list.visible * 10 + list.y + 25, list.w, -list.visible * 10 + list.h - 25, bg_color);
	if (stroka * 10 + 5 <= list.h)
		DrawBar(list.x, stroka * 10 + list.y + 5, list.w, -stroka * 10 + list.h - 5, bg_color); //закрашиваем всё до конца
	if (list.first == 0) list.count = stroka;
	if (anchor) //если посреди текста появится новый якорь - будет бесконечный цикл
	{
		anchor=NULL;
		list.first=anchor_line_num;
		Parse();
	}
	DrawScroller();
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
		
	if (isTag("html"))
	{
		IF(!strcmp(#URL + strlen(#URL) - 4, ".mht")) IF (opened==0) ignor_text = 1; ELSE ignor_text = 0;
		t_html = opened;
		return;
	}

	if (isTag("script")) || (isTag("style")) || (isTag("binary")) || (isTag("select")) ignor_text = opened;

	if(isTag("title"))
	{
		if (opened) header=NULL;
		else if (!stroka) DrawTitle(#header); //тег закрылся - вывели строку
		return;
	}

	if (ignor_text) return;
	
	IF(isTag("q"))
	{
		if (opened)
		{
			NewLine(left1, top1);
			strcat(#line, ' \"');
		}
		if (!opened)
		{
			chrcat(#line, '\"');
			NewLine(left1, top1);
		} 
	}

	if (anchor) && (!strcmp(#parametr, "id=")) //очень плохо!!! потому что если не последний тег, работать не будет
	{
		if (!strcmp(#anchor, #options))	anchor_line_num=list.first+stroka;
	}
	
	if (isTag("body"))
	{
		t_body = opened;
		do{
			if (!strcmp(#parametr, "condition_max=")) condition_max = atoi(#options);
			if (!strcmp(#parametr, "link=")) link_color_inactive = GetColor(#options);
			if (!strcmp(#parametr, "alink=")) link_color_active = GetColor(#options);
			if (!strcmp(#parametr, "text=")) text_colors[0]=GetColor(#options);
			if (!strcmp(#parametr, "bgcolor="))
			{
				bg_color=GetColor(#options);
				DrawBuf.Fill(bg_color);
			}
		} while(GetNextParam());
		if (opened) && (cur_encoding==_DEFAULT)
		{
			debugln("Document has no information about encoding, UTF will be used");
			BufEncode(_UTF);
		}
		return;
	}

	if (isTag("a"))
	{
		if (opened)
		{
			if (link) IF(text_color_index > 0) text_color_index--; //если предыдущий тег а не был закрыт

			do{
				if (!strcmp(#parametr, "href="))
				{
					if (stroka - 1 > list.visible) || (stroka < -2) return;
					
					text_color_index++;
					text_colors[text_color_index] = text_colors[text_color_index-1];
					
					link = 1;
					text_colors[text_color_index] = link_color_inactive;
					PageLinks.AddLink(#options, stolbec*6+left1, top1);
				}
				if (anchor) && (!strcmp(#parametr, "name="))
				{
					if (!strcmp(#anchor, #options))
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

	if (isTag("font"))
	{
		if (opened)
		{
			text_color_index++;
			text_colors[text_color_index] = text_colors[text_color_index-1];
		
			do{
				if (strcmp(#parametr, "color=") == 0) //&& (parametr[1] == '#')
				{
					text_colors[text_color_index] = GetColor(#options);
				}
			} while(GetNextParam());
		}
		else
			if (text_color_index > 0) text_color_index--;
		return;
	}
	if (isTag("br")) {
		NewLine(left1, top1);
		return;
	}
	if (isTag("div")) || (isTag("header")) || (isTag("article")) || (isTag("footer")) {
		IF(oldtag[0] <>'h') NewLine(left1, top1);
		return;
	}
	if (isTag("p")) {
		IF(oldtag[0] == 'h') return;
		NewLine(left1, top1);
		IF(opened) NewLine(left1, top1 + 10);
		return;
	}

	if(isTag("table")) {
		table.active = opened;
		NewLine(left1, top1);
		if (opened)	table.NewTable();
	}

	if(isTag("td")) {
		if (opened)
		{
			table.cur_col++;
			table.row_h = 0;
			do {
				if (!strcmp(#parametr, "width="))
				{
					table.col_w[table.cur_col] = atoi(#options);
					// NewLine(left1, top1);
					// strcpy(#line, #options);
					// NewLine(left1, top1);
				}
			} while(GetNextParam());
		}
		else
		{
			if (table.row_h > table.row_max_h) table.row_max_h = table.row_h;
		}
	}

	if(isTag("tr")) {
		if (opened)
		{
			table.cur_col = 0;
			table.row_max_h = 0;
			table.row_start = stroka;
		}
		else
		{
			NewLine(left1, top1);
			if (table.cur_row == 0) table.max_cols = table.cur_col;
			table.cur_row++;
			table.max_cols = table.cur_col;
		}
	}

	/*
	if (isTag("center"))
	{
		if (opened) text_align = ALIGN_CENTER;
		if (!opened)
		{
			NewLine(left1, top1);
			text_align = ALIGN_LEFT;
		}
		return;
	}
	if (isTag("right"))
	{
		if (opened) text_align = ALIGN_RIGHT;
		if (!opened)
		{
			NewLine(left1, top1);
			text_align = ALIGN_LEFT;
		}
		return;
	}
	*/
	if (isTag("h1")) || (isTag("h2")) || (isTag("h3")) || (isTag("h4")) {
		NewLine(left1, top1);
		if (opened) && (stroka>1) NewLine(left1, top1 + 10);
		strcpy(#oldtag, #tag);
		if (opened)
		{
			if (!strcmp(#parametr, "align=")) && (!strcmp(#options,"center")) text_align = ALIGN_CENTER;
			if (!strcmp(#parametr, "align=")) && (!strcmp(#options,"right")) text_align = ALIGN_RIGHT;
			b_text = 1;
		}
		if (!opened)
		{
			text_align = ALIGN_LEFT;
			b_text = 0;
		}
		return;
	}
	else
		oldtag=NULL;
		
	if (isTag("b")) || (isTag("strong")) || (isTag("big")) {
		b_text = opened;
		return;
	}
	if(isTag("i")) || (isTag("em")) || (isTag("subtitle")) {
		i_text = opened;
		return;
	}	
	if (isTag("dt"))
	{
		li_text = opened;
		IF(opened == 0) return;
		NewLine(left1, top1);
		return;
	}
	if (isTag("condition"))
	{
		condition_text_active = opened;
		if (opened) && (!strcmp(#parametr, "show_if=")) condition_text_val = atoi(#options);
		return;
	}
	if (isTag("li")) || (isTag("dt")) //надо сделать вложенные списки
	{
		li_text = opened;
		if (opened)
		{
			NewLine(left1, top1);
			if (stroka > -1) && (stroka - 2 < list.visible) DrawBuf.DrawBar(li_tab * 5 * 6 + left1 - 5, list.line_h/2-3, 2, 2, 0x555555);
		}
		return;
	}
	if (isTag("u")) || (isTag("ins")) u_text = opened;
	if (isTag("s")) || (isTag("strike")) || (isTag("del")) s_text = opened;
	if (isTag("ul")) || (isTag("ol")) IF(!opened)
	{
		li_text = opened;
		li_tab--;
		NewLine(left1, top1);
	} ELSE li_tab++;
	if (isTag("dd")) stolbec += 5;
	if (isTag("blockquote")) blq_text = opened;
	if (isTag("pre")) || (isTag("code")) pre_text = opened; 
	if (isTag("hr"))
	{
		if (anchor) || (stroka < -1)
		{
			stroka+=2;
			return;
		}
		if (strcmp(#parametr, "color=") == 0) hr_color = GetColor(#options); else hr_color = 0x999999;
		NewLine(left1, top1);
		DrawBuf.DrawBar(5, list.line_h/2, list.w-10, 1, hr_color);
		NewLine(left1, top1+list.line_h);
	}
	if (isTag("img"))
	{
		ImgCache.Images( left1, top1, WB1.list.w);
		return;
	}
	if (isTag("meta")) || (isTag("?xml"))
	{
		do{
			if (!strcmp(#parametr, "charset=")) || (!strcmp(#parametr, "content=")) || (!strcmp(#parametr, "encoding="))
			{
				strcpy(#options, #options[strrchr(#options, '=')]); //поиск в content=
				strlwr(#options);
				meta_encoding = _DEFAULT;
				if (!strcmp(#options, "utf-8"))  || (!strcmp(#options,"utf8")) meta_encoding = _UTF;
				if (!strcmp(#options, "koi8-r")) || (!strcmp(#options, "koi8-u")) meta_encoding = _KOI;
				if (!strcmp(#options, "windows-1251")) || (!strcmp(#options, "windows1251")) meta_encoding = _WIN;
				if (!strcmp(#options, "dos"))    || (!strcmp(#options, "cp-866"))   meta_encoding = _DOS;
				if (cur_encoding==_DEFAULT) BufEncode(meta_encoding);
				return;
			}
		} while(GetNextParam());
		return;
	}
}

void TWebBrowser::DrawScroller()
{
	scroll_wv.max_area = list.count;
	scroll_wv.cur_area = list.visible;
	scroll_wv.position = list.first;

	scroll_wv.all_redraw=0;
	scroll_wv.start_x = list.x + list.w;
	scroll_wv.start_y = list.y;
	scroll_wv.size_y=list.h;

	scrollbar_v_draw(#scroll_wv);
}


void TWebBrowser::NewLine(int left1, top1)
{
	if (!stroka) DrawBar(list.x, list.y, list.w, 5, bg_color);
	if (t_html) && (!t_body) return;
	if (top1>=list.y) && ( top1 < list.h+list.y-10)  && (!anchor)
	{
		if (text_align == ALIGN_CENTER) DrawBuf.AlignCenter(left1,top1,list.w,list.line_h,stolbec * 6);
		if (text_align == ALIGN_RIGHT) DrawBuf.AlignRight(left1,top1,list.w,list.line_h,stolbec * 6);
		DrawBuf.bufy = top1;
		DrawBuf.Show();
		DrawBuf.Fill(bg_color);
	}
	stroka++;
	if (blq_text) stolbec = 6; else stolbec = 0;
	if (li_text) stolbec = li_tab * 5;
}



int isTag(dword text) 
{ 
	if (!strcmp(#tag,text)) return 1; else return 0;
}


