
#include "all.h"

int dir_check(char dir[])
/// just checks, if dir[] is really a directory
{
    kol_struct70	k70;
    int		result;

    k70.p00 = 1;
    k70.p04 = 0;
    //k70.p08 = 0;
    k70.p12 = 2; // enough to read . & ..
    k70.p16 = (unsigned)malloc(32+k70.p12*560);
    k70.p20 = 0;
    k70.p21 = dir;

    result = kol_file_70(&k70);

    free((void*)k70.p16);

    if ( (0 == result)||(6 == result) )  // 6 is possible ???
        return TRUE;
    else 
        return FALSE;

}

void dir_truncate(char dir[])
{
    int i;
    i = strlen(dir)-1;
    for (;;i--)
        if ('/' == dir[i])
            {
            dir[i+1] = 0;
            break;
            }
}

void get_file_dir_loc(char *filepath, char *dir_path)
{
    char *res = strrchr(filepath, '/');
    if (res == 0)
    {
        dir_path = '\0';
        return;
    }
    size_t pos = res - filepath;
    strncpy(dir_path, filepath, pos);
    dir_path[pos] = '\0';
}


int file_check(char file[])
{
    kol_struct70	k70;
    int		result;

    k70.p00 = 0;
    k70.p04 = 0;
    //k70.p08 = 0;
    k70.p12 = 0;
    k70.p16 = 0;
    k70.p20 = 0;
    k70.p21 = file;

    result = kol_file_70(&k70);

    if (0 == result)
        return TRUE;
    else 
        return FALSE;
}


void file_not_found(char file[]) {
    printf (FILE_NOT_FOUND_ERROR, file);
}


int iswhite(char c) {return ((' ' == c) || ('\t' == c) || (13 == c) || (10 == c)); }

void trim(char string[])
{
    int i, j;

    for (i=0; ;i++)
        if ( !iswhite(string[i]) )
            break;
    j = 0;
    for (;;i++, j++)
        {
        string[j] = string[i];
        if ('\0' == string[i] )
            break;
        }

    for (i=0; ;i++)
        if ('\0' == string[i])
            break;
    i--;
    for (;i>0;--i)
        if ( iswhite(string[i]) )
            string[i] = '\0';
        else
            break;
}


// entry point
int main(int argc, char **argv)
{
    int i; for (i = 1; i < argc; i++) {
        strcat(cmdline, argv[i]);
        if (i != argc - 1) {
            strcat(cmdline, " ");
        }
    }

    NUM_OF_CMD = sizeof(COMMANDS)/sizeof(COMMANDS[0]);
    strcpy(title, "SHELL ");
    strcat(title, SHELL_VERSION);
    con_init_opt(-1, -1, -1, -1, title);

    //printf("argc = %d\ncmdline = '%s'\n", argc, cmdline);

    if (sizeof (kol_struct70) != 25) {
        printf("Invalid struct align kol_struct70, need to fix compile options\n\r");
        kol_exit();
    }

    //strcpy(cur_dir, PATH);
    //dir_truncate(cur_dir);
    getcwd(cur_dir, sizeof cur_dir);
    //printf("curdir %s\n", cur_dir);

    con_set_cursor_height(con_get_font_height()-1);

    ALIASES = malloc(128*1024);

    if (!cmdline || cmdline[0] == 0) {
        strcpy(CMD, argv[0]);
        dir_truncate(CMD);
        strcat(CMD, ".shell");
        if ( !file_check(CMD) )
            strcpy(CMD, "/sys/settings/.shell");
    }
    else {
        if (cmdline[0] == '/')
        {
            strcpy(cur_dir, cmdline);
            *(strrchr(cur_dir, '/')+1)=0;
        }
        strcpy(CMD, cmdline);
    }

    command_execute();

    for (;;) {
        //printf("\033[32;1m"); 
        printf ("# ");
        //printf("\033[0m"); 
        command_get();
        command_execute();
    }

    con_exit(0);
    kol_exit();
}
