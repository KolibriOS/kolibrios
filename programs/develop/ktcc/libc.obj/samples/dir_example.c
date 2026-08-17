#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/dirent.h>

const char folder_type[] = "Folder";
const char file_type[] = "File";

const char green[] = "\033[32m";
const char normal[] = "\033[0m";

int main()
{
    char* path = getcwd(NULL, PATH_MAX);
    printf("Current directory: %s\n", path);
    if (!mkdir("test")) {
        puts("Test folder created!");
    } else {
        puts("Error creating folder!");
    }

    DIR* mydir = opendir(path);
    if (!mydir) {
        printf("File system error: %s.\n", strerror(errno));
        return -1;
    }

    struct dirent* file_info;
    putc(' ');
    while ((file_info = readdir(mydir)) != NULL) {
        char* str_type[max(sizeof(green), sizeof(normal)) + max(sizeof(folder_type), sizeof(file_type))];
        if (file_info->d_type == IS_FOLDER) {
            strcpy(str_type, green);
            strcat(str_type, folder_type);
        } else {
            strcpy(str_type, normal);
            strcat(str_type, file_type);
        }
        printf("%s%3d  %20s  %s\n ", normal, file_info->d_ino, file_info->d_name, str_type);
    };
    puts(normal);

    setcwd("/sys/develop");
    path = getcwd(NULL, PATH_MAX);
    printf("Move to the directory: %s\n", path);
    free(path);
    return 0;
}
