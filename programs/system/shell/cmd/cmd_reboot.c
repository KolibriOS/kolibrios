
int cmd_reboot(char param[])
{
if (!strcmp(param, "kernel"))
	{
	kol_system_end(4);
	return TRUE;
	}
else
	{
	kol_system_end(3);
	return TRUE;
	}
}
