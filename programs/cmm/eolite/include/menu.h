#ifdef LANG_RUS

char file_actions[]= 
"Открыть           |Enter
Открыть с помощью  |Ctrl+Ent
-
Копировать путь    |Ctrl+P
-
Копировать|Ctrl+C
Вырезать|Ctrl+X
Вставить|Ctrl+V
-
Переименовать      |F2
Удалить            |Del
Свойства           |F1";
char empty_folder_actions[]=
"Вставить      |Ctrl+V";
char burger_menu_items[] = 
"Новое окно|Ctrl+N
-
Открыть консоль|Ctrl+G
Поиск|Ctrl+F
-
Настройки|F10
О программе";

#elif LANG_EST
char file_actions[]= 
"Ava           |Enter
Ava ...        |Ctrl+Ent
-
Copy path      |Ctrl+P
-
Kopeeri        |Ctrl+C
Lїika          |Ctrl+X
Aseta          |Ctrl+V
-
Nimeta №mber   |F2
Kustuta        |Del
Properties     |F1";
char empty_folder_actions[]=
"Aseta         |Ctrl+V";
char burger_menu_items[] = 
"New window|Ctrl+N
-
Open console here|Ctrl+G
Search|Ctrl+F
-
Settings|F10
About";

#else
char file_actions[]= 
"Open         |Enter
Open with...  |Ctrl+Ent
-
Copy path     |Ctrl+P
-
Copy          |Ctrl+C
Cut           |Ctrl+X
Paste         |Ctrl+V
-
Rename        |F2
Delete        |Del
Properties    |F1";
char empty_folder_actions[]=
"Paste        |Ctrl+V";
char burger_menu_items[] = 
"New window|Ctrl+N
-
Open console here|Ctrl+G
Search|Ctrl+F
-
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
		case 3: EventCopyItemPath(); break;
		case 4: CopyFilesListToClipboard(COPY); break;
		case 5: CopyFilesListToClipboard(CUT); break;
		case 6: EventPaste(path); break;
		case 7: FnProcess(2); break;
		case 8: ShowPopinForm(POPIN_DELETE); break;
		case 9: FnProcess(1); break;
	}
	if (active_menu == MENU_BURGER) switch(_id) {
		case 1: EventOpenNewEolite(); break;
		case 2: EventOpenConsoleHere(); break;
		case 3: EventOpenSearch(); break;
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

