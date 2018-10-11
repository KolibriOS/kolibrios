#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/list_box.h"
#include "../lib/gui.h"

struct _object
{
	int x,y,w,h,id;
};

_object butv = { 20, 20, 100, 30, 10};
_object buta = {150, 20, 100, 30, 20};

char vegetables[] = 
"Onion
Melon
Tomato
Squash
Salad";

char animals[] =
"Cat
Dog
Pig
Cow
Goat
Rabbit";

byte category;


void main()
{
	proc_info Form;
	int id;

	loop() switch(WaitEvent())
	{
	 case evButton:
		id=GetButtonID();               
		if (id==1) ExitProcess();
		if (id==butv.id) {
			menu.selected = category+1;
			menu.show(Form.left+5 + butv.x, Form.top+skin_height + butv.y + butv.h, 140, #vegetables, butv.id);
		}
		if (id==buta.id) {
			menu.selected = 0;
			menu.show(Form.left+5 + buta.x, Form.top+skin_height + buta.y + buta.h, 140,    #animals, buta.id);
		}
		break;

	case evKey:
		GetKeys();
		break;
	 
	 case evReDraw:
		if (menu.cur_y) {
			if (menu.cur_y > butv.id) && (menu.cur_y < buta.id) category = menu.cur_y - butv.id;
		}
		DefineAndDrawWindow(215,100,350,300,0x34,0xFFFFFF,"Window header",0);
		GetProcessInfo(#Form, SelfInfo);
		WriteText(10,110,0x80,0,#param);
		DrawCaptButton(butv.x, butv.y, butv.w, butv.h, butv.id, 0xCCCccc, 0x000000, "Vegetables");
		DrawCaptButton(buta.x, buta.y, buta.w, buta.h, buta.id, 0xCCCccc, 0x000000, "Aminal");
		break;
	}
}
