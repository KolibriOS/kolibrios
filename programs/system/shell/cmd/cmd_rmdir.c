
int cmd_rmdir(char dir[]) {
	char*		temp = (char*) malloc(FILENAME_MAX);
	kol_struct70	k70;
	unsigned	result;

	if (NULL == dir || strlen(dir) == 0) {
		printf(CMD_RMDIR_USAGE);
		return TRUE;
	}

	if ( ( 0 == strcmp(dir, ".") ) || ( 0 == strcmp(dir, "..") ) || ( 0 == strcmp(cur_dir, "/")) )  {
		return FALSE;
	}

	k70.p00 = 8;
	k70.p04 = 0;
	//k70.p08 = 0;
	k70.p12 = 0;
	k70.p16 = 0;
	k70.p20 = 0;

	if ( '/' == dir[0])
		k70.p21 = dir;
	else {
		strcpy(temp, cur_dir);
		if (temp[strlen(temp)-1] != '/') 
			strcat(temp, "/"); // add slash
		strcat(temp, dir);
		k70.p21 = temp;
	}

	if ( !dir_check(temp) )
		return FALSE;

	result = kol_file_70(&k70);

  free(temp);
  
	if (0 == result)
		return TRUE;
	else 
		return FALSE;

}

