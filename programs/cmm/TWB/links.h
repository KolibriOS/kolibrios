CustomCursor CursorPointer;
dword CursorFile = FROM "../TWB/pointer.cur";

#define NOLINE    0
#define UNDERLINE 1

struct array_link {
	dword link, text;
	int x,y,w,h;
	int underline;
};

struct LinksArray {
	array_link links[400];
	char page_links[64000];
	dword buflen;
	int count, active;
	void Hover();
	void AddLink();
	void AddText();
	void ClickLink();
	dword GetURL();
	void Clear();
	void GetAbsoluteURL();
	int UrlAbsolute();
} PageLinks;

void LinksArray::AddLink(dword lpath, int link_x, link_y)
{
	links[count].x = link_x;
	links[count].y = link_y;

	links[count].link = buflen;
	strcpy(buflen, lpath);
	buflen += strlen(lpath)+1;
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
	signed int WBY =  -WB1.list.first*WB1.list.line_h - WB1.DrawBuf.zoom;
	for (i=0; i<count; i++)
	{
		if (mx>links[i].x) && (my>links[i].y) && (mx<links[i].x+links[i].w) && (my<links[i].y+links[i].h)
		{
			if (mouse.down) DrawRectangle(links[active].x, links[active].y, links[active].w, links[active].h, 0);
			if (mouse.up) ClickLink();
			if (active==i) return;
			CursorPointer.Set();
			if (links[active].underline) DrawBar(links[active].x, WBY + links[active].y + links[active].h,links[active].w, WB1.DrawBuf.zoom, link_col_in);
			if (links[i].underline) DrawBar(links[i].x, WBY + links[i].y + links[i].h,links[i].w, WB1.DrawBuf.zoom, bg_col);
			active = i;
			status_text.start_x = wv_progress_bar.left + wv_progress_bar.width + 10;
			status_text.start_y = Form.cheight - STATUSBAR_H + 3;
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
		if (links[active].underline) DrawBar(links[active].x, WBY + links[active].y  + links[active].h,links[active].w, WB1.DrawBuf.zoom, link_col_in);
		DrawBar(status_text.start_x, status_text.start_y, status_text.area_size_x, 9, col_bg);
		active = -1;
	}
}

int LinksArray::UrlAbsolute(dword in_URL)
{
	if(!strncmp(in_URL,"http:",5)) return 1;
	if(!strncmp(in_URL,"https:",6)) return 1;
	if(!strncmp(in_URL,"mailto:",7)) return 1;
	if(!strncmp(in_URL,"ftp:",4)) return 1;
	if(!strncmp(in_URL,"WebView:",8)) return 1;
	if(!strncmp(in_URL,"/sys/",5)) return 1;
	if(!strncmp(in_URL,"/hd/",4)) return 1;
	if(!strncmp(in_URL,"/fd/",4)) return 1;
	if(!strncmp(in_URL,"/rd/",4)) return 1;
	if(!strncmp(in_URL,"/tmp/",5)) return 1;
	if(!strncmp(in_URL,"/cd/",4)) return 1;
	if(!strncmp(in_URL,"/bd/",4)) return 1;
	if(!strncmp(in_URL,"/usbhd/",7)) return 1;
	if(!strncmp(in_URL,"/kolibrios/",11)) return 1;
	return 0;
}

void LinksArray::GetAbsoluteURL(dword in_URL)
{
	int i;
	dword orig_URL = in_URL;
	char newurl[sizeof(URL)];

	if (UrlAbsolute(in_URL)) return;
	
	IF (!strcmpn(in_URL,"./", 2)) in_URL+=2;
	if (!http_transfer) 
	{
		strcpy(#newurl, BrowserHistory.CurrentUrl());
	}
	else
	{
		strcpy(#newurl, #history_list[BrowserHistory.current-1].Item); 
	}

	if (ESBYTE[in_URL] == '/') //remove everything after site domain name
	{
		i = strchr(#newurl+8, '/');
		if (i) ESBYTE[i]=0;
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

void LinksArray::ClickLink()
{
	if (http_transfer > 0) 
	{
		StopLoading();
		BrowserHistory.current--;
	}

	strcpy(#URL, PageLinks.GetURL(PageLinks.active));	
	//#1
	if (URL[0] == '#')
	{
		strcpy(#anchor, #URL+strrchr(#URL, '#'));		
		strcpy(#URL, BrowserHistory.CurrentUrl());
		WB1.list.first=WB1.list.count-WB1.list.visible;
		ShowPage();
		return;
	}
	//liner.ru#1
	if (strrchr(#URL, '#')!=-1)
	{
		strcpy(#anchor, #URL+strrchr(#URL, '#'));
		URL[strrchr(#URL, '#')-1] = 0x00;
	}
	
	PageLinks.GetAbsoluteURL(#URL);
	
	if (UrlExtIs(".png")==1) || (UrlExtIs(".gif")==1) || (UrlExtIs(".jpg")==1) || (UrlExtIs(".zip")==1) || (UrlExtIs(".kex")==1)
	|| (UrlExtIs(".7z")==1) || (UrlExtIs("netcfg")==1) 
	{
		//notify(#URL);
		if (!strncmp(#URL,"http://", 7))
		{
			strcpy(#DL_URL, #URL);
			CreateThread(#Downloader,#downloader_stak+4092);
		}
		else RunProgram("@open", #URL);
		strcpy(#editURL, BrowserHistory.CurrentUrl());
		strcpy(#URL, BrowserHistory.CurrentUrl());
		return;
	}
	if (!strncmp(#URL,"mailto:", 7))
	{
		notify(#URL);
		strcpy(#editURL, BrowserHistory.CurrentUrl());
		strcpy(#URL, BrowserHistory.CurrentUrl());
		return;
	}
	OpenPage();
	return;
}
