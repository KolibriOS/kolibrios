#define MEMSIZE 4096*120

#include "../lib/io.h"
#include "../lib/collection.h"

void main()
{   
	collection s;
	int i;
	io.run("/sys/develop/board", "");
	s.add("Hello");
	s.add("World!");
	debugln(s.get(0));
	debugln(s.get(1));
	s.drop();
}