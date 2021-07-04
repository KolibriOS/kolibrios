#ifdef LANG_RUS
#define HISTORY_HEADER "<html><title>История</title><body><b>Посещенные страницы</b><br>"
#else
#define HISTORY_HEADER "<html><title>History</title><body><b>Visited pages</b><br>"
#endif


ShowHistory()
{
	int i;
	dword history_pointer = malloc(history.items.data_size+256);
	strcat(history_pointer, HISTORY_HEADER);

	for (i=0; i<history.items.count-1; i++) //if (cache.type.get(i) == PAGE)
	{
		strcat(history_pointer, "<a href='");
		strcat(history_pointer, history.items.get(i));
		strcat(history_pointer, "'>");
		strcat(history_pointer, history.items.get(i));
		strcat(history_pointer, "</a><br>");
	}

	/*
	strcat(history_pointer, "<br><b>Cached images</b><br>");
	for (i=1; i<cache.url.count; i++) if (cache.type.get(i) == IMG)
	{
		strcat(history_pointer, cache.url.get(i));
		strcat(history_pointer, " <img src='");
		strcat(history_pointer, cache.url.get(i));
		strcat(history_pointer, "'><br>");
	}
	*/

	LoadInternalPage(history_pointer, strlen(history_pointer));
	free(history_pointer);
}
