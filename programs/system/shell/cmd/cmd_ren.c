
int cmd_ren(char param[]) {
	char* argv[100];
    int argc;
    /*
	argv[0] - path (abs or rel) to file
	argv[1] - new filename
    */

    argc = parameters_prepare(param, argv);
    if (argc != 2) {
        printf(CMD_REN_USAGE);
        parameters_free(argc, argv);
        return TRUE;
    }
    //char *x;
    // argv[1] must be file name, not path 
    if (strrchr(argv[1], '/') != NULL) {
    	//printf("%d %s", x, argv[1]);
    	return FALSE;
    }

    char *new_filename  = (char*)malloc(FILENAME_MAX); new_filename[0] = '\0';

    get_file_dir_loc(argv[0], new_filename);
    if (strlen(new_filename) > 0)
    {
    	strcat(new_filename, "/");
    }
    strcat(new_filename, argv[1]);

    char *mv_params = (char*)malloc(FILENAME_MAX*2 + 1); mv_params[0] = '\0';
    strcat(mv_params, argv[0]);
    strcat(mv_params, " ");
    strcat(mv_params, new_filename);

    //printf("(%s)\n", mv_params);
    int res = cmd_mv(mv_params);

    free(new_filename);
    free(mv_params);

    return res;
}
