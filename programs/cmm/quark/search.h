
#define SEARCH_H 34


struct SEARCH
{
	bool visible;
	int found_count;
	void show();
	void hide();
	bool draw();
	bool edit_key();
	bool edit_mouse();
	void clear();
	int find_next();
} search;

char found_text[64];

char search_text[64];
edit_box search_box = {250, 10, NULL, 0xffffff,
0x94AECE, 0xffffff, 0xffffff,0x10000000,sizeof(search_text)-1,#search_text};


void SEARCH::show()
{
	visible = true;
	search_box.flags = ed_focus;
	draw_window();
}

void SEARCH::hide()
{
	visible = false;
	draw_window();
}

bool SEARCH::edit_key()
{
	if (visible) && (search_box.flags & ed_focus) {
		EAX = key_editbox; 
		edit_box_key stdcall(#search_box);
		return true;
	}
	return false;
}

bool SEARCH::edit_mouse()
{
	if (visible) {
		edit_box_mouse stdcall(#search_box);
		if (search_box.flags & ed_focus) return true;
	}
	return false;
}

bool SEARCH::draw(dword _btn_find, _btn_hide, _y)
{
	char matches[30];
	if (!visible) return false;
	DrawBar(0, _y, Form.cwidth, 1, sc.work_graph);
	DrawBar(0, _y+1, Form.cwidth, SEARCH_H-1, sc.work);

	search_box.top = _y + 6;
	search_box.width = math.min(Form.width - 200, 250);

	DrawRectangle(search_box.left-1, search_box.top-1, search_box.width+2, 23,sc.work_graph);

	edit_box_draw stdcall(#search_box);

	DrawCaptButton(search_box.left+search_box.width+14, search_box.top-1, 90, 
		TOOLBAR_ICON_HEIGHT+1, _btn_find, sc.work_light, sc.work_text, T_FIND_NEXT);

	sprintf(#matches, T_MATCHES, found_count);
	WriteTextWithBg(search_box.left+search_box.width+14+110, 
		search_box.top+3, 0xD0, sc.work_text, #matches, sc.work);

	DefineHiddenButton(Form.cwidth-26, search_box.top-1, TOOLBAR_ICON_HEIGHT+1, 
		TOOLBAR_ICON_HEIGHT+1, _btn_hide);
	WriteText(Form.cwidth-26+7, search_box.top+2, 0x81, sc.work_graph, "x");
	return true;
}

void SEARCH::clear()
{
	visible = false;
	found_text[0] = '\0';
	found_count = 0;
}

int SEARCH::find_next(int _cur_pos)
{
	int i;
	if (!search_text[0]) return false;

	strcpy(#found_text, #search_text);
	found_count = strinum(io.buffer_data, #found_text);
	draw_window();

	for (i=_cur_pos+1; i<list.count; i++) {
		if (strstri(lines.get(i),#search_text)) return i;
	}
	return false;
}

