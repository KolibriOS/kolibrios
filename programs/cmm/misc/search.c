#define MEMSIZE 1024*20
#define ENTRY_POINT #main

#include "../lib/fs.h"

void main()
{
	char param2[4096];
	strcpy(#param2, "\\s ");
	strncpy(#param2 + 3, #param, sizeof(param2) - 2 - 3);
	RunProgram("/sys/File managers/Eolite", #param2);
	ExitProcess();
}
