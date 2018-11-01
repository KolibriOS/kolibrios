
#ifndef INCLUDE_ARRAY_H
#include "../lib/array.h"
#endif

#define ECTRL 300

struct EVENTS
{
	Array array;
	int new_id;
	void init();
	int add();
	int add_n();
	bool press();
};

void EVENTS::init(dword size)
{
	array.init(size);
	new_id = 900;
}

int EVENTS::add(dword event1)
{
	new_id++;
	array.set(new_id, event1);
	return new_id;
}

int EVENTS::add_n(dword id, event2)
{
	array.set(id, event2);
	return new_id;
}

bool EVENTS::press(dword id)
{
	dword event3;
	if (array.get(id)) {
		event3 = array.get(id);
		event3();
		return true;
	}
	return false;
}