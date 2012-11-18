
int	downloader_id;

dword
	buf,
	filesize,
	blink;

char download_path[]="/rd/1/.download";
char search_path[]="http://nigma.ru/index.php?s=";
char version[]=" Text-based Browser 0.97.7";


struct TWebBrowser {
	int left, top, width, height;
	void Scan(int);
	void GetNewUrl();
	void OpenPage();
	void ReadHtml(byte);
	void ShowPage();
	void ParseHTML(dword);
	void WhatTextStyle(int left1, top1, width1);
	void DrawPage();
	void DrawScroller();
};

TWebBrowser WB1;

byte rez, b_text, i_text, u_text, s_text, pre_text, blq_text, li_text,
	link, ignor_text, li_tab, first_line_drawed, cur_encoding;


dword text_colors[300],
	text_color_index,
	link_color,
	bg_color;

int stroka,
	stolbec,
	tab_len;

char anchor[256];
int anchor_line_num;
	
char line[500],
	tag[100],
	tagparam[10000],
	parametr[1200],
	options[4096];

#include "include\history.h"
#include "include\colors.h"
#include "include\unicode_tags.h"
#include "include\some_code.h"
#include "include\parce_tag.h"


void TWebBrowser::Scan(int id)
{
	if (id >= 400)
	{
		GetURLfromPageLinks(id);
		
		//#1
		if (URL[0] == '#')
		{
			strcpy(#anchor, #URL+strrchr(#URL, '#'));
			
			strcpy(#URL, BrowserHistory.CurrentUrl());
			
			lines.first=lines.all-lines.visible;
			ShowPage();
			return;
		}
		//liner.ru#1
		if (strrchr(#URL, '#')<>-1)
		{
			strcpy(#anchor, #URL+strrchr(#URL, '#'));
			URL[strrchr(#URL, '#')-1] = 0x00; //заглушка
		}
		
		GetNewUrl();
		
		if (!strcmp(#URL + strlen(#URL) - 4, ".gif")) || (!strcmp(#URL + strlen(#URL) - 4, ".png")) || (!strcmp(#URL + strlen(#URL) - 4, ".jpg"))
		{
			//if (strstr(#URL,"http:")) 
			RunProgram("/sys/media/kiv", #URL);
			strcpy(#editURL, BrowserHistory.CurrentUrl());
			strcpy(#URL, BrowserHistory.CurrentUrl());
			return;
		}
		if (!strcmpn(#URL,"mailto:", 7))
		{
			RunProgram("@notify", #URL);
			strcpy(#editURL, BrowserHistory.CurrentUrl());
			strcpy(#URL, BrowserHistory.CurrentUrl());
			return;
		}

		OpenPage();
		return;
	}
	
	IF(lines.all < lines.visible) SWITCH(id) //если мало строк игнорируем некоторые кнопки
	{ CASE 183: CASE 184: CASE 180: CASE 181: return; } 
	
	switch (id)
	{
		case 011: //Ctrk+K 
			ReadHtml(_KOI);
			break;
		case 021: //Ctrl+U
			ReadHtml(_UTF);
			break;
		case 004: //Ctrl+D
			ReadHtml(_DOS);
			break;
		case 001:
			if (!pre_text) pre_text=2;
				else pre_text=0;
			break;
		case 005: //truetype
			if (use_truetype == 2) 
			{
				RunProgram("@notify", "Library does not exists /rd/1/lib/truetype.obj"w);
				return;
			}
			if (use_truetype == 1) use_truetype=0; else use_truetype=1;
			break;
		case BACK:
			if (!BrowserHistory.GoBack()) return;
			OpenPage();
			return;
		case FORWARD:
			if (!BrowserHistory.GoForward()) return;
			OpenPage();
			return;
		case 052:  //Нажата F3
			if (strcmp(get_URL_part(5),"http:")<>0) RunProgram("/rd/1/tinypad", #URL); else RunProgram("/rd/1/tinypad", #download_path);
			return;
		case 054: //F5
			IF(edit1.flags == 66) break;
		case REFRESH:
			if (GetProcessSlot(downloader_id)<>0)
			{
				KillProcess(downloader_id);
				Pause(20);
				Draw_Window();
				return;
			}
			anchor_line_num=lines.first; //весёлый костыль :Р
			anchor[0]='|';
			OpenPage();
			return;
		case 014: //Ctrl+N новое окно
		case 020: //Ctrl+T новая вкладка
		case NEWTAB:
			MoveSize(190,80,OLD,OLD);
			RunProgram(#program_path, #URL);
			return;
			
		case HOME:
			strcpy(#editURL, "http://kolibri-os.narod.ru");
		case GOTOURL:
		case 0x0D: //enter
			strcpy(#URL, #editURL);
			OpenPage();
			return;
		case 173:	//ctrl+enter
		case SEARCHWEB:
			strcpy(#URL, #search_path);
			strcat(#URL, #editURL);
			OpenPage();
			return;

		case ID1: //мотаем вверх
			IF(lines.first <= 0) return;
			lines.first--;
			break; 
		case ID2: //мотаем вниз
			IF(lines.visible + lines.first >= lines.all) return;
			lines.first++;
			break; 
		case 183: //PgDown
			IF(lines.first == lines.all - lines.visible) return;
			lines.first += lines.visible + 2;
			IF(lines.visible + lines.first > lines.all) lines.first = lines.all - lines.visible;
			break;
		case 184: //PgUp
			IF(lines.first == 0) return;
			lines.first -= lines.visible - 2;
			IF(lines.first < 0) lines.first = 0;
			break;
		case 180: //home
			IF(lines.first == 0) return;
			lines.first = 0;
			break; 
		case 181: //end
			IF (lines.first == lines.all - lines.visible) return;
			lines.first = lines.all - lines.visible;
			break; 
		default:
			return;
	}
	ParseHTML(buf);
}


char *ABSOLUTE_LINKS[]={ "http:", "mailto:", "ftp:", "/sys/", "/rd/", "/fd/", "/bd/", "/hd/", "/cd/", "/tmp/", 0};

//dword TWebBrowser::GetNewUrl(dword CUR_URL, NEW_URL){
void TWebBrowser::GetNewUrl(){
	int i, len;
	
	for (i=0; ABSOLUTE_LINKS[i]; i++)
	{
		len=strlen(ABSOLUTE_LINKS[i]);
		if (!strcmpn(#URL, ABSOLUTE_LINKS[i], len)) return;
	}
		
	IF (!strcmpn(#URL,"./", 2)) strcpy(#URL, #URL+2); //игнорим :)
	if (URL[0] == '/') strcpy(#URL, #URL+1);

	strcpy(#editURL, BrowserHistory.CurrentUrl()); //достаём адрес текущей страницы
		
	_CUT_ST_LEVEL_MARK:
		
		if (editURL[strrchr(#editURL, '/')-2]<>'/')  // если не http://
		{
			editURL[strrchr(#editURL, '/')] = 0x00; //обрезаем её урл до последнего /
		}
		
		IF (!strcmp(get_URL_part(3),"../")) //на уровень вверх
		{
			strcpy(#URL,#URL+3);
			editURL[strrchr(#editURL, '/')-1] = 0x00; //обрезаем её урл до последнего /
			goto _CUT_ST_LEVEL_MARK;
		}
		
		if (editURL[strlen(#editURL)-1]<>'/') strcat(#editURL, "/"); 
		
		strcat(#editURL, #URL); //клеим новый адрес
		strcpy(#URL, #editURL);
}


	
void TWebBrowser::ReadHtml(byte encoding)
{
	if (!strcmp(get_URL_part(5),"http:"))) 
		file_size stdcall (#download_path);
	else
		file_size stdcall (#URL);
	
	filesize = EBX;
	if (!filesize) return;
	
	mem_Free(buf);
	buf = mem_Alloc(filesize);
	if (!strcmp(get_URL_part(5),"http:"))) 
		ReadFile(0, filesize, buf, #download_path);
	else
		ReadFile(0, filesize, buf, #URL);
		
	cur_encoding = encoding;
	if (encoding==_WIN) wintodos(buf);
	if (encoding==_UTF) utf8rutodos(buf);
	if (encoding==_KOI) koitodos(buf);
}


void TWebBrowser::OpenPage()
{
	if (GetProcessSlot(downloader_id)<>0) PutPaletteImage(#toolbar,200,42,0,0,8,#toolbar_pal);
	KillProcess(downloader_id);
	strcpy(#editURL, #URL);
	BrowserHistory.AddUrl();
	strcpy(#header, #version);
	pre_text =0;
	if (!strcmp(get_URL_part(5),"http:")))
	{
		KillProcess(downloader_id); //убиваем старый процесс
		DeleteFile(#download_path);
		IF (URL[strlen(#URL)-1]=='/') URL[strlen(#URL)-1]='';
		downloader_id = RunProgram("/sys/network/downloader", #URL);
		//Browser Hack v2.0
		Pause(60);
		if (GetProcessSlot(downloader_id)<>0)
		{
			debug("Browser Hack v2.0: Killing downloader and trying to run it one more!");
			KillProcess(downloader_id); //убиваем старый процесс
			downloader_id = RunProgram("/sys/network/downloader", #URL);
		}
		//
		IF (downloader_id<0) RunProgram("@notify", "Error running Downloader. Internet unavilable.");
		Draw_Window();
		return;
	}
	lines.first = lines.all =0;
	ReadHtml(_WIN);
	WB1.ShowPage();
}


void TWebBrowser::ShowPage()
{
	edit1.size = edit1.pos = strlen(#editURL);
	edit1.offset=0;
	edit_box_draw stdcall(#edit1); //рисуем строку адреса
	
	if (!filesize)
	{
		DrawBar(left, top, width+4, height, 0xFFFFFF); //закрашиваем всё донизу
		if (GetProcessSlot(downloader_id)<>0) WriteText(left + 10, top + 18, 0x80, 0, "Loading...", 0);
		else
		{
			WriteText(left + 10, top + 18, 0x80, 0, "Page not found. May be, URL contains some errors.", 0);
			if (!strcmp(get_URL_part(5),"http:"))) WriteText(left + 10, top + 32, 0x80, 0, "Or Internet unavilable for your configuration.", 0);
		}
		//return;
	}
	else
		ParseHTML(buf);

	if (!header) strcpy(#header, #version);
	if (!strcmp(#version, #header)) DrawTitle(#header);
}



void TWebBrowser::ParseHTML(dword bword){
	word bukva[2];
	int j, perenos_num;
	byte ignor_param;
	char temp[768];
	
	stroka = -lines.first;
	stolbec = 0;
	
	for (j = 400; j < blink + 1; j++;) DeleteButton(j);
	blink = 400;

	b_text = i_text = u_text = s_text = blq_text = first_line_drawed =
	li_text = link = ignor_text = text_color_index = text_colors[0] = li_tab = 0; //обнуляем теги
	link_color = 0x0000FF;
	bg_color = 0xFFFFFF;
	line = '';
	strcpy(#page_links,"|");
	strcpy(#header, #version);

	if (pre_text<>2)
	{
		pre_text=0;
		if (!strcmp(#URL + strlen(#URL) - 4, ".txt")) pre_text = 1;
		if (!strcmp(#URL + strlen(#URL) - 4, ".mht")) ignor_text = 1;
	}
	
	for ( ; buf+filesize > bword; bword++;)
	{
		bukva = ESBYTE[bword];
		if (ignor_text) && (bukva<>'<') continue;
		switch (bukva)
		{
		case 0x0a:
			if (pre_text)
			{
				bukva = temp = '';
				goto NEXT_MARK;
			}
		case '\9':
			if (pre_text) //иначе идём на 0x0d	
			{
				tab_len=strlen(#line)/8;
				tab_len=tab_len*8;
				tab_len=8+tab_len-strlen(#line);
				for (j=0; j<tab_len; j++;) strcat(#line," ");
				break;
			}		
		case 0x0d:
			bukva = ' ';
			goto DEFAULT_MARK;
		case '=': //поддержка шайтанской кодировки страниц, сохранённых через ИЕ7
			if (strcmp(#URL + strlen(#URL) - 4, ".mht")<>0) goto DEFAULT_MARK;

			bword++;
			bukva=ESBYTE[bword];
			strcpy(#temp,#bukva);
			bword++;
			bukva=ESBYTE[bword];
			strcat(#temp,#bukva);
			
			bukva=Hex2Symb(#temp);
			if (bukva) goto DEFAULT_MARK;
			break;
			
		case '&': //&nbsp; and so on
			bword++;
			tag=0;
			for (j=0; (ESBYTE[bword]<>';') && (j<7);   j++, bword++;)
			{
				bukva = ESBYTE[bword];
				strcat(#tag, #bukva);
			}
			
			for (j=0; unicode_tags[j]!=0; j+=2;) 
			{
				if (!strcmp(#tag, unicode_tags[j]))
				{
					strcat(#line, unicode_tags[j+1]);
					break 1;
				}
			}
			
			rez = atoi(#tag + 1) - 1040;
			if (tag[1] == '1') && (rez>=0) && (rez<=72) && (strlen(#tag) == 5)
			{
				bukva = unicode_chars[rez];
				//GOTO DEFAULT_MARK; //обрабатываем букву лучше наверно strcat(#line, unicode_tags[j+1]); и break 1; 
				strcat(#line, #bukva);
				break; 
			}
			
			strcat(#line,#tag); //выводим на экран необработанный тег, так браузеры зачем-то делают
			break;
		case '<':
			bword++; //промотаем символ <
			tag = parametr = tagparam = ignor_param = NULL;
			if (ESBYTE[bword] == '!') //фильтрация внутри <!-- -->, дерзко
			{
				bword++;
				if (ESBYTE[bword] == '-')
				{
				HH_:
					do
					{
						bword++;
						if (buf + filesize <= bword) break 2;
					}
					while (ESBYTE[bword] <>'-');
					
					bword++;
					if (ESBYTE[bword] <>'-') goto HH_;
				}
			}
			while (ESBYTE[bword] <>'>') && (bword < buf + filesize) //получаем тег и его параметры
			{
				bukva = ESBYTE[bword];
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
				bword++;
			}
			strlwr(#tag);
			strlwr(#tagparam);

			if (tag[strlen(#tag)-1]=='/') tag[strlen(#tag)-1]=NULL; //for br/
			if (tagparam) && (strlen(#tagparam) < 4000) GetNextParam();

			DrawPage();
			line=NULL;

			if (tag) WhatTextStyle(left + 5, stroka * 10 + top + 5, width - 20); //обработка тегов

			tag = parametr = tagparam = ignor_param = NULL;
			break;
		default:
			DEFAULT_MARK:
			if (!pre_text) && (bukva == ' ')
			{
				if (line[strlen(#line)-1]==' ') break; //убрать 2 пробела подряд
				if (!stolbec) && (!line) break; //строка не может начинаться с пробела
			}
			if (strlen(#line)<sizeof(line))	strcat(#line, #bukva);

			if (stolbec + strlen(#line) > lines.column_max)
			{
			NEXT_MARK:
				perenos_num = strrchr(#line, ' ');
				if (!perenos_num) && (strlen(#line)>lines.column_max) perenos_num=lines.column_max;
				strcpy(#temp, #line + perenos_num); //перенос по словам
				line[perenos_num] = 0x00;
				if (stroka >= lines.visible) && (lines.first <>0) break 1; //уходим...
				DrawPage();
				TextGoDown(left + 5, stroka * 10 + top + 5, width - 20); //закрашиваем следущую строку
				strcpy(#line, #temp);
			}
		}
	}

	DrawPage(); //рисует последнюю строку, потом это надо убрать, оптимизировав код
	if (lines.visible * 10 + 25 <= height)
		DrawBar(left, lines.visible * 10 + top + 25, width - 15, -lines.visible * 10 + height - 25, bg_color);
	if (stroka * 10 + 15 <= height)
		DrawBar(left, stroka * 10 + top + 15, width - 15, -stroka * 10 + height - 15, bg_color); //закрашиваем всё до конца
	if (lines.first == 0) lines.all = stroka;
	if (anchor)
	{
		//если посреди текста появится новый якорь - будет бесконечный цикл
		anchor='';
		lines.first=anchor_line_num;
		ParseHTML(buf);
	}

	DrawScroller();
}

void TWebBrowser::DrawPage() //резать здесь!!1!
{
	int start_x, start_y, line_length;
	char temp[sizeof(line)];
	
	if (!header) //&& (tag) 
	{
		if (strlen(#version)+strlen(#line)+2>sizeof(header))
		{
			strcpy(#temp, #line);
			temp[sizeof(header)-strlen(#version)-2]=0;
			strcpy(#header, #temp);
			strcpy(#line, #line+strlen(#temp));
		}
		else
		{
			strcpy(#header, #line);
			line=0;
		}
			
		strcat(#header, " -");
		strcat(#header, #version);
		return;
	}
	
	if (stroka >= 0) && (stroka - 2 < lines.visible) && (line) && (!anchor)
	{
		if (!stroka) && (!stolbec)
		{
			DrawBar(left, top, width-15, 15, bg_color); //закрашиваем первую строку
			first_line_drawed=1;
		}
		
		start_x=stolbec * 6 + left+5;
		start_y=stroka * 10 + top + 5;
		line_length=strlen(#line)*6;

		if (use_truetype == 1)
		{
			//line_length =  get_length stdcall (#line,-1,16,line_length);
			text_out stdcall (#line, #fontlol, 17, text_colors[text_color_index], start_x, start_y-3);
		}
		else
		{
			WriteText(start_x, start_y, 0x80, text_colors[text_color_index], #line, 0);
			IF (b_text)	{ $add ebx, 1<<16   $int 0x40 }
		}
		IF (i_text) Skew(start_x, start_y, line_length+6, 10);
		IF (s_text) DrawBar(start_x, start_y + 4, line_length, 1, text_colors[text_color_index]);
		IF (u_text) DrawBar(start_x, start_y + 8, line_length, 1, text_colors[text_color_index]);
		IF (link) {
			DefineButton(start_x-2, start_y, line_length + 3, 9, blink + BT_HIDE, 0xB5BFC9);
			DrawBar(start_x, start_y + 8, line_length, 1, text_colors[text_color_index]);
		}
		stolbec += strlen(#line);
	}
}


char oldtag[100];
void TWebBrowser::WhatTextStyle(int left1, top1, width1) {
	dword hr_color;

    dword image;
    char temp[4096], alt[4096];
    int w=0, h=0, img_lines_first=0;

	//проверяем тег открывается или закрывается
	if (tag[0] == '/') 
	{
		rez = 0;
		strcpy(#tag, #tag+1);
	}
	else
		rez = 1;
		
	if (!chTag("html")) {
		IF(!strcmp(#URL + strlen(#URL) - 4, ".mht")) IF (rez==0) ignor_text = 1; ELSE ignor_text = 0;
		return;
	}

	if (!chTag("script")) || (!chTag("style")) || (!chTag("binary")) ignor_text = rez;

	if(!chTag("title"))
	{
		if (rez)
		{
			header=0;
		}
		else //тег закрылся - вывели строку
		{
			if (stroka==0) DrawTitle(#header);
		}
		return;
	}

	if (ignor_text) return;



	
	IF(!chTag("q")) strcat(#line, "\"");


	if (anchor) && (!strcmp(#parametr, "id=")) //очень плохо!!! потому что если не последний тег, работать не будет
	{
		if (!strcmp(#anchor, #options))
		{
			anchor_line_num=lines.first+stroka;
		}
	}
	
	if (!chTag("body"))
	{
		BODY_MARK:
		
		if (!strcmp(#parametr, "link="))
			link_color = GetColor(#options);
		
		if (!strcmp(#parametr, "text="))
			text_colors[0]=GetColor(#options);
		
		if (!strcmp(#parametr, "bgcolor="))
			bg_color=GetColor(#options);
		
		IF(tagparam)
		{
			GetNextParam();
			GOTO BODY_MARK;
		}
		
		return;
	}

	if (!chTag("a"))
	{
		if (rez)
		{
			 
			if (link) IF(text_color_index > 0) text_color_index--; //если предыдущий тег а не был закрыт

			_A_MARK:
			if (!strcmp(#parametr, "href="))
			{
				if (stroka - 1 > lines.visible) || (stroka < -2) return;
				
				text_color_index++;
				text_colors[text_color_index] = text_colors[text_color_index-1];
				
				link = 1;
				blink++;
				text_colors[text_color_index] = link_color;
				strcat(#page_links, #options);
				strcat(#page_links, "|");
			}
			if (anchor) && (!strcmp(#parametr, "name="))
			{
				if (!strcmp(#anchor, #options))
				{
					anchor_line_num=lines.first+stroka;
				}
			}
			if (tagparam)
			{
				GetNextParam();
				GOTO _A_MARK;
			}
		}
		else {
			link = 0;
			IF(text_color_index > 0) text_color_index--;
		}
		return;
	}


	if (!chTag("font"))
	{
		if (rez)
		{
			text_color_index++;
			text_colors[text_color_index] = text_colors[text_color_index-1];
		
			COL_MARK:
			if (strcmp(#parametr, "color=") == 0) //&& (parametr[1] == '#')
			{
				text_colors[text_color_index] = GetColor(#options);
			}
			IF(tagparam) {
				GetNextParam();
				GOTO COL_MARK;
			}
		}
		else
			if (text_color_index > 0) text_color_index--;
		return;
	}

	if(!chTag("tr")) || (!chTag("br")) {
		TextGoDown(left1, top1, width1);
		return;
	}
	if (!chTag("div")) {
		IF(oldtag[0] <>'h') TextGoDown(left1, top1, width1);
		return;
	}
	if (!chTag("p")) {
		IF(oldtag[0] == 'h') return;
		TextGoDown(left1, top1, width1);
		IF(rez) TextGoDown(left1, top1 + 10, width1);
		return;
	}

	if (!chTag("h1")) || (!chTag("h2")) || (!chTag("h3")) || (!chTag("h4")) {
		TextGoDown(left1, top1, width1);
		IF(rez) TextGoDown(left1, top1 + 10, width1);
		b_text = rez;
		strcpy(#oldtag, #tag);
		return;
	}
	else
		oldtag='';
		
	if (!chTag("b")) || (!chTag("strong")) || (!chTag("big")) {
		b_text = rez;
		return;
	}
	////////////////////////////
	if(!chTag("i")) || (!chTag("em")) || (!chTag("subtitle")) {
		i_text = rez;
		return;
	}	
	////////////////////////////
	if (!chTag("dt"))
	{
		li_text = rez;
		IF(rez == 0) return;
		TextGoDown(left1, top1, width1);
		return;
	}
	/////////////////////////////
	if(!chTag("li")) || (!chTag("dt")) //надо сделать вложенные списки
	{
		li_text = rez;
		IF(rez == 0) return;
		TextGoDown(left1, top1, width1);
		IF(stroka > -1) && (stroka - 2 < lines.visible) DrawBar(li_tab * 5 * 6 + left1 - 5, top1 + 12, 2, 2, 0);
		return;
	}
	////////////////////////////
	IF(!chTag("u")) || (!chTag("ins")) u_text = rez;
	IF(!chTag("s")) || (!chTag("strike")) || (!chTag("del")) s_text = rez;
	IF(!chTag("ul")) || (!chTag("ol")) IF(!rez) {
		li_text = rez;
		li_tab--;
		TextGoDown(left1, top1, width1);
	} ELSE li_tab++;
	IF(!chTag("dd")) stolbec += 5;
	IF(!chTag("blockquote")) blq_text = rez;
	IF(!chTag("pre")) pre_text = rez; 
	IF(!chTag("hr")) {
		TextGoDown(left1, top1, width1);
		TextGoDown(left1, top1 + 10, width1);
		IF(strcmp(#parametr, "color=") == 0) hr_color = GetColor(#options);
		ELSE hr_color = 0x999999;
		IF(stroka > 0) DrawBar(left1, top1 + 14, width1 - 8, 1, hr_color);
	}

	if (!chTag("img"))
	{
		if (GetFileInfo(libimg)<>0) return;  //если библиотеки нет
		IMG_TAG:
			if (!strcmp(#parametr,"src="))   //надо объединить с GetNewUrl()
			{
				if (downloader_id) strcpy(#temp, #history_list[history_current-1].Item);
					else strcpy(#temp, BrowserHistory.CurrentUrl()); //достаём адрес текущей страницы
				if (strcmpn(#temp, "http:", 5)!=0) || (strcmpn(#options, "http:", 5)!=0)
				{
					temp[strrchr(#temp, '/')] = 0x00; //обрезаем её урл до последнего /
					strcat(#temp, #options);
					image=load_image(#temp);
					w=DSWORD[image+4];
					h=DSWORD[image+8];
				}
			}
  			if (!strcmp(#parametr,"alt="))
			{
				strcpy(#alt, "[");
				strcat(#alt, #options);
				strcat(#alt, "]");
			}

		IF(tagparam)
		{
			GetNextParam();
			GOTO IMG_TAG;
		}
		
		if (!image) 
		{
			if (alt) && (link) strcat(#line, #alt);
			return;
		}
		
		if (w>width1) w=width1;
		
		if (stroka==0) DrawBar(left, top, width-15, 15, bg_color); //закрашиваем первую строку
		stroka+=h/10;
		if (top1+h<WB1.top) || (top1>WB1.top+WB1.height-10) return; //если ВСЁ изображение ушло ВЕРХ или ВНИЗ
		if (top1<WB1.top) //если часть изображения сверху
		{
			DrawBar(left, top, width-15, 10, bg_color); //закрашиваем первую строку
			img_lines_first=WB1.top-top1;
			h=h-img_lines_first;
			top1=WB1.top;
		}
		if (top1>WB1.top+WB1.height-h-15) //если часть изображения снизу
		{
			h=WB1.top+WB1.height-top1-15;
		}	
		if (h<=0) return;
		if (anchor) return;
		
		img_draw stdcall (image,left1-5,top1+10,w, h,0,img_lines_first);
		DrawBar(left1+w - 5, top1 + 10, width1-w + 5, h, bg_color);
		IF (link) DefineButton(left1 - 5, top1+10, w, h, blink + BT_HIDE, 0xB5BFC9);
		return;
	}

	if (!chTag("meta")) || (!chTag("?xml"))
	{
		META:
		if (!strcmp(#parametr, "charset=")) || (!strcmp(#parametr, "content=")) || (!strcmp(#parametr, "encoding="))
		{
			strcpy(#options, #options[strrchr(#options, '=')]); //поиск в content=

			if (!strcmp(#options,"utf-8"))   || (!strcmp(#options,"utf8"))      ReadHtml(_UTF);
			if (!strcmp(#options, "koi8-r")) || (!strcmp(#options, "koi8-u"))   ReadHtml(_KOI);
			if (!strcmp(#options, "dos"))    || (!strcmp(#options, "cp-866"))   ReadHtml(_DOS);
		}
		if (tagparam)
		{
			GetNextParam();
			goto META;
		}
		return;
	}
}


void TextGoDown(int left1, top1, width1)
{
	if (!stroka) && (!stolbec) && (!first_line_drawed)
	{
		DrawBar(WB1.left, WB1.top, WB1.width-15, 15, bg_color); //закрашиваем первую строку
		first_line_drawed=1;
	}
	stroka++;
	if (blq_text) stolbec = 8;
	ELSE stolbec = 0;
	if (li_text) stolbec = li_tab * 5;
	IF(stroka >= 0) && (stroka - 2 < lines.visible)  && (!anchor) DrawBar(left1 - 5, top1 + 10, width1 + 5, 10, bg_color);
}


//скролл
void TWebBrowser::DrawScroller() //не оптимальная отрисовка, но зато в одном месте
{
	scroll1.max_area = lines.all;
	scroll1.cur_area = lines.visible;
	scroll1.position = lines.first;

	scroll1.all_redraw=1;
	scroll1.start_x=Form.width-28; //left + width - 15
	scroll1.size_y=WB1.height;

	scrollbar_v_draw(#scroll1);
}
