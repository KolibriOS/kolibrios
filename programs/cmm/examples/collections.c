#define MEMSIZE 4096*120

#include "../lib/io.h"
#include "../lib/collection.h"


void main()
{   
	io.run("/sys/develop/board", "");
	test_int();
	test_str();
	ExitProcess();
}

void test_int()
 collection_int ci=0;
 int i;
 {  
	ci.add(0);
	ci.add(1);
	ci.add(2);
	ci.add(3);
	debugln("-> 0 1 2 3");
	for (i=0; i<ci.count; i++) debugln(itoa(ci.get(i)));
	ci.pop();
	ci.pop();
	ci.add(4);
	debugln("-> 0 1 4");
	for (i=0; i<ci.count; i++) debugln(itoa(ci.get(i)));
	
	ci.set(1,9);
	debugln("-> 0 9 4");
	for (i=0; i<ci.count; i++) debugln(itoa(ci.get(i)));

	ci.set(6,6);
	debugln("-> 0 9 4 0 0 0 6");
	for (i=0; i<ci.count; i++) debugln(itoa(ci.get(i)));

	ci.swap(0,2);
	debugln("-> 4 9 0 0 0 0 6");
	for (i=0; i<ci.count; i++) debugln(itoa(ci.get(i)));
}

void test_str()
 collection s=0;
 {
	s.add("Hello");
	s.add("World");
	debugln(s.get(0)); //-> Hello
	debugln(s.get(1)); //-> World
	s.pop();
	debugln(s.get(0)); //-> Hello
	debugln(s.get(1)); //-> 0
	s.add("Kolibri");	
	debugln(s.get(0)); //-> Hello
	debugln(s.get(1)); //-> Kolibri
	s.drop();
}
