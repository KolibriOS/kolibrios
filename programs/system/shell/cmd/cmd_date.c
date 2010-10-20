

int cmd_date(char param[])
{
unsigned date;
unsigned time;

date = kol_system_date_get();
#if LANG_ENG
	printf("  Date [dd.mm.yy]: %x%x.%x%x.%x%x", 
#elif LANG_RUS
	printf("  „ â  [¤¤.¬¬.££]: %x%x.%x%x.%x%x", 
#endif
	(date&0xf00000)>>20, (date&0xf0000)>>16,  // day
	(date&0xf000)>>12, (date&0xf00)>>8, //month
	(date&0xf0)>>4, (date&0xf) ); // year


time = kol_system_time_get();

#if LANG_ENG
	printf("\n\r  Time [hh:mm:ss]: %x%x:%x%x:%x%x\n\r", 
#elif LANG_RUS
	printf("\n\r  ‚à¥¬ï [çç:¬¬:áá]: %x%x:%x%x:%x%x\n\r", 
#endif
	(time&0xf0)>>4, (time&0xf), // hours
	(time&0xf000)>>12, (time&0xf00)>>8, // minutes
	(time&0xf00000)>>20, (time&0xf0000)>>16 ); // seconds

return TRUE;
}
