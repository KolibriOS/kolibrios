
#define SEARCH_H 34


struct SEARCH
{
	bool visible;
	int found_count;
	collection_int found;
	void show();
	void hide();
	bool edit_key();
	bool edit_mouse();
	int find_all();
	int find_next();
	int find_prior();
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

int SEARCH::find_all()
{
	dword haystack = textbuf.p;
	int needle_len = strlen(#found_text);
	found.drop();
	loop() {
		if (! haystack = strstri(haystack, #found_text)) break;
		found.add(haystack - needle_len);
		haystack += needle_len;
	}
}

int SEARCH::find_next(int _first)
{
	int i;
	if (!search_text[0]) return false;

	if (!streq(#found_text, #search_text)) {
		strcpy(#found_text, #search_text);
		find_all();
		draw_window();
	}

	for (i=0; i<found.count; i++) {
		if (signed found.get(i) - lines.get(_first) > 0) {
			while(signed found.get(i) - lines.get(_first) > 0) _first++;
			return _first-1;
		}
	}
	return false;
}

int SEARCH::find_prior(int _first)
{
	int i;
	if (!search_text[0]) return false;

	if (!streq(#found_text, #search_text)) {
		strcpy(#found_text, #search_text);
		find_all();
		draw_window();
	}

	for (i=0; i<found.count; i++) {
		if (signed found.get(i) - lines.get(_first) > 0) {
			while(signed lines.get(_first) - found.get(i-1) > 0) _first--;
			return _first;
		}
	}
	return false;
}
