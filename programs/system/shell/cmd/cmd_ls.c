
int cmd_ls(char dir[]) {

	kol_struct70	k70;
	unsigned	*n;
	unsigned	num_of_file; // number of files in directory
	unsigned	*t;
	unsigned	type_of_file; // check is this a file or a folder
	int		i, result;
	char* tmp = (char*) malloc(FILENAME_MAX);

	bool single_column_mode = FALSE;


	k70.p00 = 1;
	k70.p04 = 0;
	//k70.p08 = 0;
	k70.p12 = 2;  // just for test exist & read number of entries
	k70.p16 =  (unsigned) malloc(32+k70.p12*560);
	k70.p20 = 0;


	if (!strnicmp(dir,"-1",1)) 
		{
		single_column_mode = TRUE;
		dir += 3;
		}

	if ( !strlen(dir) ) // if argument is empty, list current directory
		k70.p21 = cur_dir;
	else
	{
		if (dir[0] != '/') // if given directory is relative path, then append cur_dir on left side
		{
			strcpy(tmp, cur_dir);
			if (tmp[strlen(tmp)-1] != '/')
			{
				strcat(tmp, "/"); // add slash
			}
			strcat(tmp, dir);
			k70.p21 = tmp;
		} else // if given directory is an absolute path
		{
			k70.p21 = dir;
		}
	}

	result = kol_file_70(&k70);
	if ( !((result==0) || (result==6)) ) // check does the directory exists
		{
		free( (void*) k70.p16);
		return FALSE;
		}

	n =  (unsigned*) (k70.p16+8);
	num_of_file = *n;

	// now read full directory
	k70.p12 = num_of_file;  
	free( (void*) k70.p16);
	k70.p16 =  (unsigned) malloc(32+k70.p12*560);
	if ( !k70.p16 )
		return FALSE;
		
	result = kol_file_70(&k70);
	if ( !((result==0) || (result==6)) ) 
		{
		free( (void*) k70.p16);
		return FALSE;
		}

	// if there was '-1' param, then show single column mode
	// otherwise show files in several columns
	if (single_column_mode == TRUE)
	{
	_SINGLE_COLUMN_MODE:
		for (i = 0; i < num_of_file; i++)
		{
		printf ("  %s", k70.p16+32+40+(264+40)*i);
		t =  (unsigned*) (k70.p16+32+(264+40)*i);
		type_of_file = *t;
		if ( (0x10 == (type_of_file&0x10)) || (8 == (type_of_file&8)) )
			printf ("/");
		printf ("\n\r");
		}
	}
	else
	{
		int longest_name_len = 0;
		int console_window_width = 78; //need to get this value from console.obj if it's possible
		for (i = 0; i < num_of_file; i++)
			{
			int current_name_len;
			current_name_len = strlen( (char*)k70.p16+32+40+(264+40)*i);
			if (current_name_len > longest_name_len) longest_name_len = current_name_len;
			}

		longest_name_len+=2; //consider space separator and '/' symbol for folders
		int columns_max = console_window_width / longest_name_len;

		if (longest_name_len >= console_window_width) goto _SINGLE_COLUMN_MODE; //there was too long filename

		for (i = 0; i < num_of_file; i++)
			{
			char cur_file[2048];
			strncpy(cur_file, (char*)k70.p16+32+40+(304)*i, sizeof(cur_file)-2);

			t =  (unsigned*) (k70.p16+32+(304)*i);
			type_of_file = *t;
			
			int is_folder = 0;
			if ( (0x10 == (type_of_file&0x10)) || (8 == (type_of_file&8)) ) { is_folder = 1; strcat(cur_file, "/"); }

			if (is_folder) { printf("\033[0;36m"); } // set cyan for folder
			printf ("%*s", -longest_name_len, cur_file);
			if (is_folder) { printf("\033[0m"); } // is had been set, reset

			if ((i>0) && ((i+1)%columns_max == 0)) printf ("\n\r");
			}	
		if ((i)%columns_max != 0) printf("\n\r");
	}

	free((void*)k70.p16);
  free(tmp);
	return TRUE;
}

