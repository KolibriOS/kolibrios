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
    double expected_d = 100.0;
    double result_d = strtod(start, &end);
    assert(result_d == expected_d);
    assert(!strcmp(end, " Rub"));

    end = NULL;
    float expected_f = 100.0f;
    float result_f = strtof(start, &end);
    assert(result_f == expected_f);
    assert(!strcmp(end, " Rub"));

    end = NULL;
    long double expected_ld = 100.0;
    long double result_ld = strtold(start, &end);
    assert(result_ld == expected_ld);
    assert(!strcmp(end, " Rub"));

    char* st3 = "21.3e3Hello World!";
    assert(atof(st3) == 21300.0);

    char* st4 = "12345";
    float fpart;
    int ipart;
    sscanf(st4, "%3f%d", &fpart, &ipart);
    assert(fpart == 123.0);
    assert(ipart == 45);

    char* st5 = "123.45";
    float fval;
    sscanf(st5, "%f", &fval);
    assert(fval == 123.45f);

    double dval;
    sscanf(st5, "%lf", &dval);
    assert(dval == 123.45);

    long double ldval;
    sscanf(st5, "%Lf", &ldval);
    assert(ldval == 123.45);

    float gval;
    sscanf(st5, "%g", &gval);
    assert(gval == 123.45f);

    float eval;
    sscanf(st5, "%e", &eval);
    assert(eval == 123.45f);

    /* Signed exponent must be parsed (regression guard for e+/e-). */
    assert(strtod("12e+3", NULL) == 12000.0);
    float epos;
    sscanf("12E+3", "%f", &epos);
    assert(epos == 12000.0f);
    double eneg = strtod("3000e-3", NULL);
    assert(eneg > 2.9 && eneg < 3.1);

    /* Uppercase float specifiers behave like lowercase in scanf. */
    float gup;
    assert(sscanf("123.45", "%G", &gup) == 1);
    assert(gup == 123.45f);

    /* No valid number: value 0, endptr unchanged, scanf reports no match. */
    char* sdot = ".";
    char* edot;
    assert(strtod(sdot, &edot) == 0.0);
    assert(edot == sdot);
    float fdot;
    assert(sscanf(".", "%f", &fdot) == 0);

    /* Malformed exponent: value ends before 'e', sign preserved. */
    char* sbad = "-1.5e";
    char* ebad;
    assert(strtod(sbad, &ebad) == -1.5);
    assert(*ebad == 'e');

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
