ShowHistory()
{
		int i;
		static int history_pointer;
		
		free(history_pointer);
		history_pointer = malloc(64000);
		strcat(history_pointer, " <title>History</title><h1>History</h1>");
		strcat(history_pointer, "<h2>Visited pages</h2><blockquote><br>");
		for (i=1; i<BrowserHistory.links_count; i++)
		{
			strcat(history_pointer, "<a href='");
			strcat(history_pointer, BrowserHistory.GetUrl(i));
			strcat(history_pointer, "'>");
			strcat(history_pointer, BrowserHistory.GetUrl(i));
			strcat(history_pointer, "</a><br>");
		}
		strcat(history_pointer, "</blockquote><h2>Cached images</h2><br>");
		for (i=1; i<ImgCache.pics_count; i++)
		{
			strcat(history_pointer, "<img src='");
			strcat(history_pointer, #pics[i].path);
			strcat(history_pointer, "' /><br>");
			strcat(history_pointer, #pics[i].path);
		}
		WB1.Prepare(history_pointer, strlen(history_pointer));
}