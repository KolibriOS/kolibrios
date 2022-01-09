
//===================================================//
//                                                   //
//                      MODULE                       //
//                                                   //
//===================================================//

#define TABS_MAX 5

TWebBrowser tabdata[TABS_MAX+1]=0;
_history tabstory[TABS_MAX+1]=0;

struct TAB
{
	int count;
	signed int active;
	bool add();
	bool close();
	void save_state();
	void restore();
} tab = {1,0};


bool TAB::add()
{
	if (count==TABS_MAX) return false;
	save_state();
	count++;
	active = count-1;
	history = tabstory[active];
	return true;
}

bool TAB::close(int _tab_number)
{
	int i;
	if (count==1) return false;
	for (i=_tab_number; i<TABS_MAX; i++) {
		tabdata[i] = tabdata[i+1];
		tabstory[i] = tabstory[i+1];
	}
	if (_tab_number<active) && (active>0) active--;
	if (active==count-1) && (active>0) active--;
	count--;
	return true;
}

void TAB::save_state()
{
	tabstory[active] = history;
	tabdata[active] = WB1;
}

void TAB::restore(int _id)
{
	tab.active = _id;
	WB1 = tabdata[_id];	
	history = tabstory[_id];
}

//===================================================//
//                                                   //
//                 WebView Actions                   //
//                                                   //
//===================================================//

#define DEFAULT_TABW 220
int tab_w = DEFAULT_TABW;

int GetTabWidth()
{
	if (tab.count == TABS_MAX) return Form.cwidth / tab.count;
	if (tab.count * DEFAULT_TABW + TAB_H < Form.cwidth) return DEFAULT_TABW; else 
	return Form.cwidth - TAB_H - 2 / tab.count;
}

void DrawTab(int _id)
{
	#define CLOSE_S 13
	dword bgcol, border_bottom_color;
	char header_no_version[sizeof(TWebBrowser.header)];
	char name[DEFAULT_TABW/6];
	int xxx = _id * tab_w;

	if (_id==tab.active) {
		tab.save_state();
		bgcol = sc.light; 
		border_bottom_color = sc.light;
	} else {
		bgcol=sc.work;
		border_bottom_color = sc.line;
	}
	if (tabdata[_id].header) {
		strncpy(#header_no_version, #tabdata[_id].header, strlen(#tabdata[_id].header)-sizeof(version)-2);
		strncpy(#name, #header_no_version, tab_w-CLOSE_S/6-2);
	}
	DrawBar(xxx, TOOLBAR_H, 1, TAB_H, sc.dark);
	DrawBar(xxx+1, TOOLBAR_H, tab_w-1, TAB_H-1, bgcol);
	DrawBar(xxx+1, TOOLBAR_H+TAB_H-1, tab_w-1, 1, border_bottom_color);
	DefineHiddenButton(xxx, TOOLBAR_H-1, tab_w, TAB_H, TAB_ID+_id);
	WriteTextCenter(xxx, TOOLBAR_H+6, tab_w-CLOSE_S, sc.work_text, #name);

	DefineHiddenButton(xxx+tab_w-CLOSE_S-3, TOOLBAR_H+3, CLOSE_S-1, CLOSE_S-1, TAB_CLOSE_ID+_id);
	DrawBar(xxx+tab_w-CLOSE_S-3, TOOLBAR_H+3, CLOSE_S, CLOSE_S, sc.dark);
	WriteText(xxx+tab_w-CLOSE_S+1, TOOLBAR_H+5, 0x80, sc.light, "x");
}

void DrawActiveTab()
{
	if (tab_w == GetTabWidth())	DrawTab(tab.active);
	else DrawTabsBar();
}

int DrawNewTabButton()
{
	dword btn_light = MixColors(sc.button, 0xFFFfff, 220);
	dword btn_dark = MixColors(sc.button, 0, 180);
	int xxx = tab.count * tab_w;

	if (tab.count < TABS_MAX) {
		DrawBar(xxx, TOOLBAR_H, 1, TAB_H, sc.line);
		DrawBar(xxx+1, TOOLBAR_H, TAB_H, TAB_H-1, sc.button);
		DrawRectangle3D(xxx+1, TOOLBAR_H, TAB_H, TAB_H-1, btn_light, btn_dark);
		PutPixel(xxx+1+TAB_H, TOOLBAR_H, btn_dark);
		DefineHiddenButton(xxx+1, TOOLBAR_H, TAB_H-1, TAB_H-1, NEW_TAB);
		WriteText(xxx+7, TOOLBAR_H+2, 0x90, sc.button_text, "+");
		return xxx + TAB_H + 2;
	} else {
		return xxx;
	}
}

void DrawTabsBar()
{
	dword i;
	tab_w = GetTabWidth();
	for (i=0; i<tab.count; i++) DrawTab(i); 
	i = DrawNewTabButton();
	DrawBar(i, TOOLBAR_H, Form.cwidth-i, TAB_H-1, MixColors(sc.dark, sc.work, 128));
	DrawBar(i, TOOLBAR_H+TAB_H-1, Form.cwidth-i, 1, sc.line);
}

void EventTabClose(int _id)
{
	DeleteButton(TAB_ID + tab.count-1);
	if (_id == tab.active) {
		tab.close(_id);
		tab.restore(tab.active);
		SetElementSizes();
		WB1.Reparse();
		WB1.DrawPage();
		SetOmniboxText(history.current());
	} else {
		tab.close(_id);
	}
	DrawTabsBar();
	DrawStatusBar(NULL);
}

void EventCloseActiveTab()
{
	EventTabClose(tab.active);
}

void EventTabClick(int _id)
{
	if (_id>=tab.count) _id = 0;
	if (_id==-1) _id = tab.count-1;
	tab.save_state();
	tab.restore(_id);
	DrawTabsBar();
	SetElementSizes();
	WB1.Reparse();
	WB1.DrawPage();		
	SetOmniboxText(history.current());
	DrawStatusBar(NULL);
}

void EventOpenNewTab(dword _url)
{
	tab.add();
	OpenPage(_url);
	DrawTabsBar();
}

void EventActivateNextTab()
{
	EventTabClick(tab.active+1);
}

void EventActivatePreviousTab()
{
	EventTabClick(tab.active-1);
}

