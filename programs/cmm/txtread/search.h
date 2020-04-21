
#include "../lib/collection.h"

#define SEARCH_H 34


struct SEARCH
{
	bool visible;
	int found_count;
	collection lines;
	collection pos;
	void show();
	void hide();
	bool draw();
	void draw_found();
	int height();
	bool edit_key();
	bool edit_mouse();
	void add();
	void clear();
	int find_next();
	int highlight();
} search;

char search_text[64];
char found_text[64];
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

int SEARCH::height()
{
	return visible * SEARCH_H;
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

void SEARCH::draw_found()
{
	strcpy(#param, "Matches: ");
	strcat(#param, itoa(found_count));
	strcat(#param, "   ");	
	WriteTextWithBg(search_box.left+search_box.width+14+110, search_box.top+3, 0xD0, sc.work_text, #param, sc.work);
}

bool SEARCH::draw(dword _btn_find, _btn_hide)
{
	if (!visible) return false;
	DrawBar(0,Form.cheight - SEARCH_H, Form.cwidth, 1, sc.work_graph);
	DrawBar(0,Form.cheight - SEARCH_H+1, Form.cwidth, SEARCH_H-1, sc.work);

	search_box.top = Form.cheight - SEARCH_H + 6;
	search_box.width = math.min(Form.width - 200, 250);

	DrawRectangle(search_box.left-1, search_box.top-1, search_box.width+2, 23,sc.work_graph);

	edit_box_draw stdcall(#search_box);

	DrawCaptButton(search_box.left+search_box.width+14, search_box.top-1, 90, 
		TOOLBAR_ICON_HEIGHT+1, _btn_find, sc.work_light, sc.work_text, "Find next");

	draw_found();

	DefineHiddenButton(Form.cwidth-26, search_box.top-1, TOOLBAR_ICON_HEIGHT+1, 
		TOOLBAR_ICON_HEIGHT+1, _btn_hide);
	WriteText(Form.cwidth-26+7, search_box.top+2, 0x81, sc.work_graph, "x");
	return true;
}

void SEARCH::clear()
{
	pos.drop();
	lines.drop();
	visible = false;
	found_text[0] = '\0';
	found_count = 0;
}

void SEARCH::add(dword _pos, _line)
{
	pos.add(itoa(_pos));
	lines.add(_line);
}

int SEARCH::find_next(int _cur_pos, _bg_color)
{
	int i;
	if (!search_text[0]) return false;

		strcpy(#found_text, #search_text);
		highlight(0xFF0000, _bg_color);
		draw_found();

	for (i=_cur_pos+1; i<pos.count; i++) {
		if (strstri(lines.get(i),#search_text)!=-1) return atoi(pos.get(i));
	}
	return false;
}

int SEARCH::highlight(dword _color, _bg_color)
{
	int i;
	dword col;
	found_count = 0;
	for (i=0; i<pos.count; i++) {
		if (strstri(lines.get(i),#search_text)==-1) {
			col=_bg_color;
		} else {
			col=_color;
			found_count++;	
		} 
		draw_bar(0, atoi(pos.get(i)), 3, list.item_h, col);
	}
}

void draw_bar(dword _x, _y, _w, _h, _color)
{
	int i;
	for (i = _y*list.w+_x*KFONT_BPP+kfont.raw ; i<_y*list.w+_x+_w*KFONT_BPP+kfont.raw ; i+=KFONT_BPP) 
	{
		ESDWORD[i] = _color;
	}
	if (_h>0) draw_bar(dword _x, _y+1, _w, _h-1, _color);
}