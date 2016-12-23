char char_width[255];

void get_label_symbols_size()
{
	int i;
	kfont.changeSIZE();
	for (i=0; i<256; i++) char_width[i] = kfont.symbol_size(i);
}

int get_label_len(dword _text) 
{
	int len=0;
	byte ch;
	loop () {
		ch = ESBYTE[_text];
		if (!ch) return len;
		len += char_width[ch];
		_text++;
	}
}

void WriteTextIntoBuf(int _x, _y; dword _text_col, _text_off) 
{
	char error_message[128];
	if (_x > list.w) {
		sprintf(#error_message, "'WriteTextIntoBuf _x overflow: H %d X %d' -A", kfont.size.height, _x);
		notify(#error_message);
	}
	if (_y+kfont.size.pt > kfont.size.height) {
		sprintf(#error_message, "'WriteTextIntoBuf _y overflow: H %d Y %d' -A", kfont.size.height, _y);
		notify(#error_message);
		return;
	}
	kfont.WriteIntoBuffer(_x, _y, list.w, kfont.size.height, 0xFFFFFF, _text_col, kfont.size.pt, _text_off);
	if (_y/list.item_h-list.first==list.visible) DrawPage();
}


void label_draw_bar(dword _x, _y, _w, _color)
{
	int i;
	for (i = _y*list.w+_x*KFONT_BPP+kfont.raw ; i<_y*list.w+_x+_w*KFONT_BPP+kfont.raw ; i+=KFONT_BPP) 
	{
		ESDWORD[i] = _color;
	}
}