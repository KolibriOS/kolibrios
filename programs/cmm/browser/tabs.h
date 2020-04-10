
//===================================================//
//                                                   //
//                      MODULE                       //
//                                                   //
//===================================================//

#define TABS_MAX 5

TWebBrowser data[TABS_MAX+1];
_history tabstory[TABS_MAX+1];

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
	for (i=_tab_number; i<=TABS_MAX; i++) {
		data[i] = data[i+1];
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
	data[active] = WB1;
}

void TAB::restore(int _id)
{
	tab.active = _id;
	WB1 = data[_id];	
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
		bgcol = system.color.work_light; 
		border_bottom_color = system.color.work_light;
	} else {
		bgcol=system.color.work;
		border_bottom_color = system.color.work_graph;
	}
	if (data[_id].header) {
		strncpy(#header_no_version, #data[_id].header, strlen(#data[_id].header)-sizeof(version)-2);
		strncpy(#name, #header_no_version, tab_w-CLOSE_S/6-2);
	}
	DrawBar(xxx, TOOLBAR_H, 1, TAB_H, system.color.work_dark);
	DrawBar(xxx+1, TOOLBAR_H, tab_w-1, TAB_H-1, bgcol);
	DrawBar(xxx+1, TOOLBAR_H+TAB_H-1, tab_w-1, 1, border_bottom_color);
	DefineHiddenButton(xxx, TOOLBAR_H-1, tab_w, TAB_H, TAB_ID+_id);
	WriteTextCenter(xxx, TOOLBAR_H+6, tab_w-CLOSE_S, system.color.work_text, #name);

	DefineHiddenButton(xxx+tab_w-CLOSE_S-3, TOOLBAR_H+3, CLOSE_S-1, CLOSE_S-1, TAB_CLOSE_ID+_id);
	DrawBar(xxx+tab_w-CLOSE_S-3, TOOLBAR_H+3, CLOSE_S, CLOSE_S, system.color.work_dark);
	WriteText(xxx+tab_w-CLOSE_S+1, TOOLBAR_H+5, 0x80, system.color.work_light, "x");
}

void DrawActiveTab()
{
	if (tab_w == GetTabWidth())	DrawTab(tab.active);
	else DrawTabsBar();
}

void DrawNewTabButton()
{
	dword btn_light = MixColors(system.color.work_button, 0xFFFfff, 220);
	dword btn_dark = MixColors(system.color.work_button, 0, 180);
	int xxx = tab.count * tab_w;
	DrawBar(xxx, TOOLBAR_H, 1, TAB_H, system.color.work_graph);
	DrawBar(xxx+1, TOOLBAR_H, TAB_H, TAB_H-1, system.color.work_button);
	DrawRectangle3D(xxx+1, TOOLBAR_H, TAB_H, TAB_H-1, btn_light, btn_dark);
	PutPixel(xxx+1+TAB_H, TOOLBAR_H, btn_dark);
	DefineHiddenButton(xxx+1, TOOLBAR_H, TAB_H-1, TAB_H-1, NEW_TAB);
	WriteText(xxx+7, TOOLBAR_H+2, 0x90, system.color.work_button_text, "+");	
}

void DrawTabsBar()
{
	dword i;
	tab_w = GetTabWidth();
	for (i=0; i<tab.count; i++) DrawTab(i);
	DrawNewTabButton();
	i = tab_w * i + TAB_H + 2;
	DrawBar(i, TOOLBAR_H, Form.cwidth-i, TAB_H-1, MixColors(system.color.work_dark, system.color.work, 128));
	DrawBar(i, TOOLBAR_H+TAB_H-1, Form.cwidth-i, 1, system.color.work_graph);
}

void EventTabClose(int _id)
{
	DeleteButton(tab.count);
	if (_id == tab.active) {
		tab.close(_id);
		tab.restore(tab.active);
		WB1.ParseHtml(WB1.bufpointer, WB1.bufsize);
		WB1.DrawPage();
		SetOmniboxText(history.current());
	} else {
		tab.close(_id);
	}
	DrawTabsBar();
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
	SetElementSizes();
	if (!BrowserWidthChanged()) {
		DrawTabsBar();
		WB1.ParseHtml(WB1.bufpointer, WB1.bufsize);
		WB1.DrawPage();		
	}
	SetOmniboxText(history.current());
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


:void DebugTabs()
{
	debugln("\n\n\nHISTORY==========================");
	history.add("history");
	history.debug();

	debugln("\n\n\nTABSTORY[0]======================");
	tabstory[0].add("tabstory0");
	tabstory[0].debug();

	debugln("\n\n\nTABSTORY[1]======================");
	tabstory[1].add("tabstory1");
	tabstory[1].debug();

	debugln("\n\n\n\n");
	debugval("history.items.data_start", history.items.data_start);
	debugval("tabstory[0].items.data_start", tabstory[0].items.data_start);
	debugval("tabstory[1].items.data_start", tabstory[1].items.data_start);
	debugln("\n\n\n\n");
}
