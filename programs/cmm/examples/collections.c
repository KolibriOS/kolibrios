#define MEMSIZE 4096*10

#include "../lib/io.h"
#include "../lib/collection.h"

void main()
{   
	collection s;
	io.run("/sys/develop/board", "");
	s.init(4096);
	s.add("lorem");
	s.add("ipsum");
	s.add("1234566");
	debugln(s.get(0));
	debugln(s.get(1));
	debugln(s.get(2));
}