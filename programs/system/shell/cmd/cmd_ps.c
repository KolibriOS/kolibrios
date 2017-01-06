
int cmd_ps(char param[])
{

int		i, n, sel;
char		*buf1k;
unsigned	PID;
short		STATE;

buf1k = malloc(1024);
if (NULL == buf1k)
	return FALSE;

sel = param && strlen(param) > 0;

for (i = 1;;i++)
	{
	n = kol_process_info(i, buf1k);
	PID = *(buf1k+30);
	STATE = *(buf1k+50);
	if (9 != STATE)
		{
		if (!sel || 0 == strnicmp(param, buf1k+10, 10))
			{
			printf ("  %7d %s\n\r", PID, buf1k+10);
			if (sel) 
				{
					LAST_PID = PID;
					int cpu_tck = kol_system_cpufreq() / 100;
					printf ("  CPU %d%% RAM %d\n\r", *(int*)buf1k / cpu_tck , *(int*)(buf1k+26)+1);
				}
			}
		}
	if (i == n)
		break;
	}

free(buf1k);
return TRUE;

}

