

int newline_to_file(char *fn, unsigned long long pos)
{
	char *newline = "\n\r";

	kol_struct70 k70_out;

	k70_out.p00 = 3;
	k70_out.p04 = pos; // offset 
    //k70_out.p08 = 0;
    k70_out.p12 = 2;
    k70_out.p16 = (unsigned)newline;
    k70_out.p20 = 0;
    k70_out.p21 = fn;

    return kol_file_70(&k70_out); // write
}


int cmd_echo(char text[])
{ // added output redirection by rgimad 2020.

    int text_len = strlen(text);
    int out_len = text_len;
    int i, redirect = 0, redirect_mode = 0; // if redirect = 0 echo to screen, if 1 to file. If redirect_mode = 0 rewrite the file, if 1 append to the file
    char *filename; // pointer to name of the file redirect to
    int ignore_redir_char = 0;

    for (i = 0; i < text_len; i++)
    {
    	if (text[i] == '"')
    	{
    		ignore_redir_char = !ignore_redir_char;
    		continue;
    	}
    	if (!ignore_redir_char && text[i] == '>')
    	{
    		if (i + 1 < text_len && text[i + 1] == '>')
    		{
    			redirect = 1;
    			redirect_mode = 1;
    			filename = text + i + 2;
    			out_len = i;
    			break;
    		} else
    		{
    			redirect = 1;
    			redirect_mode = 0;
    			filename = text + i + 1;
    			out_len = i;
    			break;
    		}
    	}
    }

    // remove leading spaces in filename
    while (*filename == ' ') { filename++; }

    // remove spaces at the end of out_len
    while (out_len > 0 && text[out_len - 1] == ' ') { out_len--; }

    // delete quotes if has
    if (text[out_len - 1] == '"') { out_len--; }
    if (text[0] == '"') { text++; out_len--; }


    if (redirect == 0) // echo to screen
    {
    	text[out_len] = '\0';
    	printf("%s\n\r", text);
    } else
    {
	    char *filename_out = (char*) malloc(FILENAME_MAX); // abs path

	    if (filename[0] != '/')
	    {
	        strcpy(filename_out, cur_dir);
	        if (filename_out[strlen(filename_out)-1] != '/')
	        {
	            strcat(filename_out, "/"); // add slash
	        }
	        strcat(filename_out, filename);
	    } else
	    {
	        strcpy(filename_out, filename);
	    }

	    kol_struct70 k70_out, k70_in;
	    int result;

    	if (redirect_mode == 0) // rewrite the output file
    	{

    		k70_out.p00 = 2;
    		k70_out.p04 = 0; // offset
		    //k70_out.p08 = 0;
		    k70_out.p12 = out_len;
		    k70_out.p16 = (unsigned)text;
		    k70_out.p20 = 0;
		    k70_out.p21 = filename_out;

		    result = kol_file_70(&k70_out); // write

	        if (result != 0) // unable to write
	        {
	        	free(filename_out);
	        	return FALSE;
	        }
	        newline_to_file(filename_out, out_len);

	        free(filename_out);
	        return TRUE;
    	} else // append to the output file
    	{
    		kol_struct_BDVK	bdvk;

    		k70_in.p00 = 5;
		    k70_in.p04 = 0LL;
		    k70_in.p12 = 0;
		    k70_in.p16 = (unsigned) &bdvk;
		    k70_in.p20 = 0;
		    k70_in.p21 = filename_out;

		    result = kol_file_70(&k70_in); // get information about file
		    if ( 0 != result ) // file doesnt exist, then rewrite
		    {
		    	k70_out.p00 = 2;
	    		k70_out.p04 = 0; // offset
			    //k70_out.p08 = 0;
			    k70_out.p12 = out_len;
			    k70_out.p16 = (unsigned)text;
			    k70_out.p20 = 0;
			    k70_out.p21 = filename_out;

			    result = kol_file_70(&k70_out); // write
		        if (result != 0) // unable to write
		        {
		        	free(filename_out);
		        	return FALSE;
		        }
		        newline_to_file(filename_out, out_len);

		        free(filename_out);
		        return TRUE;
		    }
		    // if exists, append
		    unsigned long long filesize = bdvk.p32;

		    k70_out.p00 = 3;
    		k70_out.p04 = filesize; // offset 
		    //k70_out.p08 = 0;
		    k70_out.p12 = out_len;
		    k70_out.p16 = (unsigned)text;
		    k70_out.p20 = 0;
		    k70_out.p21 = filename_out;

		    result = kol_file_70(&k70_out); // write
	        if (result != 0) // unable to write
	        {
	        	free(filename_out);
	        	return FALSE;
	        }

	        newline_to_file(filename_out, filesize + out_len);
	        free(filename_out);
	        return TRUE;
    	}
    }
    //free(filename_out);
    return TRUE;
}

