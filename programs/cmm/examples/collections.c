#define MEMSIZE 4096*120

#include "../lib/io.h"
#include "../lib/collection.h"

collection s;

void main()
{   
	io.run("/sys/develop/board", "");
	test1();
	test2();
}

void test1() {
	s.add("Hello");
	s.add("World!");
	debugln(s.get(0)); //-> Hello
	debugln(s.get(1)); //-> World
	s.drop();
}

void test2()
{   
	int i;
	s.add("0");
	s.add("1");
	s.add("2");
	s.add("3");
	for (i=0; i<s.count; i++) debugln(s.get(i)); //-> 0 1 2 3
	s.count--;
	s.count--;
	s.add("4");
	for (i=0; i<s.count; i++) debugln(s.get(i)); //-> 0 1 4
	s.drop();
}