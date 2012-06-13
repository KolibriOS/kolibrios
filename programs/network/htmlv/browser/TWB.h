
int	downloader_id;

dword j,
	buf,
	filesize,
	blink = 400;
 int i;

char download_path[]="/rd/1/.download";
char search_path[]="http://nigma.ru/index.php?s=";
char version[]=" Text-based Browser 0.92";


struct TWebBrowser {
	int left, top, width, height;
	void DrawScroller();
	void ShowPage();
	void ParseHTML(dword, dword);
	void Scan(dword);
	void WhatTextStyle(int left1, top1, width1);
};

TWebBrowser WB1;

byte rez, b_text, i_text, u_text, s_text, pre_text, blq_text, li_text, link, ignor_text, li_tab, body_present;


dword text_colors[10],
	text_color_index,
	link_color,
	bg_color;

int stroka,
	stolbec,
	tab_len;
	
char line[330],
	tag[100],
	tagparam[10000],
	parametr[1200],
	options[1000];


#include "include\history.h"
#include "include\colors.h"
#include "include\unicode_tags.h"
#include "include\some_code.h"


void TWebBrowser::Scan(dword id) {
	if (id > 399)
	{
		GetURLfromPageLinks(id);
		//эту всю хрень нужно в GetNewUrl() переместить
		if (URL[0] == '#') {  //мы не умеем переходить по ссылке внутри документа. Пока что...
			copystr(BrowserHistory.CurrentUrl(), #editURL);
			copystr(#URL, #editURL + strlen(#editURL));
			copystr(BrowserHistory.CurrentUrl(), #URL);
			ShowPage(#URL);
			return;
		}
		URL[find_symbol(#URL, '#')-1] = 0x00; //заглушка, но это не совсем правильно - в едитурл должно оставаться

		GetNewUrl();
		
		if (!strcmp(#URL + strlen(#URL) - 4, ".gif")) || (!strcmp(#URL + strlen(#URL) - 4, ".png")) || (!strcmp(#URL + strlen(#URL) - 4, ".jpg"))
		{
			RunProgram("/sys/media/kiv", #URL);
			copystr(BrowserHistory.CurrentUrl(), #URL);
			return;
		}

		OpenPage();
		return;
	}
	
	//edit1.flags=64;
	IF(count < max_kolvo_strok) SWITCH(id) //если мало строк игнорируем некоторые кнопки
	{ CASE 183: CASE 184: CASE 180: CASE 181: return; } 
	
	switch (id)
	{
		case 011: //Ctrk+K 
			ReadHtml();
			koitodos(buf);
			break;
		case 021: //Ctrl+U
			ReadHtml();
			utf8rutodos(buf);
			break;
		case BACK:
			if (!BrowserHistory.GoBack()) return;
			OpenPage();
			return;
		case FORWARD:
			if (!BrowserHistory.GoForward()) return;
			OpenPage();
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
			OpenPage(); //от сердца отрываю, здесь нужно za_kadrom старое
			return;
		case 014: //Ctrl+N новое окно
		case 020: //Ctrl+T новая вкладка
		case NEWTAB:
			MoveSize(190,80,OLD,OLD);
			RunProgram(#program_path, #URL);
			return;
		case 052:  //Нажата F3
			IF(edit1.flags <> 66) 
			IF (strcmp(get_URL_part(5),"http:")<>0) RunProgram("tinypad", #URL); ELSE RunProgram("tinypad", #download_path);
			return;

		case HOME:
			copystr("http://kolibri-os.narod.ru", #editURL);
		case GOTOURL:
		case 0x0D: //enter
			copystr(#editURL, #URL);
			OpenPage();
			return;
		case 173:	//ctrl+enter
		case SEARCHWEB:
			copystr(#search_path, #URL);
			copystr(#editURL, #URL + strlen(#URL));
			OpenPage();
			return;

		case ID1: //мотаем вверх
			IF(za_kadrom <= 0) return;
			za_kadrom--;
			break; 
		case ID2: //мотаем вниз
			IF(max_kolvo_strok + za_kadrom >= count) return;
			za_kadrom++;
			break; 
		case 183: //PgDown
			IF(za_kadrom == count - max_kolvo_strok) return;
			za_kadrom += max_kolvo_strok + 2;
			IF(max_kolvo_strok + za_kadrom > count) za_kadrom = count - max_kolvo_strok;
			BREAK;
		case 184: //PgUp
			IF(za_kadrom == 0) RETURN;
			za_kadrom -= max_kolvo_strok - 2;
			IF(za_kadrom < 0) za_kadrom = 0;
			BREAK;
		case 180: //home
			IF(za_kadrom == 0) RETURN;
			za_kadrom = 0;
			BREAK; 
		case 181: //end
			IF (za_kadrom == count - max_kolvo_strok) RETURN;
			za_kadrom = count - max_kolvo_strok;
			BREAK; 
		default:
			RETURN;
	}
	ParseHTML(buf, filesize);
}



void GetNewUrl(){
	IF (!strcmp(get_URL_part(2),"./")) copystr(#URL+2,#URL); //игнорим :)
	
	if (URL[0] <> '/')
	&& (strcmp(get_URL_part(5),"http:")<>0)	&& (strcmp(get_URL_part(5),"mailt")<>0)	&& (strcmp(get_URL_part(5),"ftp:/")<>0) 
	{
		copystr(BrowserHistory.CurrentUrl(), #editURL); //достаём адрес текущей страницы
		
		_CUT_ST_LEVEL_MARK:
		
		if (editURL[find_symbol(#editURL, '/')-2]<>'/')  // если не http://pagename.ua <-- нахрена эта строка???
		{
			editURL[find_symbol(#editURL, '/')] = 0x00; //обрезаем её урл до последнего /
		}
		
		IF (!strcmp(get_URL_part(3),"../")) //на уровень вверх
		{
			copystr(#URL+3,#URL);
			editURL[find_symbol(#editURL, '/')-1] = 0x00; //обрезаем её урл до последнего /
			goto _CUT_ST_LEVEL_MARK;
		}
		
		if (editURL[strlen(#editURL)-1]<>'/') copystr("/", #editURL + strlen(#editURL)); 
		copystr(#URL, #editURL + strlen(#editURL)); //клеим новый адрес
		copystr(#editURL, #URL);
	}
}


	
void ReadHtml()
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
}


void OpenPage()
{
	if (GetProcessSlot(downloader_id)<>0) PutPaletteImage(#toolbar,200,42,0,0,8,#toolbar_pal);
	KillProcess(downloader_id);
	copystr(#URL, #editURL);
	BrowserHistory.AddUrl();
	za_kadrom = count = 0;
	if (!strcmp(get_URL_part(5),"http:")))
	{
		copystr(#version, #header);
		KillProcess(downloader_id); //убиваем старый процесс
		DeleteFile(#download_path);
		IF (URL[strlen(#URL)-1]=='/') URL[strlen(#URL)-1]='';
		downloader_id = RunProgram("/sys/network/downloader", #URL);
		//это гениально и это пиздец!!!
		Pause(60);
		if (GetProcessSlot(downloader_id)<>0)
		{
			WriteDebug("Browser Hack v2.0: Killing downloader and trying to run it one more!");
			KillProcess(downloader_id); //убиваем старый процесс
			downloader_id = RunProgram("/sys/network/downloader", #URL);
		}
		//
		IF (downloader_id<0) RunProgram("@notify", "Error running Downloader. Internet unavilable.");
		Draw_Window();
		return;
	}
	ReadHtml();
	if (filesize) wintodos(buf);
	WB1.ShowPage(#URL);
}


void TWebBrowser::ShowPage(dword adress)
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
	
	ParseHTML(buf, filesize);
	IF (!strcmp(#version, #header)) DrawTitle(#header);
}



void TWebBrowser::ParseHTML(dword bword, fsize){
	word bukva[1];
	byte ignor_param = 0;
	char temp[768];
	stroka = -za_kadrom;
	stolbec = 0;
	FOR(j = 400; j < blink + 1; j++;) DeleteButton(j);
	b_text = i_text = u_text = s_text = pre_text = blq_text = body_present =
	li_text = link = ignor_text = text_color_index = text_colors[0] = li_tab = 0; //обнуляем теги
	link_color = 0x0000FF;
	bg_color = 0xFFFFFF;
	blink = 400;
	line = '';
	copystr("|", #page_links);
	copystr(#version, #header);
	IF(!strcmp(#URL + strlen(#URL) - 4, ".txt"))
	{
		DrawBar(left, top, width-15, 15, bg_color); //закрашиваем первую строку
		pre_text = 1; //зачётное отображение текста 
	}
	IF(!strcmp(#URL + strlen(#URL) - 4, ".mht")) ignor_text = 1;
	for (bword = buf; buf + fsize > bword; bword++;) {
	  bukva = ESBYTE[bword];
	  switch (bukva) {
		case 0x0a:
			IF(pre_text == 1) {
				bukva = '';
				temp = '';
				goto NEXT_MARK;
			}
		case '\9':
			if (pre_text == 1) //иначе идём на 0x0d	
			{
				tab_len=strlen(#line)/8;
				tab_len=tab_len*8;
				tab_len=8+tab_len-strlen(#line);
				for (i=0; i<tab_len; i++;) copystr(" ", #line + strlen(#line));
				break;
			}		
		case 0x0d:
			bukva = ' ';
			goto DEFAULT_MARK;
		case '<':
			bword++; //промотаем символ <
			IF(ESBYTE[bword] == '!') //фильтрация внутри <!-- -->, дерзко
			{
				bword++;
				IF(ESBYTE[bword] == '-') {
					HH_: do {
						bword++;
						IF(bword >= buf + fsize) break 1;
					} while (ESBYTE[bword] <>'-');
					bword++;
					IF(ESBYTE[bword] <>'-') GOTO HH_;
				}
			}
			WHILE (ESBYTE[bword] <>'>') && (bword < buf + fsize) //получаем тег и его параметры
			{
				bukva = ESBYTE[bword];
				IF(bukva == '\9') || (bukva == '\x0a') || (bukva == '\x0d') bukva = ' ';
				IF(!ignor_param) && (bukva <>' ') copystr(#bukva, #tag + strlen(#tag));
				ELSE {
					ignor_param = true;
					copystr(#bukva, #tagparam + strlen(#tagparam));
				}
				bword++;
			}
			lowcase(#tag);
			lowcase(#tagparam);

			IF (tag[strlen(#tag)-1]=='/') tag[strlen(#tag)-1]=''; //небольшой фикс для работы с XHTML-тегами типа br/
			IF(strlen(#tagparam) > 0) && (strlen(#tagparam) < 4000) GetNextParam();
			WhatTextStyle(left + 5, stroka * 10 + top + 5, width - 20); //обработка тегов

			line = tag = parametr = tagparam = ignor_param = 0; //всё обнуляем
			break;
		case '=': //поддержка шайтанской кодировки страниц, сохранённых через ИЕ7
			IF(strcmp(#URL + strlen(#URL) - 4, ".mht")<>0) goto DEFAULT_MARK;

			bword++;
			bukva=ESBYTE[bword];
			copystr(#bukva, #temp);

			bword++;
			bukva=ESBYTE[bword];
			copystr(#bukva, #temp + strlen(#temp));
			
			bukva=Hex2Symb(#temp);
			IF (bukva) goto DEFAULT_MARK;
			break;
			
		case '&': //обработка тегов типа &nbsp;
			IF(ignor_text) break;
			bword++;
			tag='';
			FOR (j=0;   (ESBYTE[bword] <>';') && (j < 7);     j++; bword++;)
			{
				bukva = ESBYTE[bword];
				copystr(#bukva, #tag + strlen(#tag));
			}
			
			FOR (j=0; unicode_tags[j]!=0; j+=2;) 
			{
				IF(!strcmp(#tag, unicode_tags[j]))
				{
					copystr(unicode_tags[j+1], #line + strlen(#line));
					break 1;
				}
			}
			
			rez = StrToInt(#tag + 1) - 1040;
			IF(tag[1] == '1') && (rez>=0) && (rez<=72) && (strlen(#tag) == 5)
				{
					bukva = unicode_chars[rez];
					GOTO DEFAULT_MARK; //обрабатываем букву
				}
			
			//WriteDebug(#tag); //тэг не найден - выводим на доску отладки
			copystr(#tag, #line + strlen(#line)); //выводим на экран необработанный тег, так браузеры зачем-то делают
			break;
		default:
			DEFAULT_MARK:
			IF(ignor_text) break;
			IF(pre_text == 0) && (bukva == ' ') && (strcmp(#line + strlen(#line) - 1, " ") == 0) continue;
			//
			if (stolbec + strlen(#line) > max_kolvo_stolbcov)
			{
				copystr(#line + find_symbol(#line, ' '), #temp); //перенос по словам
				line[find_symbol(#line, ' ')] = 0x00;
				NEXT_MARK: IF(stroka - 1 > max_kolvo_strok) && (za_kadrom <>0) break 1; //уходим...
				WhatTextStyle(left + 5, stroka * 10 + top + 5, width - 20); //вывод строки
				TextGoDown(left + 5, stroka * 10 + top + 5, width - 20); //закрашиваем следущую строку
				copystr(#temp, #line);
			}
			IF(pre_text == 0) && (bukva == ' ') && (stolbec == 0) && (strlen(#line) == 0) CONTINUE;
			copystr(#bukva, #line + strlen(#line));
	  }
	}
	if (strcmp(#URL + strlen(#URL) - 4, ".txt")<>0) && (body_present==0)
		DrawBar(left, top, width-15, 15, bg_color); //закрашиваем первую строку если какой-то рахит не создал тег боди

	if (max_kolvo_strok * 10 + 25 <= height)
		DrawBar(left, max_kolvo_strok * 10 + top + 25, width - 15, -max_kolvo_strok * 10 + height - 25, bg_color);
	if (stroka * 10 + 15 <= height)
		DrawBar(left, stroka * 10 + top + 15, width - 15, -stroka * 10 + height - 15, bg_color); //закрашиваем всё до конца
	if (za_kadrom == 0) count = stroka;
	DrawScroller(); //рисуем скролл
}


void GetNextParam()
{
	byte	kavichki = false;
	int		i = strlen(#tagparam) - 1;
	
	WHILE((i > 0) && ((tagparam[i] == '"') || (tagparam[i] == ' ') || (tagparam[i] == '\'') || (tagparam[i] == '/')))
	{
		IF (tagparam[i] == '"') || (tagparam[i] == '\'') kavichki=tagparam[i];
		tagparam[i] = 0x00;
		i--;
	}

	IF (kavichki)
	{
		i=find_symbol(#tagparam, kavichki);
		copystr(#tagparam + i, #options);
	}
	ELSE
	{
		WHILE((i > 0) && (tagparam[i] <>'=')) i--; //i=find_symbol(#tagparam, '=')+1;
		i++;
		
		copystr(#tagparam + i, #options); //копируем опцию
		WHILE (options[0] == ' ') copystr(#options + 1, #options);
	}
	tagparam[i] = 0x00;

	FOR ( ; ((tagparam[i] <>' ') && (i > 0); i--)
	{
		IF (tagparam[i] == '=') //дерзкая заглушка
		{
			//copystr(#tagparam+i+2,#options);
			tagparam[i + 1] = 0x00;
		}
	}

	copystr(#tagparam + i + 1, #parametr); //копируем параметр
	tagparam[i] = 0x00;
}



char oldtag[100];
void TWebBrowser::WhatTextStyle(int left1, top1, width1) {
	dword hr_color;

    dword image=0;
    char temp[4096];
    int w, h, img_za_kadrom=0;

	//проверяем тег открывается или закрывается
	IF(tag[0] == '/') 
	{
		rez = 0;
		copystr(#tag + 1, #tag);
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
		copystr(#line, #header);
		copystr(" -", #header + strlen(#header));
		copystr(#version, #header + strlen(#header));
		if (stroka==0) DrawTitle(#header);
		return;
	}
	
	IF (ignor_text == 1) return;
	//
	
	//
	IF(!chTag("q")) copystr("\"", #line + strlen(#line));
	
	//вывод на экран
	if (stroka >= 0) && (stroka - 2 < max_kolvo_strok) && (line) 
	{
		WriteText(stolbec * 6 + left1, top1, 0x80, text_colors[text_color_index], #line, 0); //может тут рисовать белую строку?
		IF (b_text)	{ $add ebx, 1<<16   $int 0x40 }
		IF (i_text) Skew(stolbec * 6 + left1, top1, strlen(#line)+1*6, 10); //наклонный текст
		IF (s_text) DrawBar(stolbec * 6 + left1, top1 + 4, strlen(#line) * 6, 1, text_colors[text_color_index]); //зачёркнутый
		IF (u_text) DrawBar(stolbec * 6 + left1, top1 + 8, strlen(#line) * 6, 1, text_colors[text_color_index]); //подчёркнутый
		IF (link) {
			DefineButton(stolbec * 6 + left1 - 2, top1, strlen(#line) * 6 + 3, 9, blink + BT_HIDE, 0xB5BFC9); //
			DrawBar(stolbec * 6 + left1, top1 + 8, strlen(#line) * 6, 1, text_colors[text_color_index]);
		}
	}
	//
	IF(!tag) return;
	stolbec += strlen(#line);

	if (!chTag("body"))
	{
		BODY_MARK:
		
		if (strcmp(#parametr, "link=") == 0)
			link_color = GetColor(#options);
		
		if (strcmp(#parametr, "text=") == 0)
		{
			text_colors[0]=GetColor(#options);
		}
		
		if (strcmp(#parametr, "bgcolor=") == 0)
		{
			bg_color=GetColor(#options);
		}
		
		IF(tagparam) {
			GetNextParam();
			GOTO BODY_MARK;
		}
		
		body_present = 1; //если калич не создал тег боди нужно извращаться

		if (rez) DrawBar(WB1.left, WB1.top, WB1.width-15, 15, bg_color); //закрашиваем первую строку
		return;
	}
	//////////////////////////
	if (!chTag("a")) {
		IF (stroka - 1 > max_kolvo_strok) || (stroka < -2) return;
		if (rez) {
			HREF: IF(strcmp(#parametr, "href=") == 0) {
				IF(link == 1) text_color_index--; //если какой-то долбоёб не закрыл тэг
				link = 1;
				blink++;
				text_color_index++;
				text_colors[text_color_index] = link_color;
				copystr(#options, #page_links + strlen(#page_links));
				copystr("|", #page_links + strlen(#page_links));
			}
			IF(tagparam) {
				GetNextParam();
				GOTO HREF;
			}
		}
		ELSE {
			link = 0;
			IF(text_color_index > 0) text_color_index--;
		}
		return;
	}
	/////////////////////////
	if (!chTag("font"))
	{
		IF (stroka - 1 > max_kolvo_strok) return;
		COL_MARK:
		if (strcmp(#parametr, "color=") == 0) //&& (parametr[1] == '#')
		{
			text_color_index++;
			text_colors[text_color_index] = GetColor(#options);
		}
		IF(tagparam) {
			GetNextParam();
			GOTO COL_MARK;
		}
		IF(!rez) && (text_color_index > 0) text_color_index--;
		return;
	}
	//////////////////////////
	IF(!chTag("tr")) || (!chTag("br")) {
		TextGoDown(left1, top1, width1);
		return;
	}
	IF(!chTag("div")) {
		IF(oldtag[0] <>'h') TextGoDown(left1, top1, width1);
		return;
	}
	IF(!chTag("p")) {
		IF(oldtag[0] == 'h') return;
		TextGoDown(left1, top1, width1);
		IF(rez) TextGoDown(left1, top1 + 10, width1);
		return;
	}
	////////////////////////////
	IF(!chTag("h1")) || (!chTag("h2")) || (!chTag("h3")) || (!chTag("h4")) {
		TextGoDown(left1, top1, width1);
		IF(rez) TextGoDown(left1, top1 + 10, width1);
		b_text = rez;
		copystr(#tag, #oldtag);
		return;
	} ELSE copystr("", #oldtag);
	IF(!chTag("b")) || (!chTag("strong")) || (!chTag("big")) {
		b_text = rez;
		return;
	}
	////////////////////////////
	IF(!chTag("i")) || (!chTag("em")) || (!chTag("subtitle")) {
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
	if(!chTag("li")) //надо сделать вложенные списки
	{
		li_text = rez;
		IF(rez == 0) return;
		TextGoDown(left1, top1, width1);
		IF(stroka > -1) && (stroka - 2 < max_kolvo_strok) DrawBar(li_tab * 5 * 6 + left1 - 5, top1 + 12, 2, 2, 0);
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
		//IF (GetFileInfo(#libimg)<>0) return;  //если библиотеки нет
		IMG_TAG:
			IF (strcmp(#parametr,"src=")==0)   //надо объединить с GetNewUrl()
	          {
				copystr(BrowserHistory.CurrentUrl(), #temp); //достаём адрес текущей страницы
				temp[find_symbol(#temp, '/')] = 0x00; //обрезаем её урл до последнего /
				copystr(#options,#temp+strlen(#temp));
				image=load_image(#temp);
	
	                w=DSWORD[image+4];
	                h=DSWORD[image+8]; 
	          }
		IF(tagparam) {
			GetNextParam();
			GOTO IMG_TAG;
		}

		if (w>width1) w=width1;
		
        if (image)
        {
			stroka+=h/10;
			
			if (top1+h<WB1.top) || (top1>WB1.top+WB1.height-10) //если ВСЁ изображение ушло ВЕРХ или ВНИЗ
				return;

			if (top1<WB1.top) //если часть изображения сверху
			{
				img_za_kadrom=WB1.top-top1;
				h=h-img_za_kadrom;
				top1=WB1.top;
			}
			
			if (top1>WB1.top+WB1.height-h-15) //если часть изображения снизу     IF (stroka - 2 < max_kolvo_strok)
			{
				h=WB1.top+WB1.height-top1-15;
			}	

			IF (h<=0) return;
			
			img_draw stdcall (image,left1-5,top1+10,w, h,0,img_za_kadrom);
			DrawBar(left1+w - 5, top1 + 10, width1-w + 5, h, bg_color);
			IF (link)
			{
				DefineButton(left1 - 5, top1+10, w, h, blink + BT_HIDE, 0xB5BFC9);
			}

        }
		/*else
		{
			IF (strcmp(#parametr,"alt=")==0) copystr(#options,#line+strlen(#line));
		}*/
		return;
	}

	if (!chTag("meta")) || (!chTag("?xml"))
	{
		META:
		if (!strcmp(#parametr, "charset=")) || (!strcmp(#parametr, "content=")) || (!strcmp(#parametr, "encoding="))
		{
			copystr(#options[find_symbol(#options, '=')],#options); //поиск в content=

			IF (!strcmp(#options,"utf-8")) || (!strcmp(#options,"utf8"))
			{
				ReadHtml();
				utf8rutodos(buf);
			}
			IF(!strcmp(#options, "koi8-r")) || (!strcmp(#options, "koi8-u"))
			{
				ReadHtml();
				koitodos(buf);
			}
			IF(!strcmp(#options, "dos")) || (!strcmp(#options, "cp-866"))
			{
				ReadHtml();
			}
		}
		IF(tagparam)
		{
			GetNextParam();
			goto META;
		}
		return;
	}
}


void TextGoDown(int left1, top1, width1)
{
	stroka++;
	IF(blq_text == 1) stolbec = 8;
	ELSE stolbec = 0;
	IF(li_text == 1) stolbec = li_tab * 5;
	IF(stroka >= 0) && (stroka - 2 < max_kolvo_strok) DrawBar(left1 - 5, top1 + 10, width1 + 5, 10, bg_color);
}


//скролл
void TWebBrowser::DrawScroller() //не оптимальная отрисовка, но зато в одном месте
{
	scroll1.max_area = count;
	scroll1.cur_area = max_kolvo_strok;
	scroll1.position = za_kadrom;

	scroll1.all_redraw=1;
	scroll1.start_x=Form.width-28; //left + width - 15
	scroll1.size_y=WB1.height;

	scrollbar_v_draw(#scroll1);

	DefineButton(scroll1.start_x+1, scroll1.start_y+1, 16, 16, ID1+BT_HIDE, 0xE4DFE1);
	DefineButton(scroll1.start_x+1, scroll1.start_y+scroll1.size_y-18, 16, 16, ID2+BT_HIDE, 0xE4DFE1);
}