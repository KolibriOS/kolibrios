
int cmd_help(char cmd[]) {
	int i;

	if ( !strlen(cmd) ) {
		int columns_max=3;
		printf (CMD_HELP_AVAIL, NUM_OF_CMD);
		for (i = 0; i < NUM_OF_CMD; i++) {
			printf("    %-12s", COMMANDS[i].name);
			if ((i) && ((i+1)%columns_max == 0)) printf("\n\r");
		}
		if ((i)%columns_max != 0) printf("\n\r");
		return TRUE;
	}
	else {
		for (i=0; i<NUM_OF_CMD; i++)
			if ( !strcmp(cmd, COMMANDS[i].name) ) {
				printf(COMMANDS[i].help);
				return TRUE;
			}

		printf (CMD_HELP_CMD_NOT_FOUND, cmd);
		printf (CMD_HELP_AVAIL, NUM_OF_CMD);
		for (i = 0; i < NUM_OF_CMD; i++)
			printf("    %s\n\r", COMMANDS[i].name);
		}

	return FALSE;
}

