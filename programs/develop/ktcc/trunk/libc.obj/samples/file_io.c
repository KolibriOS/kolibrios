#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_MAX 255

static char test_str1[] = "123454567890abcdefghijklmnopqrstufvwxyz";
static char test_str2[READ_MAX];

int main(int argc, char** argv)
{
    int i = 0;
    FILE* f;

    // write to file
    debug_printf("Write file...\n");
    f = fopen("testfile.txt", "w");

    while (test_str1[i] != 'a') {
        fputc(test_str1[i], f);
        i++;
    }
    fclose(f);

    // append to file
    debug_printf("Apend file...\n");
    f = fopen("testfile.txt", "a");
    fputs(test_str1 + i, f);
    char null_term = '\0';
    fwrite(&null_term, sizeof(char), 1, f);
    printf("Error: %s\n", strerror(errno));
    fclose(f);

    // copy from testfile.txt to copyfile.txt
    debug_printf("Read file...\n");
    f = fopen("testfile.txt", "r");
    i = 0;
    while ((test_str2[i] = fgetc(f)) != EOF && i < READ_MAX) {
        fputc(test_str2[i], stdout);
        i++;
    }
    printf("\n%s\n", test_str1);
    if (!strcmp(test_str2, test_str1)) {
        puts("TEST: OK!");
    } else {
        puts("TEST: FAIL!");
    }
    fclose(f);
}
