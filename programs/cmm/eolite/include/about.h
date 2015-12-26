//Leency 2008-2016

#define BROWSER_PATH    "/sys/network/webview"
#define BROWSER_LINK    "http://kolibri-n.org/inf/eolite/eolite_p1"

#ifdef LANG_RUS
	?define DEVELOPERS_TEXT "Разработчики:\nLeency Veliant PunkJoker Pavelyakov\nKolibriOS Team\n2008-2016"
	?define CLOSE_BUTTON_TEXT "Закрыть"
#else
	?define DEVELOPERS_TEXT "Developers:\nLeency Veliant PunkJoker Pavelyakov\nKolibriOS Team\n2008-2016"
	?define CLOSE_BUTTON_TEXT "Close"
#endif

void about_dialog()
{   
	byte id;
	proc_info about_form;

	if (active_about) {cmd_free=2;ExitProcess();} else active_about=1;
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				IF (id==1) || (id==10) 
				{
					active_about=0;
					cmd_free = 2;
					ExitProcess();
				}
				IF (id==11) RunProgram(BROWSER_PATH, BROWSER_LINK);
				break;
				
		case evKey:
				IF (GetKey()==27)
				{
					active_about=0;
					cmd_free = 2;
					ExitProcess();
				}
				break;
				
		case evReDraw:
				DefineAndDrawWindow(Form.left+Form.width/2,Form.top+Form.height/2-114,440,200+GetSkinHeight(),0x34,system.color.work,0);
				GetProcessInfo(#about_form, SelfInfo);
				if (Form.status_window>2) break;
				_PutImage(10, 15, 86, 86, #logo);
				WriteTextB(110,15,0x81,0xBF40BF,ABOUT_TITLE);
				WriteTextLines(110,53,10010000b,0,DEVELOPERS_TEXT,21);
				#ifdef LANG_RUS
				DrawFlatButton(about_form.cwidth-250,about_form.cheight-35,130,22,11,0xE4DFE1,"История разработки");
				#endif
				DrawFlatButton(about_form.cwidth-100,about_form.cheight-35,70,22,10,0xE4DFE1,CLOSE_BUTTON_TEXT);
	}
}
