char char_width[255];

void get_label_symbols_size()
{
	int i;
	label.changeSIZE();
	for (i=0; i<256; i++) char_width[i] = label.symbol_size(i);
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
		sprintf(#error_message, "\nWriteTextIntoBuf _x overflow: H %d X %d \n", label.size.height, _x);
		notify(#error_message);
	}
	if (_y+label.size.pt > label.size.height) {
		sprintf(#error_message, "\nWriteTextIntoBuf _y overflow: H %d Y %d \n", label.size.height, _y);
		notify(#error_message);
		return;
	}
	label.write_buf(_x, _y, list.w, label.size.height, 0xFFFFFF, _text_col, label.size.pt, _text_off);
	if (_y/list.item_h-list.first==list.visible) DrawPage();
}


void label_draw_bar(dword _x, _y, _w, _color)
{
	int i;
	for (i = _y*list.w+_x*3+label.raw ; i<_y*list.w+_x+_w*3+label.raw ; i+=3)   ESDWORD[i] = _color;
}