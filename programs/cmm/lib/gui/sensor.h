
#define MIN_PB_BLOCK_W 19
#define LOAD_CPU 0x2460C8
#define PROGRESS_ACTIVE 0x489FE4
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
	DrawRectangle(x-1, y-1, w+1, h+1, system.color.work_graph);
	DrawRectangle3D(x-2, y-2, w+3, h+3, system.color.work_dark, system.color.work_light);
}

:void sensor::draw_progress(dword progress_w, active_value, bg_value, mesure)
{
	if (progress_w < MIN_PB_BLOCK_W) progress_w = MIN_PB_BLOCK_W;
	if (progress_w > w-MIN_PB_BLOCK_W) progress_w = w-MIN_PB_BLOCK_W;

	DrawBar(x, y,     w-progress_w, 1,   MixColors(PROGRESS_ACTIVE, PROGRESS_BG, 200));
	DrawBar(x, y+1,   w-progress_w, h-2, PROGRESS_ACTIVE);
	DrawBar(x, y+h-1, w-progress_w, 1,   MixColors(PROGRESS_ACTIVE, system.color.work_graph, 200));

	sprintf(#param, "%i%s", active_value, mesure);
	WriteText(w-progress_w- calc(strlen(#param)*8) /2 + x, h/2-7+y, 0x90, PROGRESS_BG, #param);

	DrawBar(x+w-progress_w, y, progress_w, h, PROGRESS_BG);
	sprintf(#param, "%i%s", bg_value, mesure);
	WriteText(-progress_w - calc(strlen(#param)*8)/2 + w+x, h/2-7+y, 0x90, PROGRESS_BG_TEXT, #param);
}