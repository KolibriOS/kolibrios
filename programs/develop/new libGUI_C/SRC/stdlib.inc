/*
	some standart libC helper functions
*/

static void* malloc(DWORD size)
{
	return(gui_ksys_malloc(size));
}

static void free(void *memory)
{
	gui_ksys_free(memory);
}

static void* realloc(void *old_mem,DWORD new_size)
{
	return(gui_ksys_realloc(new_size,old_mem));
}

static void	exit(int c)
{
	gui_ksys_exit(c);
}

