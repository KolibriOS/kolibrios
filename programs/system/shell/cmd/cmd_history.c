
int cmd_history(char arg[])
{

int i;

for (i = CMD_HISTORY_NUM_REAL - 1; i >= 0; i--)
	{
	printf("%s\n", CMD_HISTORY[i]);
	}

return TRUE;
}

