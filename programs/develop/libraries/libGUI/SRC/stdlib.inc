/*
        some standart libC functions
*/
/*
static void* malloc(DWORD size)
{
        return dlmalloc(size);
}

static void free(void *memory)
{
        dlfree(memory);
}

static void* realloc(void *old_mem,DWORD new_size)
{
        return dlrealloc(old_mem,new_size);
}
*/

static void     exit(int c)
{
        gui_ksys_exit(c);
}
