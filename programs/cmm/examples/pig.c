/*
 * Template C-- program.
*/

#define MEMSIZE 4096*5

#include "../lib/kolibri.h"
#include "../lib/fs.h"
#include "../lib/patterns/restart_process.h"

proc_info Form;

void main()
{
	int i;
	CreateDir("/tmp0/1/pig");
	RunProgram("/sys/UNZ", "-o /tmp0/1/pig -h /kolibrios/games/pig.zip");
	for (i = 0; i < 200; i++)
	{
		if (CheckProcessExists("UNZ")==false) break;
		pause(3);
	}
	RunProgram("/tmp0/1/pig/pig", NULL);
}

