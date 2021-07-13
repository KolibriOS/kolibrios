#ifdef LANG_RUS

char file_actions[]= 
"Открыть           |Enter
Открыть с помощью  |CrlEnt
-
Копировать         |Crl+C
Вырезать           |Crl+X
Вставить           |Crl+V
-
Переименовать      |F2
Удалить            |Del
Свойства           |F1";
char empty_folder_actions[]=
"Вставить      |Crl+V";
char burger_menu_items[] = 
"Новое окно|Ctrl+N
Открыть консоль|Ctrl+G
Обновить папку|Ctrl+R
Настройки|F10
О программе";

#elif LANG_EST
char file_actions[]= 
"Ava           |Enter
Ava ...        |CrlEnt
-
Kopeeri        |Crl+C
Lїika          |Crl+X
Aseta          |Crl+V
-
Nimeta №mber   |F2
Kustuta        |Del
Properties     |F1";
char empty_folder_actions[]=
"Aseta         |Crl+V";
char burger_menu_items[] = 
"New window|Ctrl+N
Open console here|Ctrl+G
Vфrskenda|Ctrl+R
Settings|F10
About";

#else
char file_actions[]= 
"Open         |Enter 
Open with...  |CrlEnt
-
Copy          |Crl+C
Cut           |Crl+X
Paste         |Crl+V
-
Rename        |F2
Delete        |Del
Properties    |F1";
char empty_folder_actions[]=
"Paste        |Crl+V";
char burger_menu_items[] = 
"New window|Ctrl+N
Open console here|Ctrl+G
Refresh folder|Ctrl+R
Settings|F10
About";
#endif


enum { MENU_FILE=1, MENU_NO_FILE, MENU_BURGER };

bool active_menu = false;

void EventMenuClick(dword _id)
{
	if (active_menu == MENU_NO_FILE) switch(_id) {
		case 1: EventPaste(path); break;
	}
	if (active_menu == MENU_FILE) switch(_id) {
		case 1: EventOpen(0); break;
		case 2: ShowOpenWithDialog(); break;
		case 3: CopyFilesListToClipboard(COPY); break;
		case 4: CopyFilesListToClipboard(CUT); break;
		case 5: EventPaste(path); break;
		case 6: FnProcess(2); break;
		case 7: ShowPopinForm(POPIN_DELETE); break;
		case 8: FnProcess(1); break;
	}
	if (active_menu == MENU_BURGER) switch(_id) {
		case 1: EventOpenNewEolite(); break;
		case 2: EventOpenConsoleHere(); break;
		case 3: EventRefreshDisksAndFolders(); break;
		case 4: FnProcess(10); break;
		case 5: EventShowAbout(); break;		
	}
	active_menu = NULL;
}

void EventShowListMenu()
{
	dword text;

	pause(3);

	if (!files.count) {
		text = #empty_folder_actions;
		active_menu = MENU_NO_FILE;
	} else {
		text = #file_actions;
		active_menu = MENU_FILE;
	}
	open_lmenu(mouse.x, mouse.y+3, MENU_TOP_LEFT, NULL, text);
}

void EventShowBurgerMenu()
{
	active_menu = MENU_BURGER;
	open_lmenu(Form.cwidth-6, 35, MENU_TOP_RIGHT, NULL, #burger_menu_items);
}

bool GetMenuClick()
{
	dword click_id;
	if (active_menu) && (click_id = get_menu_click()) {
		EventMenuClick(click_id);
		return false;
	}
	return true;
}

