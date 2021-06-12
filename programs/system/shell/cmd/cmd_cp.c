
int cmd_cp(char param[])
{
    char* argv[100];
    int argc;
    char *filename_in = NULL;
    char *filename_out = NULL;
    char *buffer = NULL;

    kol_struct70 k70_in;
    kol_struct70 k70_out;

    kol_struct_BDVK	bdvk;

    unsigned long long filesize;
    unsigned result, buf_size;

    argc = parameters_prepare(param, argv);

    if (argc != 2)
    {
        printf(CMD_CP_USAGE);
        parameters_free(argc, argv);
        return TRUE;
    }

    filename_in  = (char*) malloc(FILENAME_MAX);
    filename_out = (char*) malloc(FILENAME_MAX);

    if (argv[0][0] != '/')
    {
        strcpy(filename_in, cur_dir);
        if (filename_in[strlen(filename_in)-1] != '/')
        {
            strcat(filename_in, "/"); // add slash
        }
        strcat(filename_in, argv[0]);
    } else
    {
        strcpy(filename_in, argv[0]);
    }
    // -----
    if (argv[1][0] != '/')
    {
        strcpy(filename_out, cur_dir);
        if (filename_out[strlen(filename_out)-1] != '/')
        {
            strcat(filename_out, "/"); // add slash
        }
        strcat(filename_out, argv[1]);
    } else
    {
        strcpy(filename_out, argv[1]);
    }
       
    // add ability to use directory as destination
    if ( dir_check(filename_out) )
    {
        char *fname = strrchr(filename_in, '/') + 1;  // always exist, as we add curdir
        if (filename_out[strlen(filename_out)-1] != '/')
        {
            strcat(filename_out, "/"); // add slash
        }
        strcat(filename_out, fname);
    }


    k70_in.p00 = 5;
    k70_in.p04 = 0LL;
    k70_in.p12 = 0;
    k70_in.p16 = (unsigned) &bdvk;
    k70_in.p20 = 0;
    k70_in.p21 = filename_in;

    result = kol_file_70(&k70_in); // get information about file
    if ( 0 != result )
    	goto lbl_exit;

    // count buffer size up to 1Mb, but no more than 1/2 of free memory
    buf_size = 1 << 20; // 1Mb
    while( ((buf_size >> 10) > kol_system_memfree()) && (buf_size > 4096) )
    	buf_size /= 2;

    filesize = bdvk.p32; // getting file size (restriction - 4 GB only for FAT)
    if (buf_size > filesize)
    	buf_size = (unsigned)filesize;	// may be zero!
    if (buf_size == 0) buf_size = 4096;	// ...

    buffer = (char*) malloc(buf_size);
    if (!buffer)
    {
        result = E_NOMEM;
        goto lbl_exit;
    }

    k70_in.p00 = 0;
    //k70_in.p08 = 0;
    k70_in.p12 = buf_size;
    k70_in.p16 = (unsigned) buffer;
    k70_in.p20 = 0;
    k70_in.p21 = filename_in;

    k70_out.p00 = 2;
    //k70_out.p08 = 0;
    k70_out.p12 = buf_size;
    k70_out.p16 = (unsigned) buffer;
    k70_out.p20 = 0;
    k70_out.p21 = filename_out;

    unsigned long long offset = 0;
    do
    {
        k70_in.p04 = offset;
        if (offset + buf_size > filesize)  // last chunk
        {
            k70_in.p12 = k70_out.p12 = (unsigned)(filesize - offset); // filesize % buf_size;
        }
        
        result = kol_file_70(&k70_in); // read
        if (result != 0)
        {
            goto lbl_exit;
        }

        k70_out.p04 = offset;
        result = kol_file_70(&k70_out); // write
        if (result != 0)
        {
           goto lbl_exit;
        }

        if (k70_out.p00 == 2)
        {
            k70_out.p00 = 3; // changing function from create (2) to append (3)
        }
        offset += buf_size;
    } while (offset < filesize);

    lbl_exit:

    parameters_free(argc, argv);
    free(filename_in);
    free(filename_out);
    free(buffer);

    return (result == 0);
}

