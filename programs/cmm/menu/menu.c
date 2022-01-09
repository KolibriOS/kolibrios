#define MEMSIZE 4096*40

#include "../lib/gui.h"
#include "../lib/collection.h"
#include "../lib/list_box.h"
#include "../lib/fs.h"

#define ITEM_H 19
#define SEP_H 4

llist menu1;
collection names=0;
collection hotkeys=0;

int selected, win_x, win_y;

int max_name_len;
int max_hotkey_len;
int menu_w, menu_h;

void GetWindowPosition()
{
	int position;
	shared_mem = memopen(#shared_name, 16, SHM_OPEN + SHM_WRITE);
	selected = ESDWORD[shared_mem     ];
	win_x    = ESDWORD[shared_mem +  4];
	win_y    = ESDWORD[shared_mem +  8];
	position = ESDWORD[shared_mem + 12];
	if (position == MENU_TOP_RIGHT) win_x -= menu1.w;
	if (position == MENU_BOT_LEFT) win_y -= menu1.h;
	if (position == MENU_BOT_RIGHT) {
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

int GetSeparatorsCount()
{
	int i, count=0;
	for (i=0; i<names.count; i++) {
		if (streq(names.get(i), "-")) count++;
	}
	return count;
}

int MoveMouseToHandleSeparators(int _mouse_y)
{
	int i, item_y=menu1.y;
	int item_i=0;
	for (i=0; i<menu1.count; i++;)
	{
		if (streq(names.get(i), "-")) {
			item_y += SEP_H;
		} else {
			item_y += ITEM_H;
			item_i++;
		}
		if (_mouse_y >= item_y) && (_mouse_y < item_y + ITEM_H) {
			return item_i * ITEM_H + menu1.y;
		}
	}
	return _mouse_y;
}

void main()
{
	proc_info Form;

	if (!param) RunProgram("/sys/network/WebView", "http://board.kolibrios.org/viewtopic.php?f=24&t=4233#p74599");

	GetMenuItems(#param);
	GetMenuWidths();

	menu_w = max_name_len + max_hotkey_len + 23;
	menu_h = GetSeparatorsCount() * SEP_H 
		+ calc(names.count - GetSeparatorsCount() * ITEM_H);

	menu1.count = names.count;
	menu1.SetFont(6, 9, 0x80);
	menu1.SetSizes(2,2, menu_w, menu_h, ITEM_H);
	menu1.cur_y = -1;

	GetWindowPosition();

	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE);
	loop() switch(WaitEvent())
	{
		case evMouse:			
			GetProcessInfo(#Form, SelfInfo);
			if (!CheckActiveProcess(Form.ID)) exit();
			mouse.get();
			if (menu1.MouseOver(mouse.x, mouse.y)) {
				mouse.y = MoveMouseToHandleSeparators(mouse.y);
				if (menu1.ProcessMouse(mouse.x, mouse.y)) draw_list();
				if (mouse.lkm)&&(mouse.up) click();	
			} 
			break;

		case evKey:
			ProcessKeys();
			break;

		case evReDraw:
			DefineAndDrawWindow(win_x, win_y, menu1.w+4, menu1.h+4, 0x01, 0, 0, 0x01fffFFF);
			sc.get();
			Draw3DPopup(0,0,menu1.w+2,menu1.h+2);
			draw_list();
	}
}

void CorrectLastItem()
{
	if (menu1.cur_y > menu1.count - GetSeparatorsCount() - 1) {
		menu1.cur_y = menu1.count - GetSeparatorsCount() - 1;
	}
}

inline ProcessKeys()
{
	@GetKeyScancode();
	switch(AL) 
	{
		case SCAN_CODE_ESC:
			exit();

		case SCAN_CODE_ENTER:
			click();

		case SCAN_CODE_DOWN:
			if (!menu1.KeyDown()) 
			|| (menu1.count - menu1.cur_y - GetSeparatorsCount() -1 < 0) menu1.KeyHome();
			draw_list();
			break;

		case SCAN_CODE_UP:
			if (!menu1.KeyUp()) {
				menu1.KeyEnd();
				CorrectLastItem();
			}
			draw_list();
			break;

		case SCAN_CODE_END:
			menu1.KeyEnd();
			CorrectLastItem();
			draw_list();
			break;			

		default:
			if (menu1.ProcessKey(AL)) draw_list();
	}
}

void draw_list()
{
	int i, item_y=menu1.y, item_i=0;
	dword name_color;
	dword hotkey_color;

	static dword inactive_background_color;
	static dword active_background_color;
	static dword active_top_border_color;
	static dword inactive_text_shadow_color;
	static bool skin_dark;

	static bool colors_set;
	if (!colors_set) {
		colors_set = true;
		inactive_background_color = MixColors(sc.work, 0xFFFfff,230);
		active_background_color = MixColors(sc.button, sc.work,230);
		active_top_border_color = MixColors(sc.line, sc.button,240);
		inactive_text_shadow_color = MixColors(sc.work,0xFFFfff,120);
		skin_dark = skin_is_dark();
	}

	for (i=0; i<menu1.count; i++;)
	{
		if (streq(names.get(i), "-")) {
			DrawBar(menu1.x, item_y+0, menu1.w, 1, inactive_background_color);
			DrawBar(menu1.x-1, item_y+1, menu1.w+1, 1, sc.dark);
			DrawBar(menu1.x, item_y+2, menu1.w, 1, sc.light);
			DrawBar(menu1.x, item_y+3, menu1.w, 1, inactive_background_color);
			//DrawBar(menu1.x, item_y+0, menu1.w, 4, inactive_background_color);
			//DrawBar(13, item_y+1, menu1.w-24, 1, sc.dark);
			//DrawBar(13, item_y+2, menu1.w-24, 1, sc.light);
			item_y += SEP_H;
		} else {
			if (item_i==menu1.cur_y) {
				hotkey_color = name_color = sc.button_text;
				DrawBar(menu1.x, item_y+1,        menu1.w, ITEM_H-2, active_background_color);
				DrawBar(menu1.x, item_y,          menu1.w, 1, active_top_border_color);
				DrawBar(menu1.x, item_y+ITEM_H-1, menu1.w, 1, sc.light);
			} else {
				name_color = sc.work_text;
				hotkey_color = sc.line;
				DrawBar(menu1.x, item_y, menu1.w, ITEM_H, inactive_background_color);
				if (!skin_dark) WriteText(13+1, item_y + menu1.text_y +1, 0x80, 
					inactive_text_shadow_color, names.get(i));
			}
			WriteText(-strlen(hotkeys.get(i))*6 + 13 + max_name_len + max_hotkey_len, 
				item_y + menu1.text_y, 0x80, hotkey_color, hotkeys.get(i));
			WriteText(13, item_y + menu1.text_y, 0x80, name_color, names.get(i));
			item_y += ITEM_H;
			item_i++;		
		}
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
