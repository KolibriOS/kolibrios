
int cmd_uptime(char param[])
{
unsigned	time_tick, up_days, up_hours, up_minutes, up_seconds, up_millisecs;

time_tick = kol_time_tick();
up_days = (time_tick/(24*60*60*100));
up_hours = (time_tick/(60*60*100))%24;
up_minutes = (time_tick/(60*100))%60;
up_seconds = (time_tick/100)%60;
up_millisecs = (time_tick*10)%100;

#if LANG_ENG
	printf ("  Uptime: %d day(s), %d:%d:%d.%d\n\r", up_days, up_hours, up_minutes, up_seconds, up_millisecs);
#elif LANG_RUS
	printf ("  Uptime: %d дней, %d:%d:%d.%d\n\r", up_days, up_hours, up_minutes, up_seconds, up_millisecs);
#endif
return TRUE;
}
