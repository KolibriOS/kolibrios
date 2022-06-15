/*
	noscript.c

	Empty function definitions to satisfy dZ80 when compiling it without script support
*/

#include <stdio.h>

#include "dissz80p.h"

int	dZ80_LoadConfiguration(DISZ80 *d, char *pConfigFile)
{
	(void)d;
	(void)pConfigFile;
	return DERR_SCRIPTING_NA;
}

int	InitOpcodeTraps(DISZ80 *d)
{
	(void)d;
        return DERR_NONE;
}

int	ShutdownScripting(DISZ80 *d)
{
	(void)d;
	return DERR_NONE;
}


int ExecPreTrap(DISZ80 *d)
{
	(void)d;
        return 0;
}

void ExecPostTrap(DISZ80 *d)
{
	(void)d;
	return;
}

