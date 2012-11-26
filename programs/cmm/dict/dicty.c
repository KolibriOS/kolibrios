#include "..\lib\kolibri.h" 
#include "..\lib\strings.h" 
#include "..\lib\file_system.h"
#include "..\lib\mem.h"
#include "..\lib\dll.h"
#include "..\lib\edit_box_lib.h"

char title[]= "Dictionary v1.0";
char direction[] = "Translate direction:";
char translate_caption[] = "Translate";
char dict_not_found[] = "Dictionary not found";
char dict_opened[] = "Dictionary loaded";
char empty_word[] = "Type a word to translate";
//char direction[] = "Ќаправление перевода:"w;


int mouse_dd;
edit_box edit1= {200,20,16,0xffffff,0x94AECE,0x94AECE,0x94AECE,0,248,#edword,#mouse_dd,100000000000010b};

system_colors sc;
proc_info Form;

char edword[4096], translate_result[4096], search_word[4096], cur_dict[4096];
#define DEFAULT_DICT_DIRECROTY "dictionaries/"
#define DEFAULT_DICT "eng - rus.dict";
dword files_buf;
dword buff, fsize;


void main()
{   
	int id;
	word key;
   
   	mem_Init();
	load_dll2(boxlib, #edit_box_draw,0);
	
	
	program_path[strrchr(#program_path, '/')] = 0; //обрезаем еЄ урл до последнего /
	strcpy(#program_path+strlen(#program_path), DEFAULT_DICT_DIRECROTY);
	SetCurDir(#program_path);

	strcpy(#cur_dict, DEFAULT_DICT);
	OpenDictionary(#cur_dict);
	
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
            if (id==1) ExitProcess();
			if (id==10)
			{
				Translate();
				DrawTranslation();
			}

			if (id==11)
			{
				ShowDictList();
				break;
			}
			if (id==12)
			{
				Draw_Window();
				break;
			}

			if (id>=20)
			{
				strcpy(#cur_dict, id-20*304+files_buf+72);
				OpenDictionary(#cur_dict);
			}
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
			break;
			
         case evReDraw:
			Draw_Window();
			break;
      }
   }
}


void Draw_Window()
{
	sc.get();
	DefineAndDrawWindow(215,120,400,250,0x73,sc.work,#title);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return; //если свернуто в заголовок, ничего не рисуем
	if (Form.height<140) MoveSize(OLD,OLD,OLD,140);
	if (Form.width<400) MoveSize(OLD,OLD,400,OLD);

	DrawBar(0, 0, Form.width-9, 69, sc.work);
	edit1.focus_border_color=sc.work_graph;
	edit1.width=Form.width-edit1.left-edit1.left-9;
	edit_box_draw stdcall(#edit1); //рисуем строку адреса
	DefineButton(edit1.left+edit1.width-80,35, 80,20, 10, sc.work_button);
	WriteText(edit1.left+edit1.width-80+14,35+7,0x80,sc.work_button_text,#translate_caption, 0);
	DrawBar(0, 69, Form.width-9, 1, sc.work_graph);
	
	WriteText(edit1.left,35+7,0x80,sc.work_text,#direction, 0);
	DefineButton(edit1.left+130,35, 120,20, 11, sc.work_button);
		WriteText(edit1.left+130+112,35+7,0x80,sc.work_button_text,"\x19", 0);
	DrawBar(edit1.left+130+1,36,  107,19, 0xFFFFFF);
		WriteText(edit1.left+130+8,35+7,0x80,0x000000,#cur_dict, 0);


	DrawTranslation();
}


void OpenDictionary(dword dict_path)
{
	dword j, words_count=0;
	
	mem_Free(buff);
	buff = mem_Alloc(3500576);
	ReadFile(0, 3500576, buff, dict_path);
	IF (EAX<>6)
	{
		fsize=0;
		strcpy(#translate_result, #dict_not_found);
		Draw_Window();
		return;
	}
	fsize=EBX;
	
	strcpy(#search_word, dict_path);
	strcpy(#translate_result, #dict_opened);
	Draw_Window();
}

void ShowDictList()
{
	int j, fcount=10;
	
	mem_Free(files_buf);
	files_buf= mem_Alloc(32);
	ReadDir(0, files_buf, #program_path);
	fcount=ESDWORD[files_buf+8];
	mem_Free(files_buf);
	files_buf = mem_Alloc(fcount+1*304+32);
	
	ReadDir(fcount, files_buf, #program_path);
	
	fcount-=2;
	mem_Move(files_buf,files_buf+608,fcount*304);
		
	
	DefineButton(0,0, Form.width,Form.height, 12+BT_HIDE+BT_NOFRAME, sc.work_button);
	for (j=0; j<fcount; j++;)
	{
		DefineButton(edit1.left+130,j+1*20+35, 107,20, 20+j, sc.work_button);
		WriteText(edit1.left+130+8,j+1*20+35+7,0x80,sc.work_button_text, j*304+files_buf+72, 0);
	}
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
		bukva = ESBYTE[buff+tj];
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
					strcat(#w_native, #bukva);
					//if (w_native<>search_word) InfType = IGNORE; //если перва€ буква не совпадает игнорим всЄ слово
				}
				if (InfType==TRANSLATION) strcat(#w_translation, #bukva);
		}
	}
	if (!translate_result) strcpy(#translate_result, "Word is'nt found in the dictionary"); 
}


void DrawTranslation()
{
	int text_break=0;
	char tt[4096]='';
	
	int y_pos=70, skin_height=GetSkinHeight();
	char draw_buf[4096];
	strcpy(#draw_buf, #translate_result);
	
	DrawBar(0, y_pos, Form.width-9, Form.height - y_pos-skin_height-4, 0xFFFFFF);
	strttl(#draw_buf);
	WriteText(10+1, y_pos+8, 0x90, 0x800080, #search_word, 0);
	WriteText(10  , y_pos+8, 0x90, 0x800080, #search_word, 0);
	while (draw_buf)
	{
		text_break= Form.width/6-6;
		if (text_break>strlen(#draw_buf))
		{
			WriteText(10, y_pos+21, 0x80, 0, #draw_buf, 0);
			return;
		}
		while (draw_buf[text_break]<>' ') && (text_break>0) text_break--;
		strcpy(#tt, #draw_buf+text_break+1);
		draw_buf[text_break]=0x0;
		WriteText(10, y_pos+21, 0x80, 0, #draw_buf, 0);
		strcpy(#draw_buf, #tt);
		y_pos+=12;
		if (y_pos+24+skin_height+12>Form.height) return; //чтоб не залезало на нижний ободок
	}
}


stop:
