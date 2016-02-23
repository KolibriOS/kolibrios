#define MEMSIZE 4096*120

#include "../lib/io.h"
#include "../lib/collection.h"


void main()
{   
	io.run("/sys/develop/board", "");
	test1();
	test2();
}

void test1()
 collection s;
 {
	s.add("Hello");
	s.add("World!");
	debugln(s.get(0)); //-> Hello
	debugln(s.get(1)); //-> World
	s.drop();
}

void test2()
 collection_int ci;
 int i;
 {   
	ci.add(0);
	ci.add(1);
	ci.add(2);
	ci.add(3);
	for (i=0; i<ci.count; i++) debugi(ci.get(i)); //-> 0 1 2 3
	ci.count--;
	ci.count--;
	ci.add(4);
	for (i=0; i<ci.count; i++) debugi(ci.get(i)); //-> 0 1 4
	ci.drop();
}