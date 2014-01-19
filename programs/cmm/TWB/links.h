struct array_link {
	dword link, text;
	int x,y,w,h;
};

struct LinksArray
{
	array_link links[100];
	char page_links[12000];
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