#include <stdlib.h>
#include <stdio.h>

void f()
{
    static int c = 1;
    printf("exit #%d\n", c);
    c++;
}

int main()
{
    atexit(&f);
    atexit(&f);
    atexit(&f);

    return 0;
}
