#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ksys.h>

int main(int argc, char** argv)
{
    char hello1[] = "Hello, KolibriOS!";
    char hello2[20];
    memcpy(hello1, hello2, strlen(hello1));
    if (!__strcmp(hello1, hello2)) {
        printf("memcpy: Successfully!\n");
        return 0;
    } else {
        printf("memcpy: Failure\n");
        return -1;
    }
}
