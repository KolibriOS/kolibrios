
#define MIN_PB_BLOCK_W 19
#define LOAD_CPU 0x2460C8
#define PROGRESS_ACTIVE 0x62B7E4
#define PROGRESS_BG 0xFFFfff
#define PROGRESS_BG_TEXT 0x696969

:struct sensor {
	int x,y,w,h;
	void set_size();
	void draw_wrapper();
	void draw_progress();
};

:void sensor::set_size(dword _x, _y, _w, _h)
{
	x=_x+2; 
	y=_y;
	w=_w;
	h=_h;
	draw_wrapper();
}

:void sensor::draw_wrapper()
{
	DrawRectangle(x-1, y-1, w+1, h+1, sc.line);
	DrawRectangle3D(x-2, y-2, w+3, h+3, sc.work, sc.light);
}

:void sensor::draw_progress(dword progress_w)
{
	char textp[16];
	DrawBar(x, y,     w-progress_w, 1,   MixColors(PROGRESS_ACTIVE, PROGRESS_BG, 200));
	DrawBar(x, y+1,   w-progress_w, h-2, PROGRESS_ACTIVE);
	DrawBar(x, y+h-1, w-progress_w, 1,   MixColors(PROGRESS_ACTIVE, sc.line, 200));
	DrawBar(x+w-progress_w, y, progress_w, h, PROGRESS_BG);

	strcpy(#textp, itoa(w-progress_w*100/w));
	chrcat(#textp, '%');
	WriteText(-strlen(#textp)*8 + w / 2 + x, h/2-7+y, 0x90, 0x000000, #textp);
}