struct array_link {
	dword link, text;
	int x,y,w,h;
};

struct LinksArray
{
	array_link links[200];
	char page_links[64000];
	dword buflen;
	int count, active;

	void Hover();
	void AddLink();
	void AddText();
	dword GetURL();
	void Clear();
};

void LinksArray::AddLink(dword new_link, int link_x, link_y)
{
	links[count].x = link_x;
	links[count].y = link_y;

	links[count].link = buflen;
	strcpy(buflen, new_link);
	buflen += strlen(new_link)+1;
	count++;
}

void LinksArray::AddText(dword new_text, int link_w, link_h)
{
	if (count<1) return;
	links[count-1].w = link_w;
	links[count-1].h = link_h;

	links[count-1].text = buflen;
	strcpy(buflen, new_text);
	buflen += strlen(new_text)+1;
}

dword LinksArray::GetURL(int id)
{
	return links[id].link;
}

void LinksArray::Clear()
{
	int i;
	for (i=0; i<=count; i++) DeleteButton(i+400);
	buflen = #page_links;
	count = 0;
	active = -1;
}

void LinksArray::Hover(dword mx, my, link_col_in, link_col_a, bg_col)
{
	int i;
	if (active>=0)
	{
		//WriteText(links[active].x,links[active].y, 0x80, link_col_in, links[active].text);
		DrawBar(links[active].x,links[active].y+8,links[active].w,1, link_col_in);
		active = -1;
	}
	for (i=0; i<count; i++)
	{
		if (mx>links[i].x) && (my>links[i].y) && (mx<links[i].x+links[i].w) && (my<links[i].y+links[i].h)
		{
			//WriteText(links[i].x,links[i].y, 0x80, link_col_a, links[i].text);
			DrawBar(links[i].x,links[i].y+8,links[i].w,1, bg_col);
			active = i;
			return;
		}
	}
}


LinksArray PageLinks;


/*
------------------ Подфункция 4 - загрузить курсор -------------------
Параметры:
  * eax = 37 - номер функции
  * ebx = 4 - номер подфункции
  * dx = источник данных:
  * dx = LOAD_FROM_FILE = 0 - данные в файле
    * ecx = указатель на полный путь к файлу курсора
    * файл курсора должен быть в формате .cur, стандартном для
      MS Windows, причём размером 32*32 пикселя
  * dx = LOAD_FROM_MEM = 1 - данные файла уже загружены в память
    * ecx = указатель на данные файла курсора
    * формат данных такой же, как и в предыдущем случае
  * dx = LOAD_INDIRECT = 2 - данные в памяти
    * ecx = указатель на образ курсора в формате ARGB 32*32 пикселя
    * edx = 0xXXYY0002, где
      * XX = x-координата "горячей точки" курсора
      * YY = y-координата
      * 0 <= XX, YY <= 31
Возвращаемое значение:
  * eax = 0 - неудача
  * иначе eax = хэндл курсора
 
------------------ Подфункция 5 - установить курсор ------------------
Устанавливает новый курсор для окна текущего потока.
Параметры:
  * eax = 37 - номер функции
  * ebx = 5 - номер подфункции
  * ecx = хэндл курсора
Возвращаемое значение:
  * eax = хэндл предыдущего установленного курсора
Замечания:
  * Если передан некорректный хэндл, то функция восстановит курсор
    по умолчанию (стандартную стрелку). В частности, к восстановлению
    курсора по умолчанию приводит передача ecx=0.
 
------------------- Подфункция 6 - удалить курсор --------------------
Параметры:
  * eax = 37 - номер функции
  * ebx = 6 - номер подфункции
  * ecx = хэндл курсора
Возвращаемое значение:
  * eax разрушается
Замечания:
  * Курсор должен был быть ранее загружен текущим потоком
    (вызовом подфункции 4). Функция не удаляет системные курсоры и
    курсоры, загруженные другими приложениями.
  * Если удаляется активный (установленный подфункцией 5) курсор, то
    восстанавливается курсор по умолчанию (стандартная стрелка).
    
*/