#include <limits.h>
#include <stdio.h>
#include <sys/ksys.h>

char* test_string1 = "Hello world!";
int a, b;

int main(int argc, char** argv)
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
