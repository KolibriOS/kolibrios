#ifndef AUTOBUILD
#include "lang.h--"
#endif

#define MEMSIZE 397113
#include "..\lib\strings.h" 
#include "..\lib\mem.h" 
#include "..\lib\file_system.h"
#include "..\lib\gui.h"
#include "..\lib\obj\truetype.h"
#include "..\lib\obj\proc_lib.h"

#include "simple_open_dialog.h"
char default_dir[] = "/rd/1";
od_filter filter2 = {"TTF",0};

dword font_data;
stbtt_fontinfo font_info;
dword font_mem;

proc_info Form;
char test_text[] = "The quick brown fox jumps over the lazy dog";
char win_title[4096] = "TTF Viewer v0.1 - ";

#ifdef LANG_RUS
	?define T_INTRO "Это простая программа для просмотра шрифтов формата TTF"
	?define T_INTRO_BUTTON_TEXT "Открыть шрифт"
#else
	?define T_INTRO "This is simple program to view TTF fonts."
	?define T_INTRO_BUTTON_TEXT "Open font"
#endif


void main()
{
	int id, key;

	load_dll(libtruetype, #truetype, 1);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);

	if (param[0]) OpenFont(#param);

	loop()
   {
      switch(WaitEvent())
      {	
         case evButton:
            id=GetButtonID();               
            if (id==1) ExitProcess();
            if (id==10)
            {
            	OpenDialog_start stdcall (#o_dialog);
				OpenFont(#openfile_path);
            }
			break;
      
        case evKey:
			key = GetKey();
			break;
         
         case evReDraw:
			draw_window();
      }
   }
}


void draw_window()
{
	system.color.get();
	DefineAndDrawWindow(30, 100, 800, 250+GetSkinHeight(), 0x34, 0xFFFfff, #win_title);
	GetProcessInfo(#Form, SelfInfo);
	DrawFonts();	
}


word DrawFonts()
{
	if (!font_data) 
	{
		WriteTextCenter(0,85,Form.cwidth,0x555555, T_INTRO);
		DrawCaptButton(Form.cwidth - 140 / 2, Form.cheight - 30 / 2, 140, 30, 10, system.color.work_button, system.color.work_button_text, T_INTRO_BUTTON_TEXT);
		return;
	}
	text_out stdcall (#test_text, #font_info, 10, 0x000000, 0xFFFfff, 3, 4);
	text_out stdcall (#test_text, #font_info, 12, 0x000000, 0xFFFfff, 3, 18);
	text_out stdcall (#test_text, #font_info, 24, 0x000000, 0xFFFfff, 3, 35);
	text_out stdcall (#test_text, #font_info, 36, 0x000000, 0xFFFfff, 3, 60);
	text_out stdcall (#test_text, #font_info, 48, 0x000000, 0xFFFfff, 3, 110);
	text_out stdcall (#test_text, #font_info, 58, 0x000000, 0xFFFfff, 3, 170);
}


void OpenFont(dword font_path) 
{
	BDVK FontFile_atr;
	GetFileInfo(font_path, #FontFile_atr);
	font_data = malloc(FontFile_atr.sizelo);
	ReadFile(0, FontFile_atr.sizelo, #font_data, font_path);
	init_font stdcall (#font_info, #font_data);
	if (EAX==0)
	{
		font_data = 0;
		notify("'Can\096t open font: init_font failed' - E");
	}
	strcpy(#win_title + 18, font_path);
	draw_window();
}

stop:
