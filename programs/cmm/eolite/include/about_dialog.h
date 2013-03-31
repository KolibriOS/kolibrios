//Leency 2008-2013

#define EDITOR_PATH     "/sys/tinypad"
#define BROWSER_PATH    "/sys/htmlv"
#define BROWSER_LINK    "http://kolibri-n.org/index.php"


void about_dialog()
{   
	byte id;
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				IF (id==1) || (id==10) ExitProcess();
				IF (id==23) RunProgram(BROWSER_PATH, BROWSER_LINK);
				break;
				
		case evKey:
				IF (GetKey()==27) ExitProcess();
				break;
				
		case evReDraw:
				DefineAndDrawWindow(600,150,181,232+GetSkinHeight(),0x34,col_work,"About Eolite");
				DrawBar(0,0,172,50,0x8494C4);
				PutPaletteImage(#logo,85,85,43,7,8,#logo_pal);
				WriteTextB(46,100,0x90,0xBF40BF,ABOUT_TITLE);
				WriteText(55,120,0x80,0,"Developers:"); 
				WriteText(39,130,0x80,0,"Leency & Veliant"); 
				WriteText(45,140,0x80,0,"KolibriOS Team");
				WriteText(61,150,0x80,0,"2008-2013");
				WriteText(29,170,0x80,0,"Visit");
				DrawLink(66,170,0x80,23, "kolibri-n.org");
				DrawFlatButton(85,190,70,22,10,0xE4DFE1, "Close");
				DrawFilledBar(0, 216, 172, 12);
	}
}