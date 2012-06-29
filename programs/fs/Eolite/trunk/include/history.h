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
	copystr(#path,#cur_fol);
	cur_fol[strlen(#cur_fol)-1]=0x00; //обрезаем последний /
	copystr(#cur_fol+find_symbol(#cur_fol,'/'),#cur_fol);
	return #cur_fol;
}

void HistoryPath(byte action)
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
				copystr(#history_list[MAX_HISTORY_NUM-i].Item, #history_list[i].Item);
			}	
		}
		history_current++;
		copystr(#path,#history_list[history_current].Item);
		history_num=history_current;
	}
	
	if (action==GO_BACK)
	{
		if (history_current<=2) return;
		history_current--;
		copystr(#history_list[history_current].Item,#path);
	}

	if (action==GO_FORWARD)
	{
		if (history_current==history_num) return;
		history_current++;
		copystr(#history_list[history_current].Item,#path);
	}	
}