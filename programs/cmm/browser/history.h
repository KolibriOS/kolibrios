ShowHistory()
{
		int i;
		static int history_pointer;
		int t;
		
		free(history_pointer);
		history_pointer = malloc(history.items.data_size+256);
		strcat(history_pointer, "<html><head><title>History</title></head><body><h1>History</h1>");
		strcat(history_pointer, "<h2>Visited pages</h2><blockquote><br>");
		for (i=0; i<history.items.count; i++)
		{
			strcat(history_pointer, " <a href='");
			strcat(history_pointer, history.items.get(i));
			strcat(history_pointer, "'>");
			strcat(history_pointer, history.items.get(i));
			strcat(history_pointer, "</a><br>");
		}
		strcat(history_pointer, "</blockquote><h2>Cached images</h2>");
		for (i=1; i<ImgCache.pics_count; i++)
		{
			strcat(history_pointer, "<img src='");
			strcat(history_pointer, #pics[i].path);
			strcat(history_pointer, "'><br>");
			strcat(history_pointer, #pics[i].path);
		}
		strcat(history_pointer, "</body></html>");
		WB1.LoadInternalPage(history_pointer, strlen(history_pointer));
}