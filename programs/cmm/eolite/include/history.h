
struct _FoldersHistory {
	collection history;
	int history_current;	
	int add();
	int back();
	int forward();
} FoldersHistory;

int _FoldersHistory::add()
{
	if (!strcmp(#path, history.get(history_current-1))) return 0;
	history.count = history_current;
	history.add(#path);
	history_current++;
	return 1;
}
	
int _FoldersHistory::back()
{
	if (history_current==1) return 0;
	history_current--;
	strcpy(#path, history.get(history_current-1));
	debugln(#path);
	return 1;
}

int _FoldersHistory::forward()
{
	if (history_current==history.count) return 0;
	history_current++;
	strcpy(#path, history.get(history_current-1));
	debugln(#path);
	return 1;
}