
int cmd_sleep(char param[])
{
int delay;

if (!strlen(param))
	{
	#if LANG_ENG
		printf("    sleep <time in the 1/100 of second>\n\r");
	#elif LANG_RUS
		printf("  sleep <интервал в сотых доля секунды>\n\r");
	#endif
	return TRUE;
	}

delay = _atoi(param);
kol_sleep((unsigned)delay);
return TRUE;
}

