//Leency 2008-2013

#define BROWSER_PATH    "/sys/network/webview"
#define BROWSER_LINK    "http://kolibri-n.org/index.php"

#ifdef LANG_RUS
	?define INTRO_TEXT_1 "О Eolite"
	?define INTRO_TEXT_2 "Разработчики:"
	?define INTRO_TEXT_3 "Посетите"
	?define INTRO_TEXT_4 "Закрыть"
#elif LANG_EST
	?define INTRO_TEXT_1 "Programmis Eolite"
	?define INTRO_TEXT_2 "Arendajad:"
	?define INTRO_TEXT_3 "K№lasta"
	?define INTRO_TEXT_4 "Sulge"
#else
	?define INTRO_TEXT_1 "About Eolite"
	?define INTRO_TEXT_2 "Developers:"
	?define INTRO_TEXT_3 "Visit"
	?define INTRO_TEXT_4 "Close"
#endif

void about_dialog()
{   
	byte id;
	proc_info about_form;

	IF (active_about){cmd_free = 2;ExitProcess();}
	active_about=1;
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
				IF (id==23) RunProgram(BROWSER_PATH, BROWSER_LINK);
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
				DefineAndDrawWindow(Form.left+Form.width/2,Form.top+Form.height/2-114,300,228+GetSkinHeight(),0x34,sc.work,INTRO_TEXT_1);
				GetProcessInfo(#about_form, SelfInfo);
				if (Form.status_window>2) break;
				DrawBar(0,0,about_form.cwidth,50,0x8494C4);
				PutPaletteImage(#logo,85,85,about_form.width/2-43,7,8,#logo_pal);
				WriteTextB(about_form.width/2-46,100,0x90,0xBF40BF,ABOUT_TITLE);
				WriteTextCenter(0,120,about_form.cwidth,0,INTRO_TEXT_2);			
				WriteTextCenter(0,130,about_form.cwidth,0,"Leency Veliant PunkJoker Pavelyakov"); 
				WriteTextCenter(0,140,about_form.cwidth,0,"KolibriOS Team");
				WriteTextCenter(0,150,about_form.cwidth,0,"2008-2015");
				WriteTextCenter(10,170,about_form.width-125,0,INTRO_TEXT_3);			
				DrawLink(about_form.width/2-15,170,0x80,23, "kolibri-n.org");
				DrawFlatButton(about_form.width/2-35,about_form.height-60,70,22,10,0xE4DFE1, INTRO_TEXT_4);
	}
}