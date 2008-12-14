
int cmd_help(char cmd[])
{

int i;

char available[]={"  %d commands available:\n\r"};

if (NULL == cmd)
	{
	printf (available, NUM_OF_CMD);
	for (i = 0; i < NUM_OF_CMD; i++)
		printf("    %s\n\r", HELP_COMMANDS[i]);
	}
else
	{
	for (i=0; i<NUM_OF_CMD; i++)
		if ( !strcmp(cmd, HELP_COMMANDS[i]) )
			{
			printf(HELP_DESC[i]);
			return TRUE;
			}

	printf ("  Command \'%s\' not found.\n\r", cmd);
	printf (available, NUM_OF_CMD);
	for (i = 0; i < NUM_OF_CMD; i++)
		printf("    %s\n\r", HELP_COMMANDS[i]);
	}

return FALSE;
}
