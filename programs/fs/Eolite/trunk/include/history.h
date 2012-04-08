//06.04.2012

path_string history_list[40];
int history_num;
int history_current;

#define add_new_path 1
#define go_back 2
#define go_forward 3

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
	/*WriteDebug("");
	WriteDebug(IntToStr(history_current));
	for (i=0; i<history_num; i++;)
	{
		WriteDebug(#history_list[i].Item);
		WriteDebug(IntToStr(history_num));
	}*/
	
	if (action==add_new_path)
	{
		if (history_num>0) && (strcmp(#path,#history_list[history_current].Item)==0) return;
		history_current++;
		copystr(#path,#history_list[history_current].Item);
		history_num=history_current;
	}
	
	if (action==go_back)
	{
		if (history_current<=2) return;
		WriteDebug("");
		WriteDebug("Fuck!");
		history_current--;
		copystr(#history_list[history_current].Item,#path);
	}

	if (action==go_forward)
	{
		if (history_current==history_num) return;
		history_current++;
		copystr(#history_list[history_current].Item,#path);
		SelectFile("");
	}	
}