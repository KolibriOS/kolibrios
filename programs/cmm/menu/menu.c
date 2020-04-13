#define MEMSIZE 4096*40

#include "../lib/gui.h"
#include "../lib/io.h"
#include "../lib/collection.h"
#include "../lib/list_box.h"
#include "../lib/fs.h"

#define ITEM_H 19

llist menu1;
collection names;
collection hotkeys;

int selected, win_x, win_y;

int max_name_len;
int max_hotkey_len;

void GetWindowPosition()
{
	int position;
	shared_mem = memopen(#shared_name, 16, SHM_OPEN + SHM_WRITE);
	selected = ESDWORD[shared_mem     ];
	win_x    = ESDWORD[shared_mem +  4];
	win_y    = ESDWORD[shared_mem +  8];
	position = ESDWORD[shared_mem + 12];
	if (position == MENU_ALIGN_TOP_RIGHT) win_x -= menu1.w;
	if (position == MENU_ALIGN_BOT_LEFT) win_y -= menu1.h;
	if (position == MENU_ALIGN_BOT_RIGHT) {
		win_x -= menu1.w;
		win_y -= menu1.h;
	}
}

void GetMenuWidths()
{
	int i;
	for (i=0; i<names.count; i++) {
		max_name_len = math.max(max_name_len, strlen(names.get(i)));
	}
	for (i=0; i<hotkeys.count; i++) {
		max_hotkey_len = math.max(max_hotkey_len, strlen(hotkeys.get(i)));
	}
	max_name_len = max_name_len * 6;
	max_hotkey_len *= 6;
	if (max_hotkey_len) max_name_len += 12;
}

void GetMenuItems(dword current_name)
{
	dword next_name = strchr(current_name, '\n');
	dword hotkey = strchr(current_name, '|');

	ESBYTE[next_name] = '\0';

	if (hotkey) && (hotkey < next_name) {
		ESBYTE[hotkey] = '\0';
	} else {
		if (hotkey) && (!next_name) {
			ESBYTE[hotkey] = '\0';
		} else {
			hotkey = " ";
		}
	}

	hotkeys.add(hotkey+1);
	names.add(current_name);

	if (next_name) GetMenuItems(next_name+2);
}

void main()
{
	proc_info Form;

	if (!param) die("'Menu component is for developers only' -I");

	GetMenuItems(#param);
	GetMenuWidths();

	menu1.count = names.count;
	menu1.SetFont(6, 9, 0x80);
	menu1.SetSizes(2,2, max_name_len + max_hotkey_len + 23, menu1.count*ITEM_H, ITEM_H);
	menu1.cur_y = -1;

	GetWindowPosition();

	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE);
	loop() switch(WaitEvent())
	{
		case evMouse:			
			GetProcessInfo(#Form, SelfInfo);
			if (!CheckActiveProcess(Form.ID)) exit();
			mouse.get();
			if (menu1.ProcessMouse(mouse.x, mouse.y)) draw_list();
			if (mouse.lkm)&&(mouse.up) click();
			break;

		case evKey:
			GetKeys();
			ProcessKeys();
			break;

		case evReDraw:
			DefineAndDrawWindow(win_x, win_y, menu1.w+4, menu1.h+4, 0x01, 0, 0, 0x01fffFFF);
			system.color.get();
			Draw3DPopup(0,0,menu1.w+2,menu1.h+2);
			draw_list();
	}
}

void ProcessKeys()
{
	switch(key_scancode) 
	{
		case SCAN_CODE_ESC:
			exit();

		case SCAN_CODE_ENTER:
			click();

		case SCAN_CODE_DOWN:
			if (!menu1.KeyDown()) menu1.KeyHome();
			draw_list();
			break;

		case SCAN_CODE_UP:
			if (!menu1.KeyUp()) menu1.KeyEnd();
			draw_list();
			break;

		default:
			if (menu1.ProcessKey(key_scancode)) draw_list();
	}
}

void draw_list()
{
	int i, item_y;

	dword active_background_color = MixColors(system.color.work_button, system.color.work,230);
	dword active_top_border_color = MixColors(system.color.work_graph, system.color.work_button,240);
	dword inactive_text_shadow_color = MixColors(system.color.work,0xFFFfff,150);
	dword text_color;
	bool skin_dark = skin_is_dark();

	for (i=0; i<menu1.count; i++;)
	{
		item_y = i*ITEM_H+menu1.y;
		if (i==menu1.cur_y) {
			text_color = system.color.work_button_text;
			DrawBar(menu1.x, item_y+1,        menu1.w, ITEM_H-2, active_background_color);
			DrawBar(menu1.x, item_y,          menu1.w, 1, active_top_border_color);
			DrawBar(menu1.x, item_y+ITEM_H-1, menu1.w, 1, system.color.work_light);
			WriteText(13 + max_name_len, item_y + menu1.text_y, 0x80, text_color, hotkeys.get(i));
		} else {
			text_color = system.color.work_text;
			DrawBar(menu1.x, item_y, menu1.w, ITEM_H, system.color.work);
			if (!skin_dark) WriteText(13+1, item_y + menu1.text_y +1, 0x80, inactive_text_shadow_color, names.get(i));
			WriteText(13 + max_name_len, item_y + menu1.text_y, 0x80, system.color.work_graph, hotkeys.get(i));
		}
		WriteText(13, item_y + menu1.text_y, 0x80, text_color, names.get(i));
	}
	if (selected) WriteText(5, selected-1*ITEM_H + menu1.y + menu1.text_y, 0x80, 0xEE0000, "\x10");
}

void click()
{
	ESDWORD[shared_mem] = menu1.cur_y + 1;
	ExitProcess();
}

void exit()
{
	ESDWORD[shared_mem] = 0;
	ExitProcess();
}
