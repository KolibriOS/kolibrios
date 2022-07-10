#define MEMSIZE 4096 * 1024 * 2
#define NO_DLL_INIT
#include "../lib/strings.h"
#include "../lib/gui.h"
#include "../lib/obj/box_lib.h"

#ifdef LANG_RUS
  #define WINDOW_TITLE "Словарик 2.4"
  #define DICTIONARY_LOADED "Словарь загружен"
  #define WORD_NOT_FOUND "Слово не найдено в словаре"
  #define ERROR "Ошибка #%d"
#else
  #define WINDOW_TITLE "Dictionary v2.4"
  #define DICTIONARY_LOADED "Dictionary loaded"
  #define WORD_NOT_FOUND "Word isn't found in the dictionary"
  #define ERROR "Error #%d"
#endif

#define TEXT_ENG_RUS "ENG\26RUS"
#define TEXT_RUS_ENG "RUS\26ENG"
#define TEXT_RU2EN "ENG     RUS"
#define TEXT_EN2RU "RUS     ENG"

unsigned char speaker[23*40*3]= FROM "speaker.raw";

proc_info Form;
char search_word[256], translate_result[4096];
#define TOPH 45
#define PRONOUNCED_FILE "/tmp0/1/pronounced.txt"
#define SPEECH_PATH "/kolibrios/speech/speech"
dword speaker_id;

unsigned char eng_rus[] = FROM "eng_rus.dict""\0";
unsigned char rus_eng[] = FROM "rus_eng.dict""\0";
dword io_buffer_data;

#define ENG_RUS 0
#define RUS_ENG 1
int active_dict=2;

#define BTN_CHANGE_LANGUAGE 10
#define BTN_SPEAKER 11

char edit_text[256];
edit_box edit1 = {0,13,13,0xffffff,0x94AECE,0xffffff,0xffffff,
	0x10000000,248,#edit_text,0,ed_focus+ed_always_focus};

void main()
{   
	int btnid;
	load_dll(boxlib, #box_lib_init,0);
	OpenDictionary(ENG_RUS);
	if (param) {
		strcpy(#edit_text, #param);
		edit_box_set_text stdcall (#edit1, #param);
		Translate();
	}
	@SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);	
	loop() switch(@WaitEvent()) 
	{
		case evMouse:
			edit_box_mouse stdcall (#edit1);
			break;

		case evButton:
			btnid = @GetButtonID();               
			if (btnid==01) ExitProcess();
			if (btnid==BTN_CHANGE_LANGUAGE) { 
				if (active_dict == ENG_RUS) {
					OpenDictionary(RUS_ENG); 
				} else {
					OpenDictionary(ENG_RUS);
				}
				DrawLangButtons();
			}
			if (btnid==BTN_SPEAKER) {
				if (GetProcessSlot(speaker_id)!=0) {
					KillProcess(speaker_id);
					pause(50);
				} else {
					if (CreateFile(strlen(#translate_result)+1, #translate_result, PRONOUNCED_FILE)!=0) break;
					pause(50);
					speaker_id = RunProgram(SPEECH_PATH, PRONOUNCED_FILE);
				}
				SpeakerDraw();
			} 
			break;

		case evKey:
			@GetKey();
			edit_box_key stdcall(#edit1);	
			Translate();
			break;
			
		case evReDraw:
			sc.get();
			DefineAndDrawWindow(215,120,500,350,0x73,sc.work,WINDOW_TITLE,0);
			GetProcessInfo(#Form, SelfInfo);
			if (Form.status_window&ROLLED_UP) break;
			if (Form.height<140) { MoveSize(OLD,OLD,OLD,140); break; }
			if (Form.width<400) { MoveSize(OLD,OLD,400,OLD); break; }
			DrawBar(0, 0, Form.cwidth, TOPH, sc.work); //top bg
			DrawBar(0, TOPH, Form.cwidth, 1, sc.line);
			edit1.width = Form.cwidth-edit1.left-edit1.left - 116;
			edit_box_draw stdcall(#edit1);
			DrawWideRectangle(edit1.left-2, edit1.top-2, edit1.width+3, 25, 2, 0xffffff);
			DrawRectangle(edit1.left-3, edit1.top-3, edit1.width+4, 26, sc.line);
			DrawTranslation();
			DrawLangButtons();
	}
}


void DrawLangButtons()
{
	dword direction;
	int y = edit1.top+3;
	DrawBar(Form.cwidth-111, y, 100, 25, sc.work);
	DefineButton(Form.cwidth-79, y-4, 20, 20, BTN_CHANGE_LANGUAGE, sc.button);
	WriteText(Form.cwidth-73, y-1, 10000001b, sc.button_text, "\26");
	if (active_dict == ENG_RUS) {
		direction = TEXT_RU2EN; 
	} else {
		direction = TEXT_EN2RU;
	}
	WriteText(Form.cwidth-111, y, 0x90, sc.work_text, direction);
}

void Translate()
{
	dword tr_start, tr_end;

	sprintf(#search_word, "\10%s\13", #edit_text);
	strupr(#search_word);

	if (!edit_text) goto _TR_END;

	tr_start = strstr(io_buffer_data, #search_word);
	if (!tr_start) {
		strcpy(#translate_result, WORD_NOT_FOUND); 
	} else {
		tr_start = strchr(tr_start, '"') + 1;
		tr_end = strchr(tr_start, '"');
		strlcpy(#translate_result, tr_start, tr_end - tr_start);
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
	strlcpy(#draw_buf, #translate_result, sizeof(draw_buf)-1);
	
	DrawBar(0, y_pos, Form.cwidth, Form.cheight - y_pos, 0xFFFFFF);
	strttl(#draw_buf);
	WriteTextB(10+1, y_pos+8, 10000001b, 0x800080, #search_word);

	DrawTextViewArea(10, y_pos+31, Form.cwidth-20, Form.cheight-30, 
		#draw_buf, -1, 0x000000);

	SpeakerDraw();
}

void SpeakerDraw()
{
	dword x = Form.cwidth-38;
	dword y = Form.cheight-32;
	if (active_dict == RUS_ENG)
	{
		DeleteButton(15);
		DrawBar(x,y,23,20,0xFFFFFF);
	} else {
		DefineHiddenButton(x-5, y-5, 23+10, 20+9, BTN_SPEAKER);
		if (!GetProcessSlot(speaker_id)) {
			PutImage(x, y, 23,20, #speaker);
		} else {
			PutImage(x, y, 23,20, 23*20*3+#speaker);
		}
	}
}

stop:
