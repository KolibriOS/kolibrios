//06.04.2012

path_string history_list[40];
int history_num;
int history_current;

#define ADD_NEW_PATH 1
#define GO_BACK      2
#define GO_FORWARD   3

dword GetCurrentFolder()
{
	char cur_fol[4096];
	strcpy(#cur_fol, #path);
	cur_fol[strlen(#cur_fol)-1]=0x00; //обрезаем последний /
	strcpy(#cur_fol, #cur_fol+strrchr(#cur_fol,'/'));
	return #cur_fol;
}

int HistoryPath(byte action)
{
	int MAX_HISTORY_NUM;
	
	if (action==ADD_NEW_PATH)
	{
		if (history_num>0) && (!strcmp(#path,#history_list[history_current].Item)) return;
		
		MAX_HISTORY_NUM = sizeof(history_list)/sizeof(path_string);
		if (history_current>=MAX_HISTORY_NUM-1)
		{
			history_current/=2;
			for (i=0; i<history_current; i++;)
			{
				strcpy(#history_list[i].Item, #history_list[MAX_HISTORY_NUM-i].Item);
			}	
		}
		history_current++;
		strcpy(#history_list[history_current].Item, #path);
		history_num=history_current;
	}
	
	if (action==GO_BACK)
	{
		if (history_current<=2) return 0;
		history_current--;
		strcpy(#path, #history_list[history_current].Item);
		return 1;
	}

	if (action==GO_FORWARD)
	{
		if (history_current==history_num) return 0;
		history_current++;
		strcpy(#path, #history_list[history_current].Item);
		return 1;
	}	
}