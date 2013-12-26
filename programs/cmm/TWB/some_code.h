
enum { BACK=300, FORWARD, REFRESH, HOME, NEWTAB, GOTOURL, SEARCHWEB, INPUT_CH, INPUT_BT };
enum { _WIN, _DOS, _KOI, _UTF };

#define ID1         178
#define ID2         177

                      

dword get_URL_part(int len) {
	char temp1[sizeof(URL)];
	strcpy(#temp1, #URL);
	temp1[len] = 0x00;
	return #temp1;
}

inline byte chTag(dword text) {return strcmp(#tag,text);}


void GetURLfromPageLinks(int id)
{
	int i, j = 0;
	for (i = 0; i <= id - 401; i++)
	{
		do
		{
			j++;
			if (j>=strlen(#page_links)) return;
		}
		while (page_links[j] <>'|');
	}
	page_links[j] = 0x00;
	strcpy(#URL, #page_links+strrchr(#page_links, '|'));
}