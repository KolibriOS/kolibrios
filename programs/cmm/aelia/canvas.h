
struct _canvas
{
	void write_text();
	void draw_hor_line();
};

void _canvas::write_text(int _x, _y; dword _text_col, _text_off) 
{
	char error_message[128];

	if (_x > list.w) {
		sprintf(#error_message, "ERROR: canvas.x overflow: H %d X %d", kfont.size.height, _x);
		debugln(#error_message);
	}
	if (_y+kfont.size.pt > kfont.size.height) {
		sprintf(#error_message, "ERROR: canvas.y overflow: H %d Y %d", kfont.size.height, _y);
		debugln(#error_message);
		return;
	}
	kfont.WriteIntoBuffer(_x, _y, list.w, kfont.size.height, 0xFFFFFF, _text_col, kfont.size.pt, _text_off);
	if (_y/list.item_h-list.first==list.visible) DrawPage();
}


void _canvas::draw_hor_line(dword _x, _y, _w, _color)
{
	int i;
	for (i = _y*list.w+_x*KFONT_BPP+kfont.raw ; i<_y*list.w+_x+_w*KFONT_BPP+kfont.raw ; i+=KFONT_BPP) 
	{
		ESDWORD[i] = _color;
	}
}

