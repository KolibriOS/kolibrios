#ifndef AUTOBUILD
	#include "lang.h--"
#endif

#define MEMSIZE 0x8000
#include "..\lib\strings.h"
#include "..\lib\io.h"
#include "..\lib\gui.h"
#include "..\lib\obj\box_lib.h"

unsigned char speaker[23*40*3]= FROM "speaker.raw";

#ifdef LANG_RUS
  #define WINDOW_TITLE "Словарик 2.0"
  #define DICTIONARY_NOT_FOUND "Словарь не найден"
  #define DICTIONARY_LOADED "Словарь загружен"
  #define WORD_NOT_FOUND "Слово не найдено в словаре"
  #define ERROR "Ошибка #"
#else
  #define WINDOW_TITLE "Dictionary v2.0"
  #define DICTIONARY_NOT_FOUND "Dictionary not found"
  #define DICTIONARY_LOADED "Dictionary loaded"
  #define WORD_NOT_FOUND "Word isn't found in the dictionary"
  #define ERROR "Error #"
  #endif

proc_info Form;
char edword[256], search_word[256], translate_result[4096];
#define PRONOUNCED_FILE "/sys/pronounced.txt"
#define SPEECH_PATH "/kolibrios/media/speech/speech"
#define TOPH 44

#define TEXT_ENG_RUS "ENG\26RUS"
#define TEXT_RUS_ENG "RUS\26ENG"
#define ENG_RUS 0
#define RUS_ENG 1
int active_dict=2;

int mouse_dd, speaker_id;
edit_box edit1= {200,16,16,0xffffff,0x94AECE,0xffffff,0x94AECE,0,248,#edword,#mouse_dd,100000000000010b};


void main()
{   
	int id;
	load_dll(boxlib, #box_lib_init,0);
	OpenDictionary(ENG_RUS);
	if (param)
	{
		strcpy(#edword, #param);
		edit1.size=edit1.pos=strlen(#edword);
		Translate();
	}
	SetEventMask(0x27);
	loop()
	{
		switch(WaitEvent()) 
		{
		case evMouse:
			edit_box_mouse stdcall (#edit1);
			break;

		case evButton:
            id=GetButtonID();               
            if (id==01) { KillProcess(speaker_id); ExitProcess(); }
			if (id==10) { OpenDictionary(ENG_RUS); DrawLangButtons(); }
			if (id==11) { OpenDictionary(RUS_ENG); DrawLangButtons(); }
			if (id==15)
			{
				if (GetProcessSlot(speaker_id)!=0))
				{
					KillProcess(speaker_id);
					pause(50);
				}
				else
				{
					if (WriteFile(strlen(#translate_result)+1, #translate_result, PRONOUNCED_FILE)!=0) break;
					pause(50);
					speaker_id = RunProgram(SPEECH_PATH, PRONOUNCED_FILE);
				}
				SpeakerDraw();
			}
			break;

        case evKey:
			GetKeys();			
			EAX=key_ascii<<8;
			edit_box_key stdcall(#edit1);	
			Translate();
			break;
			
         case evReDraw:
			system.color.get();
			DefineAndDrawWindow(215,120,400,250,0x73,system.color.work,WINDOW_TITLE);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window>2) break;
			if (Form.height<140) { MoveSize(OLD,OLD,OLD,140); break; }
			if (Form.width<400) { MoveSize(OLD,OLD,400,OLD); break; }
			DrawBar(0, 0, Form.width-9, TOPH, system.color.work); //top bg
			DrawBar(0, TOPH, Form.width-9, 1, system.color.work_graph);
			edit1.width=Form.width-edit1.left-edit1.left-9 - 116;
			edit_box_draw stdcall(#edit1);
			DrawWideRectangle(edit1.left-2, edit1.top-2, edit1.width+3, 19, 2, 0xffffff);
			DrawRectangle(edit1.left-3, edit1.top-3, edit1.width+4, 20, system.color.work_graph);
			DrawTranslation();
			DrawLangButtons();
      }
   }
}


void DrawLangButtons()
{
	DrawCaptButton(Form.width-130, edit1.top-3, 50,19, 10, system.color.work_button, system.color.work_button_text, TEXT_ENG_RUS);
	DrawCaptButton(Form.width-70, edit1.top-3, 50,19, 11, system.color.work_button, system.color.work_button_text, TEXT_RUS_ENG);
	PutShadow(-active_dict*60 + Form.width-70, edit1.top-3, 50,19, 1, 2);
	SpeakerDraw();
}

void SpeakerDraw()
{
	dword x, y;
	x = Form.cwidth-38;
	y = Form.cheight-32;
	if (active_dict)
	{
		DeleteButton(15);
		DrawBar(x,y,23,20,0xFFFFFF);
	}
	else
	{
		DefineButton(x-5, y-5, 23+10, 20+9, 15+BT_HIDE+BT_NOFRAME, 0);
		if (GetProcessSlot(speaker_id)==0)) _PutImage(x, y, 23,20, #speaker); else _PutImage(x, y, 23,20, 23*20*3+#speaker);		
	}
}

void Translate()
{
	dword translation_start, translation_end;

	KillProcess(speaker_id);
	sprintf(#search_word, "\10%s\13", #edword);
	strupr(#search_word);

	if (!io.FILES_SIZE) || (!edword) goto _TR_END;

	translation_start = strstr(io.buffer_data, #search_word);
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
	dword res;
	if (dict_id==active_dict) return;
	KillProcess(speaker_id);
	active_dict = dict_id;
	if (io.buffer_data) free(io.buffer_data);
	if (active_dict==ENG_RUS) res=io.read("dictionaries/eng - rus.dict");
	if (active_dict==RUS_ENG) res=io.read("dictionaries/rus - eng.dict");	
	if (!io.buffer_data)
	{
		sprintf(#search_word, ERROR, res);
		strcpy(#translate_result, DICTIONARY_NOT_FOUND);
	}
	else
	{
		if (active_dict==ENG_RUS) strcpy(#search_word, TEXT_ENG_RUS);
		if (active_dict==RUS_ENG) strcpy(#search_word, TEXT_RUS_ENG);
		strcpy(#translate_result, DICTIONARY_LOADED);
	}
	DrawTranslation();	
}


void DrawTranslation()
{
	int text_break=0;
	char tt[4096]='';
	
	int y_pos=TOPH+1;
	char draw_buf[4096];
	strcpy(#draw_buf, #translate_result);
	
	DrawBar(0, y_pos, Form.width-9, Form.cheight - y_pos, 0xFFFFFF);
	strttl(#draw_buf);
	WriteTextB(10+1, y_pos+8, 10000001b, 0x800080, #search_word);
	while (draw_buf)
	{
		text_break= Form.width/6-6;
		if (text_break>strlen(#draw_buf))
		{
			WriteText(10, y_pos+31, 0x80, 0, #draw_buf);
			break;
		}
		while (draw_buf[text_break]<>' ') && (text_break>0) text_break--;
		strcpy(#tt, #draw_buf+text_break+1);
		draw_buf[text_break]=0x0;
		WriteText(10, y_pos+31, 0x80, 0, #draw_buf);
		strcpy(#draw_buf, #tt);
		y_pos+=12;
		if (y_pos+24+8>Form.cheight) break;
	}
	SpeakerDraw();
}


stop:
