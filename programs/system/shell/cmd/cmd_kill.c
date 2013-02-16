

int cmd_kill(char process[])
{

unsigned proc;
int result;

if (NULL == process)
	{
	printf("  kill <PID>\n\r");
	return FALSE;
	}
else
	{
	proc = _atoi(process);
	if ( 0 != proc )
		{
		result = kol_process_kill_pid(proc);
		if (result < 0)
			return FALSE;
		else
			return TRUE;
		}
	}

}
