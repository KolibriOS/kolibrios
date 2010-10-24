
int cmd_killall(char process_name[])
{

unsigned	i;
if (!strlen(process_name))
	{
	for (i = 2;i<256;i++)
		{
		kol_kill_process(i);
		}
	return TRUE;
	}
return TRUE;
}