/*
 * PIGEX - PIG Game extractor
 * We need this app because PIG can not be run from CD-drive
*/

#define MEMSIZE 4096*5

#include "../lib/kolibri.h"
#include "../lib/fs.h"
#include "../lib/patterns/restart_process.h"

void main()
{
	int i;

	if (! file_exists("/tmp0/1/pig/pig")) {
		RunProgram("/sys/UNZ", "-o \"/tmp0/1\" -h \"/kolibrios/games/pig/pig.zip\"");
		for (i = 0; i < 200; i++)
		{
			if (CheckProcessExists("UNZ")==false) break;
			pause(3);
		}		
	}

	RunProgram("/tmp0/1/pig/pig", NULL);
}

