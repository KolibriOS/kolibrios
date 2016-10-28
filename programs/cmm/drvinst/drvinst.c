#define MEMSIZE 4096*10
#include "../lib/gui.h"
proc_info Form;

#define WINDOW_TITLE "Driver Installer"
#define T_CAUTION_TITLE "CAUTION"
#define T_CAUTION_PARAGRAPH "Installing additional drivers can be harmful to the stability of the operation system and potentionally can harm hardware."
#define T_ASSEPT_RISK "I assept the risk"

#define BUTTON_ID_ASSEPT_RISK 10


void main()
{
	word id;
	loop() switch(WaitEvent())
	{
		case evButton:
			id=GetButtonID();               
			if (id==1) ExitProcess();
			break;
	  
		case evKey:
			GetKeys();
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
} 


void draw_window()
{
	incn y;
	y.n=40;
	system.color.get();
	DefineAndDrawWindow(215, 100, 450, 250, 0x33, system.color.work, WINDOW_TITLE);
	GetProcessInfo(#Form, SelfInfo);
	WriteTextB(30+2,y.n+2,0x81,MixColors(system.color.work, 0xB92234,220),T_CAUTION_TITLE);
	WriteTextB(30,y.n,0x81,0xB92234,T_CAUTION_TITLE);
	y.n = DrawTextViewArea(30, y.inc(30), Form.cwidth-60, Form.cheight-140, 15, 
		T_CAUTION_PARAGRAPH, -1, system.color.work_text);
	DrawStandartCaptButton(30, y.inc(10), BUTTON_ID_ASSEPT_RISK, T_ASSEPT_RISK);
}



