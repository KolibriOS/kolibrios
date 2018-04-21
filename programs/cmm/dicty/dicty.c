#ifndef AUTOBUILD
	#include "lang.h--"
#endif

#define MEMSIZE 4096 * 1024 * 2
#include "../lib/strings.h"
#include "../lib/gui.h"
#include "../lib/obj/box_lib.h"

#ifdef LANG_RUS
  #define WINDOW_TITLE "Словарик 2.3"
  #define DICTIONARY_LOADED "Словарь загружен"
  #define WORD_NOT_FOUND "Слово не найдено в словаре"
  #define ERROR "Ошибка #%d"
#else
  #define WINDOW_TITLE "Dictionary v2.3"
  #define DICTIONARY_LOADED "Dictionary loaded"
  #define WORD_NOT_FOUND "Word isn't found in the dictionary"
  #define ERROR "Error #%d"
  #endif

proc_info Form;
char edit_text[256], search_word[256], translate_result[4096];
#define TOPH 45

unsigned char eng_rus[] = FROM "eng_rus.dict""\0";
unsigned char rus_eng[] = FROM "rus_eng.dict""\0";
dword io_buffer_data;

#define TEXT_ENG_RUS "ENG\26RUS"
#define TEXT_RUS_ENG "RUS\26ENG"
#define TEXT_VOC_R_E "ENG     RUS"
#define TEXT_VOC_E_R "RUS     ENG"
#define ENG_RUS 0
#define RUS_ENG 1
#define BUTTON_CHANGE_LANGUAGE 10
int active_dict=2;

edit_box edit1= {200,13,13,0xffffff,0x94AECE,0xffffff,0x94AECE,0x10000000,248,#edit_text,0,100000000000010b};



void main()
{   
	int id;
	load_dll(boxlib, #box_lib_init,0);
	OpenDictionary(ENG_RUS);
	if (param)
	{
		strcpy(#edit_text, #param);
		edit1.size=edit1.pos=strlen(#edit_text);
		Translate();
	}
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);	
	loop()
	{
		switch(WaitEvent()) 
		{
		case evMouse:
			edit_box_mouse stdcall (#edit1);
			break;

		case evButton:
            id=GetButtonID();               
            if (id==01) ExitProcess();
			if (id==BUTTON_CHANGE_LANGUAGE) { 
				if (active_dict == ENG_RUS) OpenDictionary(RUS_ENG); else OpenDictionary(ENG_RUS);
				DrawLangButtons();
			}
			break;

        case evKey:
			GetKeys();
			edit_box_key stdcall(#edit1);	
			Translate();
			break;
			
         case evReDraw:
			system.color.get();
			DefineAndDrawWindow(215,120,500,350,0x73,system.color.work,WINDOW_TITLE,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			if (Form.height<140) { MoveSize(OLD,OLD,OLD,140); break; }
			if (Form.width<400) { MoveSize(OLD,OLD,400,OLD); break; }
			DrawBar(0, 0, Form.width-9, TOPH, system.color.work); //top bg
			DrawBar(0, TOPH, Form.width-9, 1, system.color.work_graph);
			edit1.width=Form.width-edit1.left-edit1.left-9 - 116;
			edit_box_draw stdcall(#edit1);
			DrawWideRectangle(edit1.left-2, edit1.top-2, edit1.width+3, 25, 2, 0xffffff);
			DrawRectangle(edit1.left-3, edit1.top-3, edit1.width+4, 26, system.color.work_graph);
			DrawTranslation();
			DrawLangButtons();
      }
   }
}


void DrawLangButtons()
{
	dword direction;
	DrawBar(Form.width-120, edit1.top+3, 100, 25, system.color.work);
	DefineButton(Form.width-88, edit1.top-4+3, 20, 20, BUTTON_CHANGE_LANGUAGE, system.color.work_button);
	WriteText(Form.width-82, edit1.top-1+3, 10000001b, system.color.work_button_text, "\26");
	if (active_dict == ENG_RUS) direction = TEXT_VOC_R_E; else direction = TEXT_VOC_E_R;
	WriteText(Form.width-120, edit1.top+3, 0x90, system.color.work_text, direction);
}

void Translate()
{
	dword translation_start, translation_end;

	sprintf(#search_word, "\10%s\13", #edit_text);
	strupr(#search_word);

	if (!edit_text) goto _TR_END;

	translation_start = strstr(io_buffer_data, #search_word);
	if (!translation_start) 
	{
		strcpy(#translate_result, WORD_NOT_FOUND); 
	}
	else 
	{
		translation_start = strchr(translation_start, '"') + 1;
		translation_end = strchr(translation_start, '"');
		strlcpy(#translate_result, translation_start, translation_end - translation_start);
	}
	_TR_END:
	strcpy(#search_word, #search_word+1);
	DrawTranslation();
}


void OpenDictionary(dword dict_id)
{
	if (dict_id==active_dict) return;
	active_dict = dict_id;
	if (active_dict==ENG_RUS) 
	{
		io_buffer_data=#eng_rus;
		strcpy(#search_word, TEXT_ENG_RUS);
	}
	if (active_dict==RUS_ENG) 
	{
		io_buffer_data=#rus_eng;
		strcpy(#search_word, TEXT_RUS_ENG);
	}
	strcpy(#translate_result, DICTIONARY_LOADED);
	DrawTranslation();	
}


void DrawTranslation()
{
	int y_pos=TOPH+1;
	char draw_buf[4096];
	strlcpy(#draw_buf, #translate_result, sizeof(draw_buf));
	
	DrawBar(0, y_pos, Form.width-9, Form.cheight - y_pos, 0xFFFFFF);
	strttl(#draw_buf);
	WriteTextB(10+1, y_pos+8, 10000001b, 0x800080, #search_word);

	DrawTextViewArea(10, y_pos+31, Form.cwidth-20, Form.cheight-30, 
		#draw_buf, -1, 0x000000);
}


stop:
