void HalloLoop()
{
	byte id, key;
	goto _HALLO_DRAW;
	loop() switch(WaitEvent())
	{
					case evButton:
							id=GetButtonID();
							if(id == 1)   ExitProcess();
							if (id == 11) RunProgram("/sys/htmlv", "http://kolibri-n.org/index.php");
							if (id == 10) GotoInstall();
							break;
					case evKey:
							key = GetKey();
							if (key == 13) GotoInstall();
							break;
						   
					case evReDraw: _HALLO_DRAW:
							if !(DefineWindow("Prepearing installation", "Install")) break;

							HalloWindow();
							break;
	}
}

void HalloWindow()
{
	int free_ram;
	unsigned char free_ram_text[256];


	WriteTextB(TEXTX, 80, 0x90, 0xCC00CC, "KolibriN 8.2a Upgrade Pack is ready for install.");
	DrawLink(TEXTX, 95, 0x90, 11, "http://kolibri-n.org");
	
	free_ram = GetFreeRAM()/1024;
	strcpy(#free_ram_text, "You have ");
	strcat(#free_ram_text, itoa(free_ram));
	strcat(#free_ram_text, " MB of free RAM. You need 100 Mb for installation.");

	WriteText(TEXTX, 140, 0x80, 0, #free_ram_text);
	WriteText(TEXTX, 160, 0x80, 0, "Please, close all opened applications before start.");
}

void GotoInstall()
{
	if (GetFreeRAM()/1024>100) InstallationLoop(); 
	else notify("You do not have enought free RAM for installation!");
}