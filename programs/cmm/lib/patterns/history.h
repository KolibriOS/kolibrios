#include "..\lib\collection.h"

struct _History {
	collection items;
	int active;	
	int add();
	int back();
	int forward();
	dword current();
} History;

int _History::add(dword in)
{
	if (!strcmp(in, items.get(active-1))) return 0;
	items.count = active;
	items.add(in);
	active++;
	return 1;
}
	
int _History::back()
{
	if (active==1) return 0;
	active--;
	return 1;
}

int _History::forward()
{
	if (active==items.count) return 0;
	active++;
	return 1;
}

dword _History::current()
{
	return items.get(active-1);
}