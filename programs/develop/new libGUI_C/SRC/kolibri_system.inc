/*
	some system function of KolibriOS and founded of them functions
*/

static DWORD gui_get_file_size(char *filename,DWORD *buf_for_size)
{
	static char		buf[44];
	static fileio_t f;
	bded_t		*bded;
	DWORD		status,value;

	f.number_subfunction=5;
	f.data=(DWORD*)buf;
	f.full_file_path=filename;

	status=gui_ksys_files_io(&f,value);

	if (status==KOLIBRIOS_SYS_FILE_ACCESS_SUCCESSFULLY)
	{
		bded=(bded_t*)buf;
		*buf_for_size=bded->file_size_low;
		buf_for_size++;
		*buf_for_size=bded->file_size_hight;
	}

	return(status);
}

static DWORD gui_read_file(char *filename,DWORD *buf_pos_size,DWORD size_read,char *buf)
{
	static fileio_t f;
	DWORD		status,value;

	f.number_subfunction=0;
	f.offset_in_file_low=(DWORD)*buf_pos_size;buf_pos_size++;
	f.offset_in_file_hight=(DWORD)*buf_pos_size;
	f.size=size_read;
	f.data=(DWORD*)buf;
	f.full_file_path=filename;

	status=gui_ksys_files_io(&f,value);

	return(status);
}

static void gui_debug_out_str(char *s)
{

	while(*s)
	{
		if (*s=='\n') gui_ksys_debug_out(13);

		gui_ksys_debug_out(*s);
		s++;
	}
}

static void* gui_cofflib_getproc(import_t *lib, char *name)
{
	int i;

	for(i = 0; lib[i].name && strcmp(name, lib[i].name); i++);

	if(lib[i].name) return lib[i].data;
			else	return NULL;
}

