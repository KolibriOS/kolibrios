
int cmd_memory(char param[])
{
	unsigned total, free, used;

	total = kol_system_mem();
	free = kol_system_memfree();
	used = total - free;

	printf (CMD_FREE_FMT, 
		total, total/1024, free, free/1024, (free*100)/total, used, total/1024-free/1024, 100-(free*100)/total );
			
	return TRUE;		
}

