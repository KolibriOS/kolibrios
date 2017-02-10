

int cmd_cd(char dir[])
{

char temp[256];
unsigned result;

if (NULL == dir)
	{
	#if LANG_ENG
		printf("  cd <directory>\n\r");
	#elif LANG_RUS
		printf("  cd <директория>\n\r");
	#endif
	return TRUE;
	}

if ( 0 == strcmp(dir, ".") )
	return FALSE;

if (  ( 0 == strcmp(dir, "..") ) && ( 0 != strcmp(cur_dir, "/")) )
	{
	cur_dir[strlen(cur_dir)-1]='\0';
	dir_truncate(cur_dir);
    set_cwd(cur_dir);
	return TRUE;
	}

if ( '/' == dir[0])
	{
	if ( dir_check(dir) )
		{
		strcpy(cur_dir, dir);
        set_cwd(cur_dir);
		return TRUE;
		}
	return FALSE;
	}
else
	{
	strcpy(temp, cur_dir);
	if (cur_dir[strlen(cur_dir)-1] != '/')
		strcat(temp, "/");
	strcat(temp, dir);

	if ( dir_check(temp) )
		{

		strcpy(cur_dir, temp);
        set_cwd(cur_dir);
		return TRUE;
		}

	return FALSE;
	}

}

