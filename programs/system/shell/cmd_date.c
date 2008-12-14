

void cmd_date()
{
unsigned date;
unsigned time;

date = kol_system_date_get();
printf("  date [dd.mm.yy]: %x%x.%x%x.%x%x", 
	(date&0xf00000)>>20, (date&0xf0000)>>16,  // day
	(date&0xf000)>>12, (date&0xf00)>>8, //month
	(date&0xf0)>>4, (date&0xf) ); // year


time = kol_system_time_get();

printf("\n\r  time [hh:mm:ss]: %x%x:%x%x:%x%x\n\r", 
	(time&0xf0)>>4, (time&0xf), // hours
	(time&0xf000)>>12, (time&0xf00)>>8, // minutes
	(time&0xf00000)>>20, (time&0xf0000)>>16 ); // seconds

}
