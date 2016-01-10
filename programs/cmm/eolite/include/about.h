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
				if (about_form.status_window>2) break;
				_PutImage(10, 23, 86, 86, #logo);
				WriteTextB(112,16,0x82,0xD49CD2,ABOUT_TITLE);
				WriteTextB(110,14,0x82,0xBF40BF,ABOUT_TITLE);
				WriteTextLines(110,53,10010000b,0,DEVELOPERS_TEXT,21);
				#ifdef LANG_RUS
				DrawFlatButton(about_form.cwidth-310,about_form.cheight-38,180,26,11,"История разработки");
				#endif
				DrawFlatButton(about_form.cwidth-110,about_form.cheight-38, 90,26,10,CLOSE_BUTTON_TEXT);
	}
}
