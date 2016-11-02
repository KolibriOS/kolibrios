#define MEMSIZE 4096*10
#include "../lib/gui.h"
proc_info Form;

#define WINDOW_TITLE "Driver Installer"
#define T_CAUTION_TITLE "CAUTION"
#define T_CAUTION_PARAGRAPH "Installing additional drivers can be harmful to the stability of the operation system and potentionally can harm hardware."
#define T_ASSEPT_RISK "I assept the risk"
#define T_README "Readme"
#define T_INSTALL "Install"

#define BUTTON_ID_ASSEPT_RISK 10
#define BUTTON_ID_README 11
#define BUTTON_ID_INSTALL 12

//WINDOW STEPS
#define WINDOW_STEP_INTRO 1;
#define WINDOW_STEP_DRIVER_LIST 2;
char window_step = WINDOW_STEP_INTRO;

void main()
{
	word id;
	loop() switch(WaitEvent())
	{
		case evButton:
			id=GetButtonID();               
			if (id==1) ExitProcess();
			if (id==BUTTON_ID_ASSEPT_RISK) {
				window_step = WINDOW_STEP_DRIVER_LIST;
				goto _EV_WINDOW_REDRAW;
			}
			break;
	  
		case evKey:
			GetKeys();
			break;
		 
		case evReDraw:
		_EV_WINDOW_REDRAW:
			system.color.get();
			DefineAndDrawWindow(215, 100, 550, 350, 0x33, system.color.work, WINDOW_TITLE);
			GetProcessInfo(#Form, SelfInfo);
			if (window_step == WINDOW_STEP_INTRO) draw_intro_window();
			if (window_step == WINDOW_STEP_DRIVER_LIST) draw_driver_list_window();
			break;
	}
} 


void draw_intro_window()
{
	incn y;
	y.n = Form.cheight/2 - 80;
	WriteTextB(30+2,y.n+2,0x81,MixColors(system.color.work, 0xB92234,220),T_CAUTION_TITLE);
	WriteTextB(30,y.n,0x81,0xB92234,T_CAUTION_TITLE);
	y.n = DrawTextViewArea(30, y.inc(30), Form.cwidth-60, Form.cheight-140, 
		T_CAUTION_PARAGRAPH, -1, system.color.work_text);
	DrawStandartCaptButton(30, y.inc(10), BUTTON_ID_ASSEPT_RISK, T_ASSEPT_RISK);
}


void draw_driver_list_window()
{
	int PADDING = 12;
	int right_frame_x = Form.cwidth/2 + PADDING + calc(PADDING/2);
	incn y;
	y.n = PADDING + 3;
	//LEFT FRAME
	DrawBar(PADDING, PADDING, Form.cwidth/2 - PADDING, Form.cheight - PADDING - PADDING, 0xEEEeee);
	//RIGHT FRAME
	WriteTextB(right_frame_x, y.n, 0x81, system.color.work_text, "ATI KMS");
	WriteText(right_frame_x, y.inc(20), 0x80, MixColors(system.color.work, 0xCCCccc,80), "ver 4.4");
	DrawTextViewArea(right_frame_x-2, y.inc(30), Form.cwidth - right_frame_x - PADDING, Form.cheight-100, 
		T_CAUTION_PARAGRAPH, -1, system.color.work_text);
	right_frame_x += DrawStandartCaptButton(right_frame_x, Form.cheight-40, BUTTON_ID_README, T_README);
	DrawStandartCaptButton(right_frame_x, Form.cheight-40, BUTTON_ID_INSTALL, T_INSTALL);
}
