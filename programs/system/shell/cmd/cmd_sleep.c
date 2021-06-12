
int cmd_sleep(char param[]) {
	int delay;
	if (!strlen(param)) {
		printf(CMD_SLEEP_USAGE);
		return TRUE;
	}
	delay = atoi(param);
	kol_sleep((unsigned)delay);
	return TRUE;
}

