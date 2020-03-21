#define MEMSIZE 4096*200

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

Array a = {0};
Dictionary b = {0};
void draw_window()
{
	dword size = 10000;
	dword i = 0;
	dword y = 10;
	i = size;
	/* ints */
	a.init(0);
	while(i){
		a.set(i,i);
		i--;
	}
	a.set(120,222);
	a.set(9990,345);
	i = size;
	while(i){
		if (a.get(i) != i) 
		{
			WriteText(15, y, 0x81, 0xFF0000, itoa(i));
			WriteText(95, y, 0x81, 0xFF0000, itoa(a.get(i)));
			y += 25;
		}
		i--;
	}
	/* strs */
	b.init(0);
	i = size;
	while(i){
		b.set(itoa(i),i);
		i--;
	}
	b.set("123","Okey");
	i = size;
	while(i){
		if (b.get(itoa(i)) != i) 
		{
			WriteText(15, y, 0x81, 0xFF0000, itoa(i));
			WriteText(95, y, 0x81, 0xFF0000, b.get(itoa(i)));
			y += 25;
		}
		i--;
	}
}
