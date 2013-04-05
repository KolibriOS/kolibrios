//Leency 2008-2013


byte copy_from[4096], cut_active=0;
enum {NOCUT, CUT};
#define WIN_W 300
#define WIN_H 20


void Copy(dword pcth, char cut)
{
	strcpy(#copy_from, pcth);
	cut_active = cut;
}

void copyf_Action(dword filename) {
	DefineAndDrawWindow(5000,0,WIN_W,WIN_H-1,0x01,col_work,0);
	DrawBar(WIN_W, 0, 1, WIN_H, 0x333333);
	DrawBar(0, 0, WIN_W, WIN_H, 0xDDDddd);
	WriteText(5,6, 0x80, 0, "Copying file:");
	WriteText(90,6, 0x80, 0, filename);
	//pause(20);
}


void Paste()
{
	char copy_to[4096], copy_rezult;
	
	if (!copy_from) ExitProcess();
	strcpy(#copy_to, #path);
	strcat(#copy_to, #copy_from+strrchr(#copy_from,'/'));
	if (!strcmp(#copy_from,#copy_to))
	{
		strcpy(#copy_to, #path);
		strcat(#copy_to, "new_");
		strcat(#copy_to, #copy_from+strrchr(#copy_from,'/'));
	}
	if (strstr(#copy_to, #copy_from))
	{
		notify("Copy directory into itself is a bad idea...");
		ExitProcess();
	}
	if (copy_rezult = copyf(#copy_from,#copy_to))
	{
		Write_Error(copy_rezult);
	}
	else if (cut_active)
	{
		strcpy(#file_path, #copy_from);
		Del_File(true);
		copy_from=NULL;
		cut_active=false;
	}
	ActivateWindow(GetProcessSlot(Form.ID));
	SendWindowMessage(evKey, 54);
	SelectFile(#copy_to+strrchr(#copy_to,'/'));
	ExitProcess();
}
