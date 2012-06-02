struct UrlsHistory {
	dword CurrentUrl();
	void AddUrl();
	void GoBack();
	void GoForward();
};

UrlsHistory BrowserHistory;

struct path_string {
char Item[4096];
};

#define MAX_HISTORY_NUM 40
path_string history_list[MAX_HISTORY_NUM];
int history_num;
int history_current;

dword UrlsHistory::CurrentUrl()
{
	return #history_list[history_current].Item;
}

void UrlsHistory::AddUrl() //тут нужен вводимый элемент - для универсальности
{
	if (history_num>0) && (strcmp(#URL,#history_list[history_current].Item)==0) return;

	if (history_current>=MAX_HISTORY_NUM-1)
	{
		history_current/=2;
		for (i=0; i<history_current; i++;)
		{
			copystr(#history_list[MAX_HISTORY_NUM-i].Item, #history_list[i].Item);
		}	
	}
	history_current++;
	copystr(#URL,#history_list[history_current].Item);
	history_num=history_current;
}


void UrlsHistory::GoBack()
{
	if (history_current<=1) return;
	history_current--;
	copystr(#history_list[history_current].Item,#URL);
}


void UrlsHistory::GoForward()
{
	if (history_current==history_num) return;
	history_current++;
	copystr(#history_list[history_current].Item,#URL);
}