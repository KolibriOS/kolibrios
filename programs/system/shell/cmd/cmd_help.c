
int cmd_help(char cmd[])
{

int i;

#if LANG_ENG
	char available[]={"  %d commands available:\n\r"};
#elif LANG_RUS
	char available[]={"  Количество доступных команд: %d\n\r"};
#endif

if ( !strlen(cmd) )
	{
	int columns_max=3;
	printf (available, NUM_OF_CMD);
	for (i = 0; i < NUM_OF_CMD; i++)
	{
		printf("    %-12s", COMMANDS[i].name);
		if ((i) && ((i+1)%columns_max == 0)) printf("\n\r");
	}
	if ((i)%columns_max != 0) printf("\n\r");
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

	#if LANG_ENG
		printf ("  Command \'%s\' not found.\n\r", cmd);
	#elif LANG_RUS
		printf ("  Команда \'%s\' не найдена.\n\r", cmd);
	#endif
	printf (available, NUM_OF_CMD);
	for (i = 0; i < NUM_OF_CMD; i++)
		printf("    %s\n\r", COMMANDS[i].name);
	}

return FALSE;
}

