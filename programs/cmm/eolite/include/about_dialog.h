//Leency 2008-2013

#define EDITOR_PATH     "/sys/tinypad"
#define BROWSER_PATH    "/sys/htmlv"
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
	IF (active_about) ExitProcess();
	active_about=1;
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				IF (id==1) || (id==10) 
				{
					active_about=0;
					ExitProcess();
				}
				IF (id==23) RunProgram(BROWSER_PATH, BROWSER_LINK);
				break;
				
		case evKey:
				IF (GetKey()==27)
				{
					active_about=0;
					ExitProcess();
				}
				break;
				
		case evReDraw:
				DefineAndDrawWindow(600,150,181,232+GetSkinHeight(),0x34,col_work,INTRO_TEXT_1);
				DrawBar(0,0,172,50,0x8494C4);
				PutPaletteImage(#logo,85,85,43,7,8,#logo_pal);
				WriteTextB(46,100,0x90,0xBF40BF,ABOUT_TITLE);
				#ifdef LANG_RUS
				WriteText(50,120,0x80,0,INTRO_TEXT_2);
				#else
				WriteText(55,120,0x80,0,INTRO_TEXT_2);
				#endif				
				WriteText(39,130,0x80,0,"Leency & Veliant"); 
				WriteText(45,140,0x80,0,"KolibriOS Team");
				WriteText(61,150,0x80,0,"2008-2013");
				#ifdef LANG_RUS
				WriteText(19,170,0x80,0,INTRO_TEXT_3);
				#else
				WriteText(29,170,0x80,0,INTRO_TEXT_3);
				#endif				
				DrawLink(71,170,0x80,23, "kolibri-n.org");
				DrawFlatButton(85,190,70,22,10,0xE4DFE1, INTRO_TEXT_4);
				DrawFilledBar(0, 216, 172, 12);
	}
}