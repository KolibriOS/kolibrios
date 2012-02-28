struct UrlsHistory {
	byte UrlHistory[6000];	
	void AddUrl();
	void GoBack();
	dword CurrentUrl();
};

UrlsHistory BrowserHistory;

void UrlsHistory::GoBack()
{
	j = find_symbol(#UrlHistory, '|') -1; //текущая страница
	if (j<=0) return;
	UrlHistory[j] = 0x00;
	j = find_symbol(#UrlHistory, '|'); //предыдущая страница -> она нам и нужна
	copystr(#UrlHistory + j, #URL);
	copystr(#URL, #editURL);
	WB1.ShowPage(#URL);
}

void UrlsHistory::AddUrl()
{
	IF (strlen(#UrlHistory)>6000) copystr(#UrlHistory+5000,#UrlHistory);
	copystr("|", #UrlHistory + strlen(#UrlHistory));
	copystr(#URL, #UrlHistory + strlen(#UrlHistory));
}

dword UrlsHistory::CurrentUrl()
{
	EAX=#UrlHistory + find_symbol(#UrlHistory, '|');
}
