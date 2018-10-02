/*
 * Template C-- program.
*/

#define MEMSIZE 4096*5

#include "../lib/kolibri.h"
#include "../lib/fs.h"

proc_info Form;

void main()
{
	RunProgram("/sys/syspanel", "/sys/settings/games.ini");
}

