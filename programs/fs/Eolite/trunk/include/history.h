//06.04.2012

path_string history_list[40];
int history_num;
int history_current;

#define add_new_path 1
#define go_back 2
#define go_forvard 3

//history_current

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
	if (action==add_new_path)
	{
		if (history_num>0) && (strcmp(#path,#history_list[history_num-1].Item)==0) return;
		
		copystr(#path,#history_list[history_num].Item);
		history_num++;
	}
	
	if (action==go_back)
	{
		if (history_num<=2) return;
		history_num--;
		copystr(#history_list[history_num-1].Item,#path);
	}

	if (action==go_forvard)
	{
		WriteDebug("");
		for (i=0; i<history_num; i++;)
		{
			WriteDebug(#history_list[i].Item);
			WriteDebug(IntToStr(history_num));
		}
		if (strcmp("",#history_list[history_num].Item)==0) return;
		history_num++;
		copystr(#history_list[history_num-1].Item,#path);
		SelectFile("");
		return;
	}	
}