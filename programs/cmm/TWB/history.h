struct path_string {
	char Item[sizeof(URL)];
	int was_first;
};

#define MAX_HISTORY_NUM 40
path_string history_list[MAX_HISTORY_NUM];

struct UrlsHistory {
	int links_count;
	int current;
	dword CurrentUrl();
	dword GetUrl();
	dword GetFirstLine();
	void AddUrl();
	byte GoBack();
	byte GoForward();
};

dword UrlsHistory::CurrentUrl() {
	return #history_list[current].Item;
}

dword UrlsHistory::GetUrl(int id) {
	return #history_list[id].Item;
}

dword UrlsHistory::GetFirstLine(int id) {
	return history_list[id].was_first;
}

void UrlsHistory::AddUrl() {
	int i;
	if (links_count>0) && (!strcmp(#URL,#history_list[current].Item)) return;

	if (current>=MAX_HISTORY_NUM-1)
	{
		current/=2;
		for (i=0; i<current; i++;)
		{
			strlcpy(#history_list[i].Item, #history_list[MAX_HISTORY_NUM-i].Item, sizeof(URL));
		}	
	}
	current++;
	// history_list[i].was_first = WB1.list.first;
	strlcpy(#history_list[current].Item, #URL, sizeof(URL));
	links_count=current;
}


byte UrlsHistory::GoBack() {
	if (current<=1) return 0;
	current--;
	strlcpy(#URL, #history_list[current].Item, sizeof(URL));
	// stroka = history_list[current].was_first;
	return 1;
}


byte UrlsHistory::GoForward() {
	if (current==links_count) return 0;
	current++;
	strlcpy(#URL, #history_list[current].Item, sizeof(URL));
	return 1;
}

UrlsHistory BrowserHistory;