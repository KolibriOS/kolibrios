#include <dir.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int main()
{
    char *path=getcwd(NULL, PATH_MAX);
    printf("Current directory: %s\n", path); 
    if(true==mkdir("test")){
        puts("Test folder created!");
    }
    else{
        puts("Error creating folder!");
    }
    short_file_info *info;
    int num = lsdir(path, &info);
    if(num==FS_ERROR)
    {
        puts("File system error.");
        return -1;
    }
    printf("Objects in the folder: %d\n", num);
    for(int j=0; j<num; j++)
    {
        printf("%s ", info[j].name);
        if(info[j].type==T_FOLDER)
        {
            printf("(Folder)\n"); 
        }
        else
        {
            printf("(File)\n");
        }
    }
    free(info);
    setcwd("/sys");
    path=getcwd(NULL, PATH_MAX);
    printf("Move to the directory: %s\n", path);
    free(path);
}
