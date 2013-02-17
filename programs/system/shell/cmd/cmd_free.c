
int cmd_memory(char param[])
{
unsigned total, free, used;

total = kol_system_mem();
free = kol_system_memfree();
used = total - free;

#if LANG_ENG
	printf ("  Total [kB / MB / %%]:  %-7d / %-5d / 100\n\r   Free [kB / MB / %%]:  %-7d / %-5d / %d\n\r   Used [kB / MB / %%]:  %-7d / %-5d / %d\n\r", 
#elif LANG_RUS
	printf ("  Всего        [КБ / МБ / %%]:  %-7d / %-5d / 100\n\r  Свободно     [КБ / МБ / %%]:  %-7d / %-5d / %d\n\r  Используется [КБ / МБ / %%]:  %-7d / %-5d / %d\n\r", 
#endif
		total, total/1024, free, free/1024, (free*100)/total, used, total/1024-free/1024, 100-(free*100)/total );
		
return TRUE;		
}

