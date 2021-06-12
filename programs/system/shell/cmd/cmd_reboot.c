
int cmd_reboot(char param[]) {
	if (!strcmp(param, "kernel"))
		kol_system_end(4);
	else
		kol_system_end(3);
	return TRUE;
}

