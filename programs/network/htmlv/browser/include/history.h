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
	if (!strcmp(get_URL_part(5),"http:"))) HttpLoad();
	WB1.ShowPage(#URL);
}

void UrlsHistory::AddUrl()
{
	if (strcmp(BrowserHistory.CurrentUrl(), #URL)==0) return; //если новый адресс = текущему
	
	IF (strlen(#UrlHistory)>6000) copystr(#UrlHistory+5000,#UrlHistory);
	copystr("|", #UrlHistory + strlen(#UrlHistory));
	copystr(#URL, #UrlHistory + strlen(#UrlHistory));
}

dword UrlsHistory::CurrentUrl()
{
	EAX=#UrlHistory + find_symbol(#UrlHistory, '|');
}
