
void get_str_uptime(char *str, const char *fmt) {
	unsigned	time_tick, up_days, up_hours, up_minutes, up_seconds, up_millisecs;

	time_tick = kol_time_tick();
	up_days = (time_tick/(24*60*60*100));
	up_hours = (time_tick/(60*60*100))%24;
	up_minutes = (time_tick/(60*100))%60;
	up_seconds = (time_tick/100)%60;
	up_millisecs = (time_tick*10)%100;

	sprintf (str, fmt, up_days, up_hours, up_minutes, up_seconds, up_millisecs);
}

int cmd_uptime(char param[]) {
	get_str_uptime(tmpstr, CMD_UPTIME_FMT);
	printf(tmpstr);
	return TRUE;
}

