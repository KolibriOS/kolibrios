

int script_check(char file[]) {
    kol_struct70	k70;
    char		buf[4];

    k70.p00 = 0;
    k70.p04 = 0;
    //k70.p08 = 0;
    k70.p12 = 4; // read 4 bytes
    k70.p16 = (unsigned) buf;
    k70.p20 = 0;
    k70.p21 = file;

    kol_file_70(&k70);

    if ( !strcmp(buf, script_sign) ) // if we found the script signature
        return TRUE;
    else
        return FALSE;
}


int script_run(char exec[], char args[]) {
    kol_struct70	k70;
    kol_struct_BDVK	bdvk;
    unsigned	result, i;
    unsigned	long long filesize, pos;
    char		*buf; // buffer, where script is copied

    k70.p00 = 5;
    k70.p04 = k70.p12 = 0;
    //k70.p08 = 0;
    k70.p16 = (unsigned) &bdvk;
    k70.p20 = 0;
    k70.p21 = exec;

    result = kol_file_70(&k70); // get file info
    if ( 0 != result ) 
        return FALSE;

    filesize = bdvk.p32; // get file size

    buf = malloc(filesize+256); // may fail for over 4Gb file, but impossible case
    if (NULL == buf)
        return FALSE;

    buf[filesize]=0;

    k70.p00 = 0;
    k70.p04 = 0;
    //k70.p08 = 0;
    k70.p12 = filesize;
    k70.p16 = (unsigned) buf;
    k70.p20 = 0;
    k70.p21 = exec;

    result = kol_file_70(&k70); // read file to the buffer
    if ( 0 != result ) 
        {
        free(buf);
        return FALSE;
        }

    pos = 0;

    for (;;) // script processing
    {
        if (pos > filesize)
            break;

        for (i=0;;i++) // reading a string
            {
            if ((0x0A == buf[pos])||(0x0D == buf[pos])||(0 == buf[pos]))
                {
                pos++;
                CMD[i] = '\0';
                break;
                }
            CMD[i] = buf[pos];
            pos++;
            }

        if ( 0 == strlen(CMD) ) // empty string
            continue;

        if ('#' == CMD[0]) // comment
            continue;

        command_execute();

    }

    free(buf);
    return TRUE;
}
