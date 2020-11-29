#include <dir.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
int main()
{
    char *path=getcwd(NULL, PATH_MAX);
    printf("Current directory: %s\n", path); 
    setcwd("/sys"); //String must be null terminated!
    path=getcwd(NULL, PATH_MAX);
    printf("Move to the directory: %s\n", path);
    free(path);
    if(true==mkdir("TEST1")){
        puts("Test folder created!");
        return(0);
    }
    else{
        puts("Error creating folder!");
        return(-1);
    }
}
