/*
	some standart libC helper functions
*/

static void* malloc(DWORD size);
static void free(void *memory);
static void* realloc(void *old_mem,DWORD new_size);
static void exit(int c);

