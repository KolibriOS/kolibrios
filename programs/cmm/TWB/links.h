
struct LinksArray
{
	char page_links[12000];
	void Add();
	dword GetURL();
	void Clear();
} PageLinks;

void LinksArray::Add(dword new_link)
{
	strcat(#page_links, new_link);
	strcat(#page_links, "|");
}

dword LinksArray::GetURL(int id)
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
	return #URL;
}

void LinksArray::Clear()
{
	strcpy(#page_links,"|");
}