
void InstallationLoop()
{
	byte id, key, started=false;
	goto _INSTALL_DRAW;
   
	loop() switch(WaitEvent())
	{						   
			case evButton:
					id=GetButtonID();
					if(id == 1) ExitProcess();
					if (id == 11) RunProgram("/sys/htmlv", "http://kolibri-n.org/donate.php");
					if (id == 10) HalloLoop();
					break;
			case evKey:
					key = GetKey();
					break;
				   
			case evReDraw: _INSTALL_DRAW:
					if !(DefineWindow("Installation Started", "Stop")) break;
					if (started) break;
					started = true;
					Install();
	}
}


char iclock[3]={1,2};
void ShowProgress(dword text1)
{
	iclock[0]><iclock[1]; 
	_PutImage(WIN_W+LOGOW/2, WIN_H+LOGOH/2, LOGOW,LOGOH, LOGOW*LOGOH*3*iclock[0]+ #logo);
	if (text1)
	{
		DrawBar(TEXTX, BLACK_H+30, Form.cwidth-TEXTX, 12, 0xFFFfff);
		DrawBar(TEXTX, BLACK_H+50, Form.cwidth-TEXTX, 12, 0xFFFfff);
		WriteText(TEXTX, BLACK_H+30, 0x80, 0, text1);
	}
}


dword *copyfiles[] = {
	"sys /sys",
	"tmp /tmp9/1",
	0
};


void Install()
{
	int i;
	proc_info Process;

	ShowProgress("Mounting virtual disk...");
	if (TmpDiskAdd(9, 100)!=0) RunProgram("/sys/tmpdisk", "a9s100");
	for (i=2; i<256; i++;)
	{
		GetProcessInfo(#Process, i);
		if (i==Form.ID) || (strchr(#Process.name, '/')) || (strchr(#Process.name, 'Z')) continue;
		KillProcess(i);
	}
	RunProgram("/sys/REFRSCRN", NULL);
	pause(100);
	ShowProgress("Copying files...");
	copyf("/sys/docpack /tmp9/1/docpack");
	DeleteFile("/sys/docpack");
	for (i = 0; copyfiles[i]!=0; i++) copyf(copyfiles[i]);
	ShowProgress("Post install actions...");
	RunProgram("/sys/launcher", NULL);
	RunProgram("/sys/media/kiv", "\\S__/tmp9/1/wallpapers/Retro flower.jpg");
	SetSystemSkin("/tmp9/1/skins/latte.skn");
	EndLoop();
}


void EndLoop()
{
	byte id, key;

	goto _END_DRAW;
   
	loop() switch(WaitEvent())
	{						   
			case evButton:
					id=GetButtonID();
					if(id == 1) ExitProcess();
					if (id == 11) RunProgram("/sys/htmlv", "http://kolibri-n.org/index.php");
					if (id == 10) ExitProcess();
					break;
			case evKey:
					key = GetKey();
					break;
				   
			case evReDraw: _END_DRAW:
					if !(DefineWindow("Installation complete", "Exit")) break;
					WriteText(TEXTX, BLACK_H*2, 0x80, 0, "KolibriN install complete.");
					WriteText(TEXTX, BLACK_H*2+40, 0x80, 0, "I spent a lot of time improving KolibriN, so I hope you'll like it.");
					WriteText(TEXTX, BLACK_H*2+55, 0x80, 0, "Please, donate as much as you can to help me further improve Kolibri,");
					WriteText(TEXTX, BLACK_H*2+70, 0x80, 0, "the project I love so much. Visit my site for more information:");
					DrawLink(TEXTX, BLACK_H*2+85, 0x80, 11, "http://kolibri-n.org/donate.php");
	}
}
