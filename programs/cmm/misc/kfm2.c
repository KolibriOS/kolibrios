#define MEMSIZE 1024*20
#define ENTRY_POINT #main

#include "../lib/fs.h"

void main()
{
	RenameMove("KFM", abspath("EOLITE"));
	RunProgram(abspath("KFM"), #param);
	RenameMove("EOLITE", abspath("KFM"));
	ExitProcess();
}
