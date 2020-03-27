#ifdef LANG_RUS
#define HISTORY_HEADER "<html>
<head>
	<title>История</title>
</head>
<body>
	<h1>История</h1>
	<br>
	<b>Посещенные страницы</b><br>
"
#else
#define HISTORY_HEADER "<html>
<head>
	<title>History</title>
</head>
<body>
	<h1>History</h1>
	<br>
	<b>Visited pages</b><br>
"
#endif


ShowHistory()
{
	int i;
	static int history_pointer;
	int t;
	
	free(history_pointer);
	history_pointer = malloc(history.items.data_size+256);
	strcat(history_pointer, HISTORY_HEADER);
	for (i=0; i<history.items.count-1; i++)
	{
		strcat(history_pointer, "\t<a href='");
		strcat(history_pointer, history.items.get(i));
		strcat(history_pointer, "'>");
		strcat(history_pointer, history.items.get(i));
		strcat(history_pointer, "</a><br>");
	}
	/*
	strcat(history_pointer, "<br><b>Cached images</b><br>");
	for (i=1; i<ImgCache.pics_count; i++)
	{
		strcat(history_pointer, "<a href='");
		strcat(history_pointer, #pics[i].path);
		strcat(history_pointer, "'>");
		strcat(history_pointer, #pics[i].path);
		strcat(history_pointer, "</a><br>");
		
		// strcat(history_pointer, "<img src='");
		// strcat(history_pointer, #pics[i].path);
		// strcat(history_pointer, "'><br>");
		// strcat(history_pointer, #pics[i].path);
	}
	*/
	strcat(history_pointer, "</body></html>");
	LoadInternalPage(history_pointer, strlen(history_pointer));
}