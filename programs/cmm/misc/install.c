#define MEMSIZE 1024*60

#include "../lib/gui.h"
#include "../lib/copyf.h"

#include "../lib/obj/libini.h"

#include "../lib/patterns/restart_process.h"

char logo[] = "
ЫЫЫЫ   ЫЫЫЫ ЫЫЫЫЫЫЫЫЫЫ ЫЫЫЫ   ЫЫЫЫ ЫЫЫЫЫЫЫЫЫЫЫ
 ЫЫЫЫ   ЫЫ   ЫЫЫ         ЫЫЫ ЫЫЫ   ЫЫ  ЫЫЫ  ЫЫ
 ЫЫЫЫЫ  ЫЫ   ЫЫЫ          ЫЫЫЫЫ    ЫЫ  ЫЫЫ  ЫЫ
 ЫЫ ЫЫЫ ЫЫ   ЫЫЫЫЫЫЫ       ЫЫЫ         ЫЫЫ    
 ЫЫ  ЫЫЫЫЫ   ЫЫЫ          ЫЫЫЫЫ        ЫЫЫ    
 ЫЫ   ЫЫЫЫ   ЫЫЫ         ЫЫЫ ЫЫЫ       ЫЫЫ    
ЫЫЫЫ   ЫЫЫ  ЫЫЫЫЫЫЫЫЫЫ ЫЫЫЫ   ЫЫЫЫ    ЫЫЫЫЫ   
";

#ifdef LANG_RUS
#define T_INTRO "Џ®Їа®Ўг©вҐ ­®ў®Ґ ўЁ§г «м­®Ґ ®д®а¬«Ґ­ЁҐ Љ®«ЁЎаЁ, Є®в®а®Ґ а ­миҐ Ўл«® ¤®бвгЇ­® в®«мЄ® ў KolibriNext."; 
#define T_INSTALL "“бв ­®ўЁвм"
#define T_COMPLETE "“бв ­®ўЄ  § ўҐаиҐ­ "
#define T_EXIT "‚ле®¤"
#else
#define T_INTRO "Try a new visual design of KolibriOS, which previously was available only in KolibriNext."; 
#define T_INSTALL "Install"
#define T_COMPLETE "Install complete"
#define T_EXIT "Exit"
#endif

#define B_INSTALL 10
#define B_EXIT 11

bool install_complete = false;

void main()
{
	word btn;
	saved_state = FILE_REPLACE;
	load_dll(libini, #lib_init,1);
	loop() switch(WaitEvent())
	{
		case evButton:
			btn = GetButtonID();               
			if (btn == 1) || (B_EXIT == btn) ExitProcess();
			if (B_INSTALL == btn) EventInstall();
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
	}
}

#define WINW 400
#define WINH 300
void draw_window()
{
	sc.get();
	DefineAndDrawWindow(screen.w-WINW/2,screen.h-WINH/2,
		WINW+9,WINH+skin_h,0x34,sc.work,"KolibriN10",0);
	DrawLogo();
	if (install_complete) DrawInstallComplete(); else DrawIntro();
}

void DrawIntro()
{
	DrawTextViewArea(30, 140, WINW-60, WINH-80, 
		T_INTRO, -1, sc.work_text);
	DrawCaptButton(WINW-110/2, WINH-70, 110, 28, B_INSTALL, 
		0x0092D8, 0xFFFfff, T_INSTALL);
}

void DrawInstallComplete()
{
	draw_icon_32(WINW-32/2, 140, sc.work, 49);
	WriteTextCenter(0,185, WINW, sc.work_text, T_COMPLETE);
	DrawCaptButton(WINW-110/2, WINH-70, 110, 28, B_EXIT, 
		0x0092D8, 0xFFFfff, T_EXIT);
}

void DrawLogo()
{
	#define LX -46*6+WINW/2
	#define LY 25
	WriteTextLines(LX-2, LY, 0x80, 0xF497C0, #logo, 9);
	WriteTextLines(LX+3, LY, 0x80, 0x7ED1E3, #logo, 9);

	pause(1);

	WriteTextLines(LX+1, LY, 0x80, 0xEC008C, #logo, 9);
	WriteTextLines(LX,   LY, 0x80, 0xEC008C, #logo, 9);
}

void EventInstall()
{
	ini_set_int stdcall ("/sys/settings/taskbar.ini", "Flags", "Attachment", 0);
	writing_error_channel = WRITE_ERROR_NOTIFY;
	copy_state = FILE_REPLACE;
	//if (copyf("/kolibrios/KolibriNext/settings", "/sys/settings")) return;
	CopyFile("/kolibrios/KolibriNext/settings/app_plus.ini", "/sys/settings/app_plus.ini");
	CopyFile("/kolibrios/KolibriNext/settings/docky.ini", "/sys/settings/docky.ini");
	CopyFile("/kolibrios/KolibriNext/settings/icon.ini", "/sys/settings/icon.ini");

	RestartProcessByName("/sys/@icon", MULTIPLE);
	RestartProcessByName("/sys/@taskbar", SINGLE);
	RestartProcessByName("/sys/@docky", SINGLE);

	RunProgram("/sys/media/kiv", "\\S__/kolibrios/res/Wallpapers/Free yourself.jpg");

	install_complete = true;

	pause(150);
	ActivateWindow_Self();
}

void Operation_Draw_Progress(dword filename) { }