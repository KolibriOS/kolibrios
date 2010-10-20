
int cmd_help(char cmd[])
{

int i;

#if LANG_ENG
	char available[]={"  %d commands available:\n\r"};
#elif LANG_RUS
	char available[]={"  Кол-во доступных команд: %d\n\r"};
#endif

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
