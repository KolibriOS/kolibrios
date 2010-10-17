
int cmd_free(char param[])
{
unsigned total, free, used;

total = kol_system_mem();
free = kol_system_memfree();
used = total - free;

printf ("  total [kB / MB / %%]:  %-7d / %-5d / 100\n\r   free [kB / MB / %%]:  %-7d / %-5d / %d\n\r   used [kB / MB / %%]:  %-7d / %-5d / %d\n\r", 
		total, total/1024, free, free/1024, (free*100)/total, used, total/1024-free/1024, 100-(free*100)/total );
		
return TRUE;		
}
