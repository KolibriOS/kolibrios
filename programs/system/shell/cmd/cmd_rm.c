
int cmd_rm(char file[]) {

	kol_struct70	k70;
	char*		temp = (char*) malloc(FILENAME_MAX);
	unsigned	result;

	if (NULL == file || strlen(file) == 0) {
		printf (CMD_RM_USAGE);
		return TRUE;
	}

	if ( '/' == file[0])
		{
		strcpy(temp, file);

		if ( !file_check(temp) )
			{
			return FALSE;
			}
		}
	else 
		{
		strcpy(temp, cur_dir);
		if (temp[strlen(temp)-1] != '/') 
			strcat(temp, "/"); // add slash
		strcat(temp, file);
		
		if ( !file_check(temp) )
			{
			return FALSE;
			}
		}

	k70.p00 = 8;
	k70.p04 = 0;
	//k70.p08 = 0;
	k70.p12 = 0;
	k70.p16 = 0;
	k70.p20 = 0;
	k70.p21 = temp;

	result = kol_file_70(&k70);
  
  free(temp);

	if (0 == result)
		return TRUE;
	else 
		return FALSE;

}

