#ifndef INCLUDE_MENU_H
#define INCLUDE_MENU_H

#ifndef INCLUDE_LIST_BOX
#include "../lib/list_box.h"
#endif

:dword menu_process_id;

:struct _menu : llist
{
	dword appear_x, appear_y, text, identifier, selected;	
	void show();
	char stak[4096];
} menu;

:void _menu::show(dword _appear_x, _appear_y, _menu_width, _menu_text, _identifier)
{
	#define ITEM_H 21
	appear_x = _appear_x;
	appear_y = _appear_y;
	text = _menu_text;
	identifier = _identifier;

	cur_y = -1;
	ClearList();
	count = chrnum(text, '\n')+1;
	SetSizes(2,2,_menu_width,count*ITEM_H,ITEM_H);

	menu_process_id = CreateThread(#_menu_thread,#stak+4092);
}

:void _menu_thread()
{
	proc_info MenuForm;
	SetEventMask(100111b);
	loop() switch(WaitEvent())
	{
		case evMouse:
			GetProcessInfo(#MenuForm, SelfInfo);
			if (!CheckActiveProcess(MenuForm.ID)) _menu_no_item_click();
			mouse.get();
			if (menu.ProcessMouse(mouse.x, mouse.y)) _menu_draw_list();
			if (mouse.lkm)&&(mouse.up) _menu_item_click();
			break;		
		case evKey:
			GetKeys();
			if (key_scancode==SCAN_CODE_ESC) _menu_no_item_click();
			if (key_scancode==SCAN_CODE_ENTER) _menu_item_click();
			if (menu.ProcessKey(key_scancode)) _menu_draw_list();
			break;
		case evReDraw:
			DefineAndDrawWindow(menu.appear_x,menu.appear_y,menu.w+2,menu.h+4,0x01, 0, 0, 0x01fffFFF);
			DrawPopup(0,0,menu.w,menu.h+3,0, 0xE4DFE1,0x9098B0);
			_menu_draw_list();				
	}
}

:void _menu_draw_list()
{
	int N, bgcol;
	for (N=0; N<menu.count; N++;)
	{
		if (N==menu.cur_y) bgcol=0xFFFfff; else bgcol=0xE4DFE1;
		DrawBar(menu.x, N*menu.item_h+menu.y, menu.w-3, menu.item_h, bgcol);
	}
	WriteTextLines(13, menu.item_h-8/2+menu.y, 0x80, 0, menu.text, menu.item_h);
	if (menu.selected) WriteText(5, menu.selected-1*menu.item_h+8, 0x80, 0xEE0000, "\x10");
}

:void _menu_item_click()
{
	menu.cur_y = menu.identifier + menu.cur_y;
	KillProcess(menu_process_id);
}

:void _menu_no_item_click()
{
	menu.cur_y = 0;
	KillProcess(menu_process_id);
}

#endif