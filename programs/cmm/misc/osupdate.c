#define MEMSIZE 1024*460

#include "../lib/io.h"
#include "../lib/gui.h"
#include "../lib/copyf.h"

#include "../lib/obj/libini.h"
#include "../lib/obj/libio.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/http.h"
#include "../lib/obj/network.h"

#include "../lib/patterns/restart_process.h"
#include "../lib/patterns/http_downloader.h"

#ifndef AUTOBUILD
#include "lang.h--"
#endif

_http http;
proc_info Form;
bool install_complete = false;
sensor progress;

#define WINW 460

//#define LANG_RUS 1

#ifdef LANG_RUS
#define WINH 345
#define T_WINDOW_TITLE "Онлайн обновление KolibriOS"
#define T_TITLE_H1 "ОНЛАЙН ОБНОВЛЕНИЕ"
#define T_INTRO "Данное приложение скачает последнюю версию KolibriOS и распакует ее на RAM-диск. При этом ядро не будет перезапущено, для этого необходимо сохранить образ и перезагрузится. Пожалуйста, закройте все открытые приложения перед началом обновления.

ВНИМАНИЕ: Все изменные файлы на RAM-диске будут перезаписаны!"; 
#define T_INSTALL "Обновить"
#define T_COMPLETE "Обновление завершено"
#define T_EXIT "Выход"
#define IMG_URL "http://builds.kolibrios.org/rus/data/data/kolibri.img"
#else
#define WINH 305
#define T_WINDOW_TITLE "KolibriOS Online Updater"
#define T_TITLE_H1 "ONLINE UPDATE"
#define T_INTRO "This app will download the latest KolibriOS dirsto and update your RAM-disk with it. Kernel won't be restarted. 
Please close all opened apps before start.

ALERT: All chages on RAM-disk will be lost!"; 
#define T_INSTALL "Update"
#define T_COMPLETE "Update complete"
#define T_EXIT "Exit"
#define IMG_URL "http://builds.kolibrios.org/eng/data/data/kolibri.img"
#endif
char accept_language[]="en"; //not used, necessary for http.get()

void main()
{
	//load_dll(libini, #lib_init,1);
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libHTTP,   #http_lib_init,1);
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_STACK);
	loop() switch(WaitEventTimeout(300) & 0xFF)
	{
		case evButton:
			if (GetButtonID() == 1) ExitProcess(); else EventInstall();
			break;
	  
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) ExitProcess();
			if (key_scancode == SCAN_CODE_ENTER) {
				if (install_complete) ExitProcess();
				else EventInstall();
			}
			break;
		 
		case evReDraw:
			draw_window();
			break;

		case evNetwork:
			if (http.transfer <= 0) break;
			http.receive();
			if (http.content_length) {
				progress.draw_progress(http.content_length - http.content_received * progress.w / http.content_length);
			}
			if (http.receive_result == 0) EventDownloadComplete();
	}
}

void draw_window()
{
	sc.get();
	DefineAndDrawWindow(screen.width-WINW/2,screen.height-WINH/2,
		WINW+9,WINH+skin_height,0x34,sc.work,T_WINDOW_TITLE,0);
	GetProcessInfo(#Form, -1);
	WriteText(30, 20, 0x81, 0xEC008C, T_TITLE_H1);
	if (install_complete) {
		DrawInstallComplete(); 
	} else {
		DrawIntro();
		progress.set_size(30, WINH-130, WINW-60, 20);
	}
}

void DrawIntro()
{
	DrawTextViewArea(30, 50, WINW-60, WINH-80, 
		T_INTRO, -1, sc.work_text);
	DrawCaptButton(WINW-110/2, WINH-70, 110, 28, 9999, 
		0x0092D8, 0xFFFfff, T_INSTALL);
}

void DrawInstallComplete()
{
	DrawIcon32(WINW-32/2, 140, sc.work, 49);
	WriteTextCenter(0,185, WINW, sc.work_text, T_COMPLETE);
	DrawCaptButton(WINW-110/2, WINH-70, 110, 28, 1, 
		0x0092D8, 0xFFFfff, T_EXIT);
}

void EventInstall()
{
	http.get(IMG_URL);
}

void EventDownloadComplete()
{
	dword unimg_id, slot_n;

	CreateFile(http.content_received, http.content_pointer, "/tmp0/1/latest.img");
	http.free();

	unimg_id = RunProgram("/sys/unimg", "/tmp0/1/latest.img /tmp0/1/latest_img -e");
	do {
		slot_n = GetProcessSlot(unimg_id);
		pause(10);
	} while (slot_n!=0);

	copyf("/rd/1/settings", "/tmp0/1/settings_backup");
	copyf("/tmp0/1/latest_img", "/rd/1");
	copyf("/tmp0/1/settings_backup", "/rd/1/settings");

	RestartAllProcess();
	install_complete = true;
	draw_window();
}

void RestartAllProcess() {
	int i;
	proc_info Process;
	for (i=0; i<MAX_PROCESS_COUNT; i++;)
	{
		GetProcessInfo(#Process, i);
		if (Process.name) 
		&& (!streq(#Process.name, "OS"))
		&& (Process.ID != Form.ID)
			KillProcess(Process.ID);
	}
	RunProgram("/sys/launcher", NULL);
}

void Operation_Draw_Progress(dword filename) { debug("copying: "); debugln(filename); }