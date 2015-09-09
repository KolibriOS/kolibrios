struct path_string {
	char Item[sizeof(URL)];
	int was_first;
};

#define MAX_HISTORY_NUM 40
path_string history_list[MAX_HISTORY_NUM];

struct UrlsHistory {
	int links_count;
	int cur_y;
	dword CurrentUrl();
	dword GetUrl();
	dword GetFirstLine();
	void AddUrl();
	byte GoBack();
	byte GoForward();
} BrowserHistory;

dword UrlsHistory::CurrentUrl() {
	return #history_list[cur_y].Item;
}

dword UrlsHistory::GetUrl(int id) {
	return #history_list[id].Item;
}

dword UrlsHistory::GetFirstLine(int id) {
	return history_list[id].was_first;
}

void UrlsHistory::AddUrl() {
	int i;
	if (links_count>0) && (!strcmp(#URL,#history_list[cur_y].Item)) return;

	if (cur_y>=MAX_HISTORY_NUM-1)
	{
		cur_y/=2;
		for (i=0; i<cur_y; i++;)
		{
			strlcpy(#history_list[i].Item, #history_list[MAX_HISTORY_NUM-i].Item, sizeof(URL));
		}	
	}
	cur_y++;
	// history_list[i].was_first = WB1.list.first;
	strlcpy(#history_list[cur_y].Item, #URL, sizeof(URL));
	links_count=cur_y;
}


byte UrlsHistory::GoBack() {
	if (cur_y<=1) return 0;
	cur_y--;
	strlcpy(#URL, #history_list[cur_y].Item, sizeof(URL));
	// stroka = history_list[cur_y].was_first;
	return 1;
}


byte UrlsHistory::GoForward() {
	if (cur_y==links_count) return 0;
	cur_y++;
	strlcpy(#URL, #history_list[cur_y].Item, sizeof(URL));
	return 1;
}
