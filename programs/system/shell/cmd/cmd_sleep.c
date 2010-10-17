
int cmd_sleep(char param[])
{
int delay;

if (!strlen(param))
	return FALSE;

delay = _atoi(param);
kol_sleep((unsigned)delay);
return TRUE;
}
