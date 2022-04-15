#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>
#include <time.h>

int comp(void* a, void* b)
{
    return *(int*)a - *(int*)b;
}

int main()
{
    puts("Start testing.");
    assert(NULL == ((void*)0));
    assert(RAND_MAX == 65535);
    assert(min(3, 10) == 3);
    assert(max(3, 10) == 10);
    assert(atof("12.4") == 12.4);
    assert(atoi("-123") == -123);
    assert(atol("-2146483647") == -2146483647L);
    assert(atoll("-9223372036854775806") == -9223372036854775806LL);
    assert(!strcmp("123", "123"));

    char st1[32];
    itoa(-2341, st1);
    assert(!strcmp(st1, "-2341"));

    assert(strlen("12345") == 5);
    assert(abs(4) == 4);
    assert(abs(-4) == 4);
    assert(labs(1000000000) == 1000000000);
    assert(labs(-1000000000) == 1000000000);
    assert(llabs(100000000000) == 100000000000);
    assert(llabs(-100000000000) == 100000000000);

    div_t output1 = div(27, 4);
    assert(output1.quot == 6);
    assert(output1.rem == 3);

    ldiv_t output2 = ldiv(27, 4);
    assert(output2.quot == 6);
    assert(output2.rem == 3);

    lldiv_t output3 = lldiv(27, 4);
    assert(output3.quot == 6);
    assert(output3.rem == 3);

    char* st2 = malloc(sizeof(char) * 2);
    assert(st2 != NULL);
    st2[0] = 'H';
    st2[1] = 'i';
    st2 = realloc(st2, sizeof(char) * 3);
    st2[2] = '!';
    assert(!strcmp(st2, "Hi!"));
    free(st2);

    st2 = calloc(2, sizeof(char));
    assert(st2 != NULL);
    st2[0] = 'H';
    st2[1] = 'i';
    assert(!strcmp(st2, "Hi"));
    free(st2);

    char* start = "100.00 Rub";
    char* end;
    assert(strtol(start, &end, 10) == 100L);
    assert(!strcmp(end, ".00 Rub"));

    end = NULL;
    assert(strtod(start, &end) == 100.0);
    assert(!strcmp(end, " Rub"));

    char* st3 = "21.3e3Hello World!";
    assert(atof(st3) == 21300.0);

    int nums[10] = { 5, 3, 9, 1, 8, 4, 2, 0, 7, 6 };
    qsort(nums, 10, sizeof(int), (int (*)(const void*, const void*))comp);
    for (int i = 0; i < 10; i++) {
        assert(nums[i] == i);
    }

    time_t libc_time = time(NULL);
    struct tm* libc_tm = localtime(&libc_time);
    printf(asctime(libc_tm));

    puts("End testing.");
    exit(0);
}
