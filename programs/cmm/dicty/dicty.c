#define MEMSIZE 0x8000
#include "..\lib\kolibri.h" 
#include "..\lib\strings.h"
#include "..\lib\mem.h"
#include "..\lib\file_system.h"
#include "..\lib\figures.h"
#include "..\lib\dll.h"
#include "..\lib\lib.obj\box_lib.h"


unsigned char speaker[23*40*3]= FROM "speaker.raw";
char title[]= "Dictionary v1.31";
char direction[] = "Translate direction:";
char translate_caption[] = "Translate";
char dict_not_found[] = "Dictionary not found";
char dict_opened[] = "Dictionary loaded";
char empty_word[] = "Type a word to translate";


system_colors sc;
proc_info Form;

char edword[256], search_word[256], translate_result[4096], dict_folder[4096], cur_dict[256];
#define DICT_DIRECROTY "dictionaries"
#define PRONOUNCED_FILE "/tmp9/1/dicty/pronounced.txt"
#define SPEECH_PATH "/tmp9/1/media/speech/speech"
dword dir_buf, file_buf, fsize;

int mouse_dd, speaker_id;
edit_box edit1= {200,20,16,0xffffff,0x94AECE,0x94AECE,0x94AECE,0,248,#edword,#mouse_dd,100000000000010b};


void main()
{   
	int id, key;
   	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0) notify("Error while loading GUI library /sys/lib/boxlib.obj");
	
	program_path[strrchr(#program_path, '/')] = 0;
	strcpy(#dict_folder, #program_path);
	strcat(#dict_folder, DICT_DIRECROTY);
	SetCurDir(#dict_folder);
	OpenDictionary(0);
	
	if (param)
	{
		strcpy(#edword, #param);
		edit1.size=edit1.pos=strlen(#edword);
		Translate();
		DrawTranslation();
		edit_box_draw stdcall(#edit1);
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
            if (id==1) {KillProcess(speaker_id); ExitProcess();}
			if (id==10) { Translate(); DrawTranslation(); }
			if (id==11)	ShowDictList();
			if (id==12)	DrawWindowContent();
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
				SpeakerDraw(Form.cwidth-38, Form.cheight-32);
			}
			if (id>=20)	OpenDictionary(id - 20);
			break;

        case evKey:
			key = GetKey();
			IF (key==013) //Enter
			{
				Translate();
				DrawTranslation();
			}
			
			EAX=key<<8;
			edit_box_key stdcall(#edit1);
			
			//LiveSearch();
			break;
			
         case evReDraw:
			DrawWindowContent();
			break;
      }
   }
}


void DrawWindowContent()
{
	sc.get();
	DefineAndDrawWindow(215,120,400,250,0x73,sc.work,#title);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	if (Form.height<140) MoveSize(OLD,OLD,OLD,140);
	if (Form.width<400) MoveSize(OLD,OLD,400,OLD);
	edit1.focus_border_color=sc.work_graph;
	edit1.width=Form.width-edit1.left-edit1.left-9;

	DrawBar(0, 0, Form.width-9, 69, sc.work);
	edit_box_draw stdcall(#edit1);
	DrawCaptButton(edit1.left+edit1.width-80,35, 80,20, 10, sc.work_button, sc.work_button_text,#translate_caption);
	DrawBar(0, 69, Form.width-9, 1, sc.work_graph);

	WriteText(edit1.left,35+7,0x80,sc.work_text,#direction);
	DefineButton(edit1.left+130,35, 120,20, 11, sc.work_button);
	DrawBar(edit1.left+130+1,36,  107,19, 0xFFFFFF);
	WriteText(edit1.left+130+112,35+7,0x80,sc.work_button_text,"\x19");
	WriteText(edit1.left+130+8,35+7,0x80,0x000000,#cur_dict);

	DrawTranslation();
}

void SpeakerDraw(dword x, y)
{
	if (!strstr(#cur_dict, "- rus")) return;
	DefineButton(x-5, y-5, 23+10, 20+9, 15+BT_HIDE+BT_NOFRAME, 0);
	if (GetProcessSlot(speaker_id)==0)) _PutImage(x, y, 23,20, #speaker); else _PutImage(x, y, 23,20, 23*20*3+#speaker);
}

void Translate()
{
	dword tj;
	char w_native[100], w_translation[100], bukva[1];
	
	byte InfType;
	#define NATIVE_WORD 0
	#define TRANSLATION 1
	#define IGNORE      2
	
	if (!fsize) return;
	KillProcess(speaker_id);
		
	translate_result = 0;
	strcpy(#search_word, #edword);
	strupr(#search_word);
	
	if (!edword)
	{
		strcpy(#translate_result, #empty_word);
		return;
	}

	for (tj=0; tj<fsize; tj++;) 
	{   
		bukva = ESBYTE[file_buf+tj];
		switch (bukva)
		{
			case '"':
				if (w_translation)
				{
					if (!strcmp(#w_native, #search_word))
					{
					
						if (translate_result) strcat(#translate_result, ", ");
						strcat(#translate_result, #w_translation);
					}
					else
						if (translate_result) return;
										
					w_translation = w_native = 0;
				}
				InfType = TRANSLATION;
				break;				
			case 0x0a:
				InfType = NATIVE_WORD;
				break;
			default:
				if (InfType==NATIVE_WORD)
				{
					chrcat(#w_native, bukva);
					//if (w_native<>search_word) InfType = IGNORE; //если первая буква не совпадает игнорим всё слово
				}
				if (InfType==TRANSLATION) chrcat(#w_translation, bukva);
		}
	}
	if (!translate_result) strcpy(#translate_result, "Word is'nt found in the dictionary"); 
}


void OpenDictionary(dword fileid)
{
	KillProcess(speaker_id);
	if (!dir_buf) ShowDictList();
	if (!dir_buf) strcpy(#cur_dict, "none");
	else strcpy(#cur_dict, fileid*304+dir_buf+72);
	fsize = ESDWORD[fileid*304 + dir_buf+64];

	free(file_buf);
	file_buf = malloc(fsize);
	ReadFile(0, fsize, file_buf, #cur_dict);
	IF (EAX<>0)
	{
		fsize = 0;
		strcpy(#search_word, "Error #");
		strcat(#search_word, itoa(EAX));
		strcpy(#translate_result, #dict_not_found);
		DrawWindowContent();
		return;
	}	
	strcpy(#search_word, #cur_dict);
	strcpy(#translate_result, #dict_opened);
	DrawWindowContent();
}


void ShowDictList()
{
	int j, fcount, error;
	
	free(dir_buf);
	error = GetDir(#dir_buf, #fcount, #dict_folder, DIRS_ONLYREAL);
	if (!error)
	{
		DefineButton(0,0, Form.width,Form.height, 12+BT_HIDE+BT_NOFRAME, sc.work_button);
		for (j=0; j<fcount; j++;)
		{
			DefineButton(edit1.left+130,j+1*20+35, 107,20, 20+j, sc.work_button);
			WriteText(edit1.left+130+8,j+1*20+35+7,0x80,sc.work_button_text, j*304+dir_buf+72);
		}
	}
}


void DrawTranslation()
{
	int text_break=0;
	char tt[4096]='';
	
	int y_pos=70;
	char draw_buf[4096];
	strcpy(#draw_buf, #translate_result);
	
	DrawBar(0, y_pos, Form.width-9, Form.cheight - y_pos, 0xFFFFFF);
	strttl(#draw_buf);
	WriteTextB(10+1, y_pos+8, 0x90, 0x800080, #search_word);
	while (draw_buf)
	{
		text_break= Form.width/6-6;
		if (text_break>strlen(#draw_buf))
		{
			WriteText(10, y_pos+21, 0x80, 0, #draw_buf);
			break;
		}
		while (draw_buf[text_break]<>' ') && (text_break>0) text_break--;
		strcpy(#tt, #draw_buf+text_break+1);
		draw_buf[text_break]=0x0;
		WriteText(10, y_pos+21, 0x80, 0, #draw_buf);
		strcpy(#draw_buf, #tt);
		y_pos+=12;
		if (y_pos+24+8>Form.cheight) break; //чтоб не залезало на нижний ободок
	}
	SpeakerDraw(Form.cwidth-38, Form.cheight-32);
}


stop:
