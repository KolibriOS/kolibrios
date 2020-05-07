#define MEMSIZE 4096*400

#include "../lib/window.h"
#include "../lib/array.h"

window win1=0;
void main()
{
	win1.background = 0xFFFFFF;
	win1.left = 200;
	win1.top = 200;
	win1.caption = "Stress test";
	win1.ondraw = #draw_window;
	win1.create();
}


void draw_window()
{
	dword init = 0;
	dword count = 0;
	dword position = 0;
	dword y = 15;
	init = malloc(0x1000);
	count = 200000;
	while (count)
	{
		position = indexArray(init, count);
		DSDWORD[position] = count*2;
		count--;
	}
	position = indexArray(init, 123);DSDWORD[position] = 0;
	position = indexArray(init, 777);DSDWORD[position] = 0;
	count = 200000;
	while (count)
	{
		position = indexArray(init, count);
		if (DSDWORD[position] != count*2)
		{
			WriteText(15, y, 0x81, 0xFF0000, itoa(count));
			y += 25;
		}
		count--;
	}
	
}
