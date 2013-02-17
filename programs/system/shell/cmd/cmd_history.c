
int cmd_history(char arg[])
{

int i;

for (i = 0; i < CMD_HISTORY_NUM_REAL; i++)
	{
	printf("%s\n", CMD_HISTORY[i]);
	}

return TRUE;
}

