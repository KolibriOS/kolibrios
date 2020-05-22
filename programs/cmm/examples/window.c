#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/window.h"

window win1=0;
void main()
{
	io.dir.load(0,DIR_ONLYREAL);
	win1.background = 0xFFFFFF;
	win1.left = 200;
	win1.ondraw = #draw_window;
	win1.create();
}
void draw_window()
{
	int i;
	for (i=0; i<io.dir.count; i++)WriteText(5,i*8+3,0x80,0xFF00FF,io.dir.position(i));
	DrawCaptButton(100, 10, 100, 22, 22, 0xCCCccc, 0x000000, "Button");
	WriteText(100,50,0x80,0,"Textline small");
	WriteText(100,70,0x90,0,"Textline big");
	DrawBar(100, 110, 100, 100, 0x66AF86);
	draw_ascii();
}

void draw_ascii()
{
	char s[2];
	int i, x, y;
	s[1] = '\0';
	for (i=0; i<256; i++) {
		y = i / 20;
		x = i % 20;
		s[0] = i;
		WriteText(x*33, y*33, 0x91, 0, #s);
	}
}