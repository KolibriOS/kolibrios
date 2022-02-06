#define MEMSIZE 1024*50

#include "../lib/gui.h"
#include "../lib/copyf.h"

#include "../lib/obj/http.h"
#include "../lib/obj/network.h"

#include "../lib/patterns/restart_process.h"

bool install_complete = false;
_http http;
dword unimg_id;

#define WINW 460
#define WINH 330

//#define LANG_RUS 1

#ifdef LANG_RUS
#define T_WINDOW_TITLE "Онлайн обновление KolibriOS"
#define T_TITLE_H1 "Онлайн обновление"
#define T_INTRO "Данное приложение скачает последнюю версию KolibriOS и распакует ее на RAM-диск. При этом ядро не будет перезапущено, для этого необходимо сохранить образ и перезагрузится. Пожалуйста, закройте все открытые приложения перед началом обновления.
ВНИМАНИЕ: Все изменные файлы на RAM-диске будут перезаписаны!"; 
#define T_INSTALL "Обновить"
#define T_DOWNLOADING "Скачиваю свежий образ kolibri.img..."
#define T_UNPACKING "Распаковываю и копирую файлы..."
#define T_COMPLETE "Обновление успешно завершено."
#define T_EXIT "Выход"
#define IMG_URL "http://builds.kolibrios.org/rus/data/data/kolibri.img"
#define KS "Сохранить настройки"
#else
#define T_WINDOW_TITLE "KolibriOS Online Updater"
#define T_TITLE_H1 "Online Updater"
#define T_INTRO "This app will download the latest KolibriOS dirsto and update your RAM-disk with it. Kernel won't be restarted. 
Please close all opened apps before start.
Note that all changes on RAM-disk will be lost."; 
#define T_INSTALL "Update"
#define T_DOWNLOADING "Downloading the latest kolibri.img..."
#define T_UNPACKING "Unpacking and copying files..."
#define T_COMPLETE "Update complete successfully."
#define T_EXIT "Exit"
#define IMG_URL "http://builds.kolibrios.org/eng/data/data/kolibri.img"
#define KS "Keep settings folder"
#endif
char accept_language[]="en"; //not used, necessary for http.get()
void Operation_Draw_Progress(dword f) {} //not used, necessary for copyf()

checkbox keep_settings = { KS, true };
sensor progress = { 40, WINH-70, WINW-80, 20 };

void main()
{
	int btn;
	saved_state = FILE_REPLACE;
	load_dll(libHTTP, #http_lib_init,1);
	SetWindowLayerBehaviour(-1, ZPOS_ALWAYS_TOP);
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_STACK);
	loop() switch(@WaitEventTimeout(300))
	{
		case evButton: 
			btn = @GetButtonID();
			if (btn<=2) ExitProcess();
			if (btn==9) goto _INSTALL; 
			keep_settings.click(btn);
			break;

		case evKey:
			switch (@GetKeyScancode()) {
				case SCAN_CODE_ESC: ExitProcess();
				case SCAN_CODE_ENTER: 
					if (install_complete) ExitProcess();
					else {
						_INSTALL: 
						http.get(IMG_URL); 
						goto _DRAW_WINDOW;
						}
			}
			break;

		case evReDraw:
			_DRAW_WINDOW:
			draw_window();

		case evNetwork:
			if (http.transfer <= 0) break;
			http.receive();
			if (http.content_length) {
				progress.draw_progress(http.content_length - http.content_received 
					* progress.w / http.content_length);
			}
			if (!http.receive_result) {
				CreateFile(http.content_received, 
					http.content_pointer, "/tmp0/1/latest.img");
				http.stop();
				EventDownloadComplete();
			}
	}
}

void draw_window()
{
	sc.get();
	DefineAndDrawWindow(screen.w-WINW/2,screen.h-WINH/2,
		WINW+9,WINH+skin_h,0x34,sc.work,T_WINDOW_TITLE,0);
	WriteText(30, 20, 0x91, 0xEC008C, T_TITLE_H1);
	if (!install_complete) {
			if (GetProcessSlot(unimg_id)) {
				//UNPACKING
				draw_icon_32(WINW-32/2, 140, sc.work, 89);
				WriteTextCenter(0, 185, WINW, sc.work_text, T_UNPACKING);
			} else if (http.transfer<=0) {
				//INTRO
				DrawTextViewArea(30, 65, WINW-60, WINH-80, T_INTRO, -1, sc.work_text);
				DrawCaptButton(WINW-160, WINH-70, 110, 28, 9, 0x0092D8, 0xFFFfff, T_INSTALL);
				keep_settings.draw(30, WINH-65);
			} else {
				//DOWNLOADING
				draw_icon_32(WINW-32/2, 140, sc.work, 51);
				WriteTextCenter(0, 185, WINW, sc.work_text, T_DOWNLOADING);
				progress.draw_wrapper();
			}
	} else {
				//COMPLETE
				draw_icon_32(WINW-32/2, 140, sc.work, 49);
				WriteTextCenter(0, 185, WINW, sc.work_text, T_COMPLETE);
				DrawCaptButton(WINW-110/2, WINH-70, 110, 28, 2, 
					0x0092D8, 0xFFFfff, T_EXIT);
	}
}

dword GetFreeSpaceOfRamdisk()
{
	dword rdempty = malloc(1440*1024);
	CreateFile(0, 1440*1024, rdempty, "/sys/rdempty");
	free(rdempty);
	rdempty = get_file_size("/sys/rdempty");
	DeleteFile("/sys/rdempty");
	return rdempty;
}

signed CheckFreeSpace(dword _latest, _combined)
{
	dword cur_size, new_size, empty;
	DIR_SIZE dir_size;

	dir_size.get("/sys");
	cur_size = dir_size.sizelo;

	copyf("/sys", _combined);
	copyf(_latest, _combined);
	dir_size.get(_combined);
	new_size = dir_size.sizelo;

	empty = GetFreeSpaceOfRamdisk();

	return cur_size + empty - new_size / 1024;
}

void EventDownloadComplete()
{
	dword slot_n;
	signed space_delta;
	int i=0;

	char osupdate[32];
	char latest[40];
	char backup[40];
	char combined[40];
	char exract_param[64];
	char backup_settings[64];

	do  { miniprintf(#osupdate, "/tmp0/1/osupdate%d", i); i++;
	} while (dir_exists(#osupdate));
	CreateDir(#osupdate);

	miniprintf(#latest, "%s/latest", #osupdate);
	miniprintf(#backup, "%s/rdbackup", #osupdate);
	miniprintf(#combined, "%s/combined", #osupdate);
	miniprintf(#backup_settings, "%s/settings", #backup);
	miniprintf(#exract_param, "/tmp0/1/latest.img %s -e", #latest);

	unimg_id = RunProgram("/sys/unimg", #exract_param);
	draw_window();

	do {
		slot_n = GetProcessSlot(unimg_id);
		pause(10);
	} while (slot_n!=0);

	space_delta = CheckFreeSpace(#latest, #combined);
	if (space_delta<0) {
		miniprintf(#param, "'Not enought free space! You need %d KB more.'E", -space_delta);
		notify(#param);
	} else {
		copyf("/sys", #backup);
		copyf(#latest, "/sys");
		if (keep_settings.checked) copyf(#backup_settings, "/sys/settings");
		install_complete = true;		
	}
}



