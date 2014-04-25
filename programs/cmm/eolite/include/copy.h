//Leency 2008-2014

byte copy_to[4096];
byte cut_active=0;

enum {NOCUT, CUT, COPY_PASTE_END};

Clipboard clipboard;

void Copy(dword pcth, char cut)
{
	if (mark_active == 0) {
		strlcpy(#elements_path.element_list[elements_path.count].Item, pcth);
		elements_path.count++;
	}
	elements_path.size = sizeof(elements_path);
	clipboard.SetSlotData(sizeof(elements_path), #elements_path);
	cut_active = cut;
}

void copyf_Draw_Progress(dword filename) {
	#define WIN_W 300
	#define WIN_H 50
	DefineAndDrawWindow(Form.left+Form.width-200,Form.top+90,WIN_W,GetSkinHeight()+WIN_H-1,0x34,sc.work,T_PASTE_WINDOW);
	WriteText(5,8, 0x80, sc.work_text, T_PASTE_WINDOW_TEXT);
	DrawBar(5, 26, WIN_W-10, 10, sc.work);
	WriteText(5,26, 0x80, sc.work_text, filename);
	if (CheckEvent()==evButton) 
	{
		notify(T_CANCEL_PASTE);
		CopyExit();
	} 
}

void Paste()
{
	char copy_rezult;
	byte copy_from[4096];
	int tst, count, j;
	dword buf;
	
	buf = clipboard.GetSlotData(clipboard.GetSlotCount()-1);
	count = DSINT[buf+8];
	if (DSDWORD[buf+4] != 3) return;
	debugi(count);
	
	for (j = 0; j < count; j++) {
		tst = j*4096;
		strlcpy(#copy_from, buf+12+tst, 4096);
		if (!copy_from) CopyExit();
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
			CopyExit();
		}
		if (copy_rezult = copyf(#copy_from,#copy_to))
		{
			Write_Error(copy_rezult);
		}
	
		else if (cut_active)
		{
			strcpy(#file_path, #copy_from);
			Del_File(true);
			
		}
	}
	if (cut_active)
	{
		cut_active=false;
	}
	mark_default();
	CopyExit();
}

void CopyExit()
{
	action_buf = COPY_PASTE_END;
	ActivateWindow(GetProcessSlot(Form.ID));
	ExitProcess();
}