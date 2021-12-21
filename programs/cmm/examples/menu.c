#define MEMSIZE 1024*40

#include "../lib/list_box.h"
#include "../lib/gui.h"
#include "../lib/fs.h"

struct _object
{
	int x,y,w,h,id;
};

_object butv = { 20, 20, 100, 20, 10};
_object buta = {150, 20, 100, 20, 20};

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


proc_info Form;

void main()
{
	dword menu_id=0, click_id;
	byte current_animal=1, current_veg=3;
	int id;

	loop() switch(WaitEvent())
	{
	 case evButton:
		id=GetButtonID();               
		if (id==1) ExitProcess();
		if (id==butv.id) {
			menu_id = butv.id;
			open_lmenu(butv.x, butv.y + butv.h, MENU_TOP_LEFT, 
				current_veg, #vegetables);
		}
		if (id==buta.id) {
			menu_id = buta.id;
			open_lmenu(buta.x + buta.w, buta.y + buta.h, 
				MENU_TOP_RIGHT, current_animal, #animals);
		}
		break;

	case evKey:
		GetKeys();
		break;
	 
	 case evReDraw:
		if (click_id = get_menu_click()) {
			if (menu_id == butv.id) current_veg = click_id;
			if (menu_id == buta.id) current_animal = click_id;
			menu_id = 0;
		}
		DefineAndDrawWindow(215,100,350,300,0x34,0xFFFFFF,"Window header",0);
		GetProcessInfo(#Form, SelfInfo);
		WriteText(10,110,0x80,0,#param);
		DrawCaptButton(butv.x, butv.y, butv.w, butv.h, butv.id, 0xCCCccc, 0x000000, "Vegetables");
		DrawCaptButton(buta.x, buta.y, buta.w, buta.h, buta.id, 0xCCCccc, 0x000000, "Aminal");
		break;
	}
}
