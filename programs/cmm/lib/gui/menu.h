#ifndef INCLUDE_MENU_H
#define INCLUDE_MENU_H

:dword menu_process_id;

#define MENU_ALIGN_TOP_LEFT  0
#define MENU_ALIGN_TOP_RIGHT 1
#define MENU_ALIGN_BOT_LEFT  2
#define MENU_ALIGN_BOT_RIGHT 3

:dword shared_mem = NULL;
:char shared_name[] = "LMENU";
:void open_lmenu(dword _x, _y, _position, _selected, _text1)
{
	if (!shared_mem) {
		shared_mem = memopen(#shared_name, 16, SHM_CREATE + SHM_WRITE);
		if (EDX) shared_mem = memopen(#shared_name, 16, SHM_WRITE);
	}
	ESDWORD[shared_mem     ] = _selected;
	ESDWORD[shared_mem +  4] = _x;
	ESDWORD[shared_mem +  8] = _y;
	ESDWORD[shared_mem + 12] = _position;
	menu_process_id = RunProgram("/sys/develop/menu", _text1);
}

:dword get_menu_click()
{
	if (menu_process_id) && (GetProcessSlot(menu_process_id)) {
		return NULL;
	} else {
		menu_process_id = NULL;
		EAX = ESDWORD[shared_mem];
		ESDWORD[shared_mem] = 0;
		return EAX;		
	}
}

#endif