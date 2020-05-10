
void Drives__Main()
{
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON);
	goto _GENERAL_REDRAW_1;
	loop()
	{
		WaitEventTimeout(500);
		switch(EAX & 0xFF)
		{
			case evButton: Sysmon__ButtonEvent(); break;
			case evKey: Sysmon__KeyEvent(); break;
			case evReDraw: 
				_GENERAL_REDRAW_1: 
				Sysmon__DefineAndDrawWindow(); 
				WriteText(WIN_PAD, WIN_CONTENT_Y+25, 0x90, sc.work, "Update period: 5 seconds");
			default:
				MonitorRd();
				MonitorTmp();
		}
	}
}

void MonitorRd()
{
	sensor rd;
	dword rdempty = malloc(1440*1024);
	rd.set_size(WIN_PAD, WIN_CONTENT_Y+25, WIN_CONTENT_W, 23);
	CreateFile(0, 1440*1024, rdempty, "/rd/1/rdempty");
	free(rdempty);
	file_size stdcall ("/rd/1/rdempty");
	rdempty = EBX / 1024;
	DeleteFile("/rd/1/rdempty");

	sprintf(#param, "System disk usage: %i Kb free of 1440 Mb", rdempty);
	DrawIconWithText(WIN_PAD, rd.y - 25, 5, #param);

	rd.draw_progress(rdempty * rd.w / 1440);	
}

dword GetTmpDiskFreeSpace(int _id)
{
	DIR_SIZE dir_size;
	sprintf(#param, "/tmp%i/1", _id);
	dir_size.get(#param);
	dir_size.bytes += dir_size.files/2 + 32 * 512; //file attr size + FAT table size
	dir_size.bytes /= 1024*1024; //convert to MiB
	return dir_size.bytes;	
}

void MonitorTmp()
{
	char text_status[64];
	int i, yy=WIN_CONTENT_Y+95;
	dword tmp_size[10];
	dword free_space;
	sensor tmp;
	for (i=0; i<=9; i++) 
	{
		file_size stdcall ( sprintf(#param, "/tmp%i/1", i) );
		tmp_size[i] =  EBX / 1024 / 1024;

		if (EBX) {
			free_space = tmp_size[i] - GetTmpDiskFreeSpace(i);
			sprintf(#text_status, "TMP%i usage: %i Mb free of %i Mb", i, free_space, tmp_size[i]);
			tmp.set_size(WIN_PAD, yy, WIN_CONTENT_W, 23);
			tmp.draw_progress(free_space * WIN_CONTENT_W / tmp_size[i]);
			DrawIconWithText(WIN_PAD, tmp.y - 25, 50, #text_status);
			yy += 65;
		}
	}
}