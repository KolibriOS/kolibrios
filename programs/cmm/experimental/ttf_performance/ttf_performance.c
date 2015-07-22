#define MEMSIZE 397113
#include "../lib/kolibri.h" 
#include "../lib/strings.h" 
#include "../lib/mem.h" 
#include "../lib/file_system.h"
#include "../lib/dll.h"
#include "../lib/obj/truetype.h"

proc_info Form;
dword font_data;
stbtt_fontinfo font_info;


inline fastcall dword get_start_time()
{
	$mov eax,26
	$mov ebx,9
	$int 0x40
}

#define TESTS_NUM 10
char test_text[] = "The quick brown fox jumps over the lazy dog";
word rez[TESTS_NUM];

void main()
{   
	BDVK FontFile_atr;
	int id, key;

	mem_Init();
	if (load_dll2(libtruetype, #truetype, 1) != 0) notify("Error: library doesn't exists - truetype");

	if (param[0]==0) strcpy(#param, "/sys/arial.ttf");
	GetFileInfo(#param, #FontFile_atr);
	font_data = malloc(FontFile_atr.sizelo);
	ReadFile(0, FontFile_atr.sizelo, #font_data, #param);

	init_font stdcall (#font_info, #font_data);
	if (EAX==0) notify("init_font failed");
	
	loop()
   {
      switch(WaitEvent())
      {	
         case evButton:
            id=GetButtonID();               
            if (id==1) ExitProcess();
			break;
      
        case evKey:
			key = GetKey();
			IF (key==013){ //Enter
				debug("Tests count: ");
				debugi(TESTS_NUM);
				for (id=0; id<TESTS_NUM; id++) 
				{
					rez[id] = DrawFonts();
					debugi(rez[id]);
				}
				debugln("Done");
			}
			break;
         
         case evReDraw:
			DefineAndDrawWindow(30,100,800,300,0x34,0xFFFFFF,"TTF Viewer v0.1");
			GetProcessInfo(#Form, SelfInfo);
			WriteTextB(50,85,0x90,0xFF00FF,"Press Enter to start testing");
			break;
      }
   }
}


word DrawFonts()
{
	dword time1, time2;
	time1 = get_start_time();
	text_out stdcall (#test_text, #font_info, 10, time1, 0xFFFfff, 3, 4);
	text_out stdcall (#test_text, #font_info, 12, time1, 0xFFFfff, 3, 18);
	text_out stdcall (#test_text, #font_info, 24, time1, 0xFFFfff, 3, 35);
	text_out stdcall (#test_text, #font_info, 36, time1, 0xFFFfff, 3, 60);
	text_out stdcall (#test_text, #font_info, 48, time1, 0xFFFfff, 3, 110);
	text_out stdcall (#test_text, #font_info, 58, time1, 0xFFFfff, 3, 170);
	time2 = get_start_time();
	return time2 - time1;
}


stop:
