#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/dir.h>
#include <sys/dirent.h>

const char* folder_type = "Folder";
const char* file_type = "File";

int main()
{
    char* path = getcwd(NULL, PATH_MAX);
    printf("Current directory: %s\n", path);
    if (mkdir("test")) {
        puts("Test folder created!");
    } else {
        puts("Error creating folder!");
    }

    DIR* mydir = opendir(path);
    if (!mydir) {
        puts("File system error.");
        return -1;
    }

    struct dirent* file_info;
    char* str_type = NULL;
    putc(' ');
    while ((file_info = readdir(mydir)) != NULL) {
        if (file_info->d_type == IS_FOLDER) {
            (*con_set_flags)(CON_COLOR_GREEN);
            str_type = (char*)folder_type;
        } else {
            (*con_set_flags)(7);
            str_type = (char*)file_type;
        }
        printf("%3d  %20s  %s\n ", file_info->d_ino, file_info->d_name, str_type);
    };

    setcwd("/sys/develop");
    path = getcwd(NULL, PATH_MAX);
    printf("Move to the directory: %s\n", path);
    free(path);
}
