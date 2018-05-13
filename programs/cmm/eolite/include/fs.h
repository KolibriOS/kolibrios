BDVK file_info_count;
int file_count_copy;

void DirFileCount(dword way)
{
	dword dirbuf, fcount, i, filename;
	dword cur_file;
	if (dir_exists(way))
	{
		cur_file = malloc(4096);
		// In the process of recursive descent, memory must be allocated
		// dynamically, because the static variable cause a fault!!!
		// But unfortunately pass away to sacrifice speed.
		GetDir(#dirbuf, #fcount, way, DIRS_ONLYREAL);
		filename = dirbuf+72;
		for (i=0; i<fcount; i++)
		{
			filename += 304;
			sprintf(cur_file,"%s/%s",way,filename);
			
			if (TestBit(ESDWORD[filename-40], 4) )
			{
				file_count_copy++;
				DirFileCount(cur_file);
			}
			else
			{
				file_count_copy++;
			}
		}
		free(cur_file);
	}
}

