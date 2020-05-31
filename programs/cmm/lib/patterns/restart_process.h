#ifndef INCLUDE_RESTART_PROCESS_H
#define INCLUDE_RESTART_PROCESS_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

enum {
	MULTIPLE,
	SINGLE
};

#define MAX_PROCESS_COUNT 256

:bool CheckProcessExists(dword proc_name) {
	int i;
	proc_info Process;
	for (i=0; i<MAX_PROCESS_COUNT; i++;)
	{
		GetProcessInfo(#Process, i);
		if (strcmpi(#Process.name, proc_name)==0) return 1;
	}
	return 0;
}

:void KillProcessByName(dword proc_name, byte multiple) {
	int i;
	proc_info Process;
	for (i=0; i<MAX_PROCESS_COUNT; i++;)
	{
		GetProcessInfo(#Process, i);
		if (strcmpi(#Process.name, proc_name)==0) 
		{ 
			KillProcess(Process.ID); 
			if (multiple==SINGLE) break;
		}
	}
}

:int GetProcessesCount(dword proc_name) {
	int i, count=0;
	proc_info Process;
	for (i=0; i<MAX_PROCESS_COUNT; i++;)
	{
		GetProcessInfo(#Process, i);
		if (strcmpi(#Process.name, proc_name)==0) count++;
	}
	return count;
}

:void RestartProcessByName(dword proc_name, byte multiple) {
	KillProcessByName(proc_name + strrchr(proc_name, '/'), multiple);
	RunProgram(proc_name, "");	
}

:void RestartAllProcess() {
	int i;
	proc_info Process, Self;
	GetProcessInfo(#Self, -1);
	for (i=0; i<MAX_PROCESS_COUNT; i++;)
	{
		GetProcessInfo(#Process, i);
		if (Process.name) 
		&& (!streq(#Process.name, "OS"))
		&& (Process.ID != Self.ID)
			KillProcess(Process.ID);
	}
	RunProgram("/sys/launcher", NULL);
}

#endif