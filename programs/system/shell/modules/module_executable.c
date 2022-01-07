
int executable_run(char cmd[], char args[]) {
    char	exec[FILENAME_MAX];
    int		result;

    if ( '/' == cmd[0]) // if path is absolute
    {
        strcpy(exec, cmd);
        if (!file_check(exec) ) // check file existense
        {
            file_not_found(cmd);
            return FALSE;
        }
    } else {
        strcpy(exec, cur_dir); // check file in current directory
        if (exec[strlen(exec)-1] != '/') 
            strcat(exec, "/"); // add slash
        strcat(exec, cmd);
        
        if ( !file_check(exec) ) // check file existense
        {
            strcpy(exec, "/sys/"); // check file on virtual disk
            strcat(exec, cmd);
            if ( !file_check(exec) ) // check file existense
            {
                file_not_found(cmd);
                return FALSE;
            }
        }
    }

    // if file exists:

    // try to run as a program
    result = program_run(exec, args);
    if (result > 0) {
        if ( !program_console(result)  ) {
            LAST_PID = result;
            printf (EXEC_STARTED_FMT, cmd, result);
        }
        return TRUE;
    } else	{
        if ( script_check(exec) ) // if file is a valid script
        {
            return script_run(exec, args);
        } else
        {
            printf (EXEC_SCRIPT_ERROR_FMT, cmd);
            return FALSE;
        }
    }

}

