
int cmd_ps(char param[])
{

int		i, n;
char		*buf1k;
unsigned	PID;
short		STATE;

buf1k = malloc(1024);
if (NULL == buf1k)
	return FALSE;

for (i = 1;;i++)
	{
	n = kol_process_info(i, buf1k);
	PID = *(buf1k+30);
	STATE = *(buf1k+50);
	if ( 9 != STATE)
		printf ("  %7d %s\n\r", PID, buf1k+10);
	if (i == n)
		break;
	}

free(buf1k);
return TRUE;

}
