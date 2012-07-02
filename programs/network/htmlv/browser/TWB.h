
int	downloader_id;

dword
	buf,
	filesize,
	blink;
int i;

char download_path[]="/rd/1/.download";
char search_path[]="http://nigma.ru/index.php?s=";
char version[]=" Text-based Browser 0.94z";


struct TWebBrowser {
	int left, top, width, height;
	void Scan(int);
	void GetNewUrl();
	void OpenPage();
	void ReadHtml(byte);
	void ShowPage();
	void ParseHTML(dword);
	void WhatTextStyle(int left1, top1, width1);
	void DrawScroller();
};

TWebBrowser WB1;

byte rez, b_text, i_text, u_text, s_text, pre_text, blq_text, li_text,
	link, ignor_text, li_tab, first_line_drawed;


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
	options[1000];


#include "include\history.h"
#include "include\colors.h"
#include "include\unicode_tags.h"
#include "include\some_code.h"
#include "include\parce_tag.h"


void TWebBrowser::Scan(int id)
{
	if (id > 399)
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
			RunProgram("/sys/media/kiv", #URL);
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
		case 053:  //Нажата F4
			if (strcmp(get_URL_part(5),"http:")<>0) RunProgram("/rd/1/develop/t_edit", #URL); else RunProgram("/rd/1/develop/t_edit", #download_path);
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
			BREAK;
		case 184: //PgUp
			IF(lines.first == 0) RETURN;
			lines.first -= lines.visible - 2;
			IF(lines.first < 0) lines.first = 0;
			BREAK;
		case 180: //home
			IF(lines.first == 0) RETURN;
			lines.first = 0;
			BREAK; 
		case 181: //end
			IF (lines.first == lines.all - lines.visible) RETURN;
			lines.first = lines.all - lines.visible;
			BREAK; 
		default:
			RETURN;
	}
	ParseHTML(buf);
}



void TWebBrowser::GetNewUrl(){
	IF (!strcmp(get_URL_part(2),"./")) strcpy(#URL, #URL+2); //игнорим :)
	
	if (URL[0] <> '/')
	&& (strcmp(get_URL_part(5),"http:")<>0)	&& (strcmp(get_URL_part(5),"mailt")<>0)	&& (strcmp(get_URL_part(5),"ftp:/")<>0) 
	{
		strcpy(#editURL, BrowserHistory.CurrentUrl()); //достаём адрес текущей страницы
		
		_CUT_ST_LEVEL_MARK:
		
		if (editURL[strrchr(#editURL, '/')-2]<>'/')  // если не http://pagename.ua <-- нахрена эта строка???
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
	if (!strcmp(get_URL_part(5),"http:")))
	{
		KillProcess(downloader_id); //убиваем старый процесс
		DeleteFile(#download_path);
		IF (URL[strlen(#URL)-1]=='/') URL[strlen(#URL)-1]='';
		downloader_id = RunProgram("/sys/network/downloader", #URL);
		//это гениально и это пиздец!!!
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
	lines.first = lines.all = 0;
	ReadHtml(_WIN);
	WB1.ShowPage();
}


void TWebBrowser::ShowPage()
{
	edit1.size = edit1.pos = strlen(#editURL);
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
		DrawTitle(#version); //?
		return;
	}
	
	ParseHTML(buf);
	IF (!strcmp(#version, #header)) DrawTitle(#header);
}



void TWebBrowser::ParseHTML(dword bword){
	word bukva[1];
	int j, perenos_num;
	byte ignor_param = 0;
	char temp[768];
	
	stroka = -lines.first;
	stolbec = 0;
	
	for (j = 400; j < blink + 1; j++;) DeleteButton(j);
	blink = 400;

	b_text = i_text = u_text = s_text = pre_text = blq_text = first_line_drawed =
	li_text = link = ignor_text = text_color_index = text_colors[0] = li_tab = 0; //обнуляем теги
	link_color = 0x0000FF;
	bg_color = 0xFFFFFF;
	line = '';
	strcpy(#page_links,"|");
	strcpy(#header,#version);

	if (!strcmp(#URL + strlen(#URL) - 4, ".txt")) pre_text = 1;
	if (!strcmp(#URL + strlen(#URL) - 4, ".mht")) ignor_text = 1;
	
	debug("Start parsing");
	
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
				for (i=0; i<tab_len; i++;) strcat(#line," ");
				break;
			}		
		case 0x0d:
			bukva = ' ';
			goto DEFAULT_MARK;
		case '<':
			bword++; //промотаем символ <
			if (ESBYTE[bword] == '!') //фильтрация внутри <!-- -->, дерзко
			{
				bword++;
				if (ESBYTE[bword] == '-') {
					HH_:
					do
					{
						bword++;
						if (bword >= buf + filesize) break 1;
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
			lowcase(#tag);
			lowcase(#tagparam);

			if (tag[strlen(#tag)-1]=='/') tag[strlen(#tag)-1]=''; //небольшой фикс для работы с XHTML-тегами типа br/
			if (strlen(#tagparam) > 0) && (strlen(#tagparam) < 4000) GetNextParam();
			//while (tagparam)
			//{
			//	GetNextParam();
				WhatTextStyle(left + 5, stroka * 10 + top + 5, width - 20); //обработка тегов
			//}

			line = tag = parametr = tagparam = ignor_param = 0; //всё обнуляем
			
			break;
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
			
		case '&': //обработка тегов типа &nbsp;
			bword++;
			tag='';
			for (j=0;  (ESBYTE[bword] <>';') && (j < 7);     j++, bword++;)
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
			
			rez = StrToInt(#tag + 1) - 1040;
			if (tag[1] == '1') && (rez>=0) && (rez<=72) && (strlen(#tag) == 5)
				{
					bukva = unicode_chars[rez];
					GOTO DEFAULT_MARK; //обрабатываем букву
				}
			
			//debug(#tag); //тэг не найден - выводим на доску отладки
			strcat(#line,#tag); //выводим на экран необработанный тег, так браузеры зачем-то делают
			break;
		default:
			DEFAULT_MARK:
			if (!pre_text) && (bukva == ' ') && (line[strlen(#line)-1]==' ') break;
			//
			if (stolbec + strlen(#line) > lines.column_max)
			{
				perenos_num = strrchr(#line, ' ');
				strcpy(#temp, #line + perenos_num); //перенос по словам
				line[perenos_num] = 0x00;
			NEXT_MARK:
				if (stroka >= lines.visible) && (lines.first <>0) break 1; //уходим...
				WhatTextStyle(left + 5, stroka * 10 + top + 5, width - 20); //вывод строки
				TextGoDown(left + 5, stroka * 10 + top + 5, width - 20); //закрашиваем следущую строку
				strcpy(#line, #temp);
			}
			if (!pre_text) && (bukva == ' ') && (!stolbec) && (!line) break;
			strcat(#line, #bukva);
	  }
	}

	if (lines.visible * 10 + 25 <= height)
		DrawBar(left, lines.visible * 10 + top + 25, width - 15, -lines.visible * 10 + height - 25, bg_color);
	if (stroka * 10 + 15 <= height)
		DrawBar(left, stroka * 10 + top + 15, width - 15, -stroka * 10 + height - 15, bg_color); //закрашиваем всё до конца
	if (lines.first == 0) lines.all = stroka;
	
	if (anchor)
	{
		anchor='';
		lines.first=anchor_line_num;
		ParseHTML(buf);
	}

	debug("End parsing");
	DrawScroller(); //рисуем скролл
}


char oldtag[100];
void TWebBrowser::WhatTextStyle(int left1, top1, width1) {
	dword hr_color;

    dword image;
    char temp[4096];
    int w, h, img_lines_first, line_length;

	//проверяем тег открывается или закрывается
	IF(tag[0] == '/') 
	{
		rez = 0;
		strcpy(#tag, #tag+1);
	}
	ELSE
		rez = 1;

	//
	IF(!chTag("html")) {
		IF(!strcmp(#URL + strlen(#URL) - 4, ".mht")) IF (rez==0) ignor_text = 1; ELSE ignor_text = 0;
		return;
	}
	IF(!chTag("script")) || (!chTag("style")) || (!chTag("binary")) ignor_text = rez;

	if(!chTag("title")) && (!rez)
	{
		strcpy(#header, #line);
		strcat(#header, " -");
		strcat(#header, #version);
		if (stroka==0) DrawTitle(#header);
		return;
	}
	
	IF (ignor_text) return;

	IF(!chTag("q")) strcat(#line, "\"");
	
	//вывод на экран
	if (stroka >= 0) && (stroka - 2 < lines.visible) && (line) && (!anchor)
	{
		if (stroka==0) && (stolbec==0)
		{
			DrawBar(left, top, width-15, 15, bg_color); //закрашиваем первую строку
			first_line_drawed=1;
		}
		line_length=strlen(#line)*6;
		WriteText(stolbec * 6 + left1, top1, 0x80, text_colors[text_color_index], #line, 0); //может тут рисовать белую строку?
		//line_length =  get_length stdcall (#line,-1,16,line_length);
		//text_out stdcall (#line, -1, 17, text_colors[text_color_index], stolbec * 6 + left1, top1-2);
		IF (b_text)	{ $add ebx, 1<<16   $int 0x40 }
		IF (i_text) Skew(stolbec * 6 + left1, top1, line_length+6, 10); //наклонный текст
		IF (s_text) DrawBar(stolbec * 6 + left1, top1 + 4, line_length, 1, text_colors[text_color_index]); //зачёркнутый
		IF (u_text) DrawBar(stolbec * 6 + left1, top1 + 8, line_length, 1, text_colors[text_color_index]); //подчёркнутый
		IF (link) {
			DefineButton(stolbec * 6 + left1 - 2, top1, line_length + 3, 9, blink + BT_HIDE, 0xB5BFC9); //
			DrawBar(stolbec * 6 + left1, top1 + 8, line_length, 1, text_colors[text_color_index]);
		}
	}

	IF(!tag) return;
	stolbec += strlen(#line);

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
			text_color_index++;
			text_colors[text_color_index] = text_colors[text_color_index-1];

			_A_MARK:
			if (!strcmp(#parametr, "href="))
			{
				if (stroka - 1 > lines.visible) || (stroka < -2) return;
				if (link) && (text_color_index > 0) text_color_index--; //если не закрыт тэг
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
		ELSE {
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
		//if (GetFileInfo(#libimg)<>0) return;  //если библиотеки нет
		IMG_TAG:
			if (!strcmp(#parametr,"src="))   //надо объединить с GetNewUrl()
			{
				strcpy(#temp, BrowserHistory.CurrentUrl()); //достаём адрес текущей страницы
				temp[strrchr(#temp, '/')] = 0x00; //обрезаем её урл до последнего /
				strcat(#temp, #options);
				image=load_image(#temp);
				w=DSWORD[image+4];
				h=DSWORD[image+8];
			}
  			/*if (!strcmp(#parametr,"alt="))
			{
				strcpy(#tag, "[Image: ");
				strcat(#tag, #options);
				strcat(#tag, "]");
			}*/

		IF(tagparam)
		{
			GetNextParam();
			GOTO IMG_TAG;
		}
		
		if (!image)
		{
			//debug(#tag);
			return;
		}

		if (w>width1) w=width1;
	
		if (stroka==0) DrawBar(left, top, width-15, 15, bg_color); //закрашиваем первую строку
		
		stroka+=h/10;
		
		if (top1+h<WB1.top) || (top1>WB1.top+WB1.height-10) //если ВСЁ изображение ушло ВЕРХ или ВНИЗ
			return;

		if (top1<WB1.top) //если часть изображения сверху
		{
			DrawBar(left, top, width-15, 10, bg_color); //закрашиваем первую строку
			img_lines_first=WB1.top-top1;
			h=h-img_lines_first;
			top1=WB1.top;
		}
		
		if (top1>WB1.top+WB1.height-h-15) //если часть изображения снизу     IF (stroka - 2 < lines.visible)
		{
			h=WB1.top+WB1.height-top1-15;
		}	

		IF (h<=0) return;
		
		img_draw stdcall (image,left1-5,top1+10,w, h,0,img_lines_first);
		DrawBar(left1+w - 5, top1 + 10, width1-w + 5, h, bg_color);
		IF (link)
		{
			DefineButton(left1 - 5, top1+10, w, h, blink + BT_HIDE, 0xB5BFC9);
		}

		return;
	}

	if (!chTag("meta")) || (!chTag("?xml"))
	{
		META:
		if (!strcmp(#parametr, "charset=")) || (!strcmp(#parametr, "content=")) || (!strcmp(#parametr, "encoding="))
		{
			strcpy(#options, #options[strrchr(#options, '=')]); //поиск в content=

			if (!strcmp(#options,"utf-8")) || (!strcmp(#options,"utf8"))		ReadHtml(_UTF);
			if (!strcmp(#options, "koi8-r")) || (!strcmp(#options, "koi8-u"))	ReadHtml(_KOI);
			if (!strcmp(#options, "dos")) || (!strcmp(#options, "cp-866"))		ReadHtml(_DOS);
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