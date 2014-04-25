//Leency 2008-2014

byte copy_to[4096];
byte cut_active=0;
byte id_add_to_copy=0;
byte add_to_copy_active=0;
enum {NOCUT, CUT, COPY_PASTE_END};

struct path_str {
	char Item[4096];
};
 
#define MAX_HISTORY_NUM 10

Clipboard clipboard;

struct Copy_Path {
	dword	size;
	dword	type;
	int     count;
	path_str copy_list[MAX_HISTORY_NUM];
};	

Copy_Path copy_path;

void add_to_copy(dword pcth)
{
	strlcpy(#copy_path.copy_list[id_add_to_copy].Item, pcth);
	if (add_to_copy_active == 1)
	{
		id_add_to_copy++;
		copy_path.count = id_add_to_copy;
	}
	else copy_path.count = 1;
}


void Copy(dword pcth, char cut)
{
	if (add_to_copy_active == 0) add_to_copy(pcth);
	copy_path.type = 3;
	copy_path.size = sizeof(copy_path);
	clipboard.SetSlotData(sizeof(copy_path), #copy_path);
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
	
	add_to_copy_active=0;
	id_add_to_copy=0;
	
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
	for (j = 0; j < count; j++) strcpy(#copy_path.copy_list[j].Item[0], 0);
	CopyExit();
}

void CopyExit()
{
	action_buf = COPY_PASTE_END;
	ActivateWindow(GetProcessSlot(Form.ID));
	ExitProcess();
}