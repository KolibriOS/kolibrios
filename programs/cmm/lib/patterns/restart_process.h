#ifndef INCLUDE_RESTART_PROCESS_H
#define INCLUDE_RESTART_PROCESS_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

enum {
	MULTIPLE,
	SINGLE
};

void RestartProcessByName(dword proc_name, byte multiple) {
	int i;
	proc_info Process;
	for (i=0; i<1000; i++;)
	{
		GetProcessInfo(#Process, i);
		if (strcmpi(#Process.name, proc_name)==0) 
		{ 
			KillProcess(Process.ID); 
			if (multiple==SINGLE) break;
		}
	}
	RunProgram(proc_name, "");	
}

#endif