#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ksys.h>

char* test_string1 = "Hello world!";
int a, b;

int base_test()
{
    sscanf("43 53", "%d %d", &a, &b);
    printf("(43 53) = (%d %d)\n", a, b);
    printf("Hello world! = %s\n", test_string1);
    printf("345.358980 = %f\n", 345.35898);
    printf("345 = %d\n", (int)345.35898);
    printf("ff = %x\n", 255);
    printf("-1 = %d\n", UINT_MAX);
    printf("5A-4B-N$ = %s%c-%u%c-N%c\n", "5", 'A', 4, 'B', '$');
    puts("Done!");
    return 0;
}

int fgets_test()
{
    char buf[8];
    for (size_t i = 0; i < 4; i++) {
        printf("input: ");
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            return 1;
        }
        printf("got: \"%s\"\n", buf);
    }

    return 0;
}

int fgets_file()
{
    FILE* f = fopen("/kolibrios/develop/tcc/samples/stdio_test.c", "r");

    if (!f) {
        return 1;
    }

    char buff[16];

    while (fgets(buff, sizeof(buff), f) != NULL) {
        printf("%s", buff);
    }

    return 0;
}

typedef int (*func_t)();

const static func_t tests[] = {
    base_test,
    fgets_file,
    fgets_test
};

int main(int argc, char** argv)
{
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        if (tests[i]()) {
            printf("Fail test %u\nerr: %s\n", i, strerror(errno));
            return 1;
        }
        printf("Tests passed: %d/%d\n", i+1, sizeof(tests) / sizeof(tests[0]));
    }
    return 0;
}
