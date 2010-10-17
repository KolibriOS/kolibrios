
int cmd_help(char cmd[])
{

int i;

char available[]={"  %d commands available:\n\r"};

if ( !strlen(cmd) )
	{
	printf (available, NUM_OF_CMD);
	for (i = 0; i < NUM_OF_CMD; i++)
		printf("    %s\n\r", COMMANDS[i].name);
	return TRUE;
	}
else
	{
	for (i=0; i<NUM_OF_CMD; i++)
		if ( !strcmp(cmd, COMMANDS[i].name) )
			{
			printf(COMMANDS[i].help);
			return TRUE;
			}

	printf ("  Command \'%s\' not found.\n\r", cmd);
	printf (available, NUM_OF_CMD);
	for (i = 0; i < NUM_OF_CMD; i++)
		printf("    %s\n\r", COMMANDS[i].name);
	}

return FALSE;
}
