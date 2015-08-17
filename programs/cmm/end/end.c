#ifndef AUTOBUILD
#include "lang.h--"
#endif

#define MEMSIZE 4096*4
#include "../lib/gui.h"
#include "../lib/file_system.h"

#ifdef LANG_RUS
#define TEXT_TITLE "Завершение работы"
#define TEXT_RDSAVE1 "Нажмите Ctrl+S для сохранения изменений"
#define TEXT_RDSAVE2 "сделанных в процессе работы в системе."
#define TEXT_REBOOT "Перезагрузка"
#define TEXT_OFF "Выключение"
#define TEXT_CANCEL "Отмена"
#else
#define TEXT_TITLE "Shutdown computer"
#define TEXT_RDSAVE1 "Press Ctrl+S to save all changes"
#define TEXT_RDSAVE2 "that were done during system work."
#define TEXT_REBOOT "Reboot"
#define TEXT_OFF "Power off"
#define TEXT_CANCEL "Close"
#endif

#define WIN_W 440
#define WIN_H 200
#define BOT_PANEL_H 70

void main()
{
	int key;
	int WIN_X = GetScreenWidth() - WIN_W / 2;
	int WIN_Y = GetScreenHeight() - WIN_H / 2;

	loop()
	{
	  switch(WaitEvent())
	  {
		 case evButton:
			key=GetButtonID();               
			if (key==1) ExitProcess();
			GOTO _BUTTON_MARK;
	  
		case evKey:
			key = GetKey();
			_BUTTON_MARK:
			if (key==ASCII_KEY_ENTER) ExitSystem(REBOOT);
			if (key==ASCII_KEY_END) ExitSystem(TURN_OFF);
			if (key==ASCII_KEY_ESC) ExitProcess();
			if (key==19) RunProgram("rdsave",0);
			break;
		 
		 case evReDraw:
			DefineAndDrawWindow(WIN_X, WIN_Y, WIN_W-1, WIN_H-1, 0x41, 0, 0, 0);
			DrawWideRectangle(0, 0, WIN_W, WIN_H, 2, 0xA3A7AA);
			DrawBar(2, 2, WIN_W-4, WIN_H-BOT_PANEL_H-2, 0x202020);
			DrawBar(2, WIN_H-BOT_PANEL_H-2, WIN_W-4, BOT_PANEL_H, 0x4B4B4B);
			WriteText(30, 27, 10110001b, 0xFFFfff, TEXT_TITLE);
			WriteText(30, 70, 10110000b, 0xFFFfff, TEXT_RDSAVE1);
			WriteText(30, 85, 10110000b, 0xFFFfff, TEXT_RDSAVE2);
			EndButton( 20, 0x4E91C5, ASCII_KEY_ESC, TEXT_CANCEL, "Esc");
			EndButton(160, 0x55C891, ASCII_KEY_ENTER, TEXT_REBOOT, "Enter");
			EndButton(300, 0xC75C54, ASCII_KEY_END, TEXT_OFF, "End");
			break;
	  }
   }
}

void EndButton(dword x, bgcol, id, but_text, hotkey_text)
{
	word buty=WIN_H-60;
	word butw=116;
	word buth=43;
	DrawWideRectangle(x-3, buty-3, butw+6, buth+6, 3, 0x202020);
	DefineButton(x, buty, butw-1, buth-1, id, bgcol);
	WriteTextB(-utf8_strlen(but_text)*8 + butw / 2 + x, buty+8, 10110000b, 0xFFFfff, but_text);
	WriteTextCenter(x, buty+26, butw, 0xFFFfff, hotkey_text);
}