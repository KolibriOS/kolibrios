
int cmd_waitfor(char param[])
// waits for LASTPID or pid in param
{

int		i, n, sel, sel_pid;
char		*buf1k;
unsigned	PID;
short		STATE;

sel = param && strlen(param) > 0;
sel_pid = LAST_PID;
if (sel) 
	{
	sel_pid = _atoi(param);
	}
if (0 == sel_pid)
	return FALSE;

	#if LANG_ENG
		printf("  Awaing finish PID %d\n\r", sel_pid);
	#elif LANG_RUS
		printf("  Ожидаем завершения PID %d\n\r", sel_pid);
	#endif

buf1k = malloc(1024);
if (NULL == buf1k)
	return FALSE;

while(1)
	{
	for (i = 1;;i++)
		{
		n = kol_process_info(i, buf1k);
		PID = *(buf1k+30);
		STATE = *(buf1k+50);
		if (PID == sel_pid)
			if(9 == STATE)
				goto exit_normal;
			else break;
		if (i == n)
			goto exit_normal;
		}
	kol_sleep(10); // 100ms
	}

exit_normal:
free(buf1k);
return TRUE;

}

