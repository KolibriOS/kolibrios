
int cmd_cd(char dir[])
{

char temp[256];
unsigned result;

if (NULL == dir)
	{
	printf("  cd directory\n\r");
	return FALSE;
	}

if ( 0 == strcmp(dir, ".") )
	return FALSE;

if (  ( 0 == strcmp(dir, "..") ) && ( 0 != strcmp(cur_dir, "/")) ) 
	{
	cur_dir[strlen(cur_dir)-1]='\0';
	dir_truncate(cur_dir);
	return FALSE;
	}

if ( '/' == dir[0])
	{
	if ( dir_check(dir) )
		{
		strcpy(cur_dir, dir);
		return TRUE;
		}
	return FALSE;
	}
else
	{
	strcpy(temp, cur_dir);
	strcat(temp, dir);

	if ( dir_check(temp) )
		{
		strcpy(cur_dir, temp);
		return TRUE;
		}

	return FALSE;
	}

}
