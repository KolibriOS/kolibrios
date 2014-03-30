CustomCursor CursorPointer;
dword CursorFile = FROM "../TWB/pointer.cur";

#define NOLINE    0
#define UNDERLINE 1


struct array_link {
	dword link, text;
	int x,y,w,h;
	int underline;
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
	void GetAbsoluteURL();
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

void LinksArray::AddText(dword new_text, int link_w, link_h, link_underline)
{
	if (count<1) return;
	links[count-1].w = link_w;
	links[count-1].h = link_h;
	links[count-1].underline = link_underline;

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
	CursorPointer.Restore();
}

char temp[sizeof(URL)];
PathShow_data status_text = {0, 17,250, 6, 250, 0, 0, 0x0, 0xFFFfff, 0, #temp, 0};

void LinksArray::Hover(dword mx, my, link_col_in, link_col_a, bg_col)
{
	int i;
	for (i=0; i<count; i++)
	{
		if (mx>links[i].x) && (my>links[i].y) && (mx<links[i].x+links[i].w) && (my<links[i].y+links[i].h)
		{
			if (active==i) return;
			CursorPointer.Set();
			if (links[active].underline) DrawBar(links[active].x,links[active].y+10,links[active].w,1, link_col_in);
			if (links[i].underline) DrawBar(links[i].x,links[i].y+10,links[i].w,1, bg_col);
			active = i;
			status_text.start_x = progress_bar.left+progress_bar.width+10;
			status_text.start_y = Form.cheight-STATUSBAR_H+3;
			status_text.area_size_x = Form.cwidth - status_text.start_x -3;
			DrawBar(status_text.start_x, status_text.start_y, status_text.area_size_x, 9, col_bg);
			status_text.text_pointer = links[active].link;
			PathShow_prepare stdcall(#status_text);
			PathShow_draw stdcall(#status_text);
			return;
		}
	}
	if (active!=-1)
	{
		CursorPointer.Restore();
		if (links[active].underline) DrawBar(links[active].x,links[active].y+10,links[active].w,1, link_col_in);
		DrawBar(status_text.start_x, status_text.start_y, status_text.area_size_x, 9, col_bg);
		active = -1;
	}
}

char *ABSOLUTE_LINKS[]={ "http:", "mailto:", "ftp:", "/sys/", 
"/kolibrios/", "/rd/", "/bd", "/hd", "/cd", "/tmp", "/usbhd", "WebView:", 0};
void LinksArray::GetAbsoluteURL(dword in_URL){
	int i, len;
	dword orig_URL = in_URL;
	char newurl[sizeof(URL)];
	
	for (i=0; ABSOLUTE_LINKS[i]; i++)
	{
		len=strlen(ABSOLUTE_LINKS[i]);
		if (!strcmpn(in_URL, ABSOLUTE_LINKS[i], len)) return;
	}
	IF (!strcmpn(in_URL,"./", 2)) in_URL+=2;
	strcpy(#newurl, BrowserHistory.CurrentUrl());

	if (ESBYTE[in_URL] == '/')
	{
		i = strchr(#newurl+8, '/');
		if (i>0) newurl[i+7]=0;
		in_URL+=1;
	}
		
	_CUT_ST_LEVEL_MARK:
		
	if (newurl[strrchr(#newurl, '/')-2]<>'/')
	{
		newurl[strrchr(#newurl, '/')] = 0x00;
	}
	
	IF (!strncmp(in_URL,"../",3))
	{
		in_URL+=3;
		newurl[strrchr(#newurl, '/')-1] = 0x00;
		goto _CUT_ST_LEVEL_MARK;
	}
	
	if (newurl[strlen(#newurl)-1]<>'/') strcat(#newurl, "/"); 
	
	strcat(#newurl, in_URL);
	strcpy(orig_URL, #newurl);
}



LinksArray PageLinks;
