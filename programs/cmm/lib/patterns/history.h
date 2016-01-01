#include "..\lib\collection.h"

struct _history {
	collection items;
	int active;	
	int add();
	int back();
	int forward();
	dword current();
} history;

int _history::add(dword in)
{
	if (!strcmp(in, items.get(active-1))) return 0;
	items.count = active;
	items.add(in);
	active++;
	return 1;
}
	
int _history::back()
{
	if (active==1) return 0;
	active--;
	return 1;
}

int _history::forward()
{
	if (active==items.count) return 0;
	active++;
	return 1;
}

dword _history::current()
{
	return items.get(active-1);
}