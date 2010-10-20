

int _atoi ( char *s )
{
int i, n;
 
n = 0;
for ( i = 0; s[i]!= '\0'; ++i)
	if ((s[i]<'0') || (s[i]>'9'))
		return 0;
	else
		n = 10 * n + s[i] - '0';

return n;
}



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
