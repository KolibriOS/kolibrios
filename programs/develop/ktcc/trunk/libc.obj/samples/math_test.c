#include <math.h>
#include <stdio.h>

int main()
{
    int i;
    for (i = 0; i < 20; i++) {
        printf("------------------------------------------------------\n");
        // printf ( "remainder of 5.3 / 2 is %f\n", remainder (5.3,2) );
        // printf ( "remainder of 18.5 / 4.2 is %f\n", remainder (18.5,4.2) );
        // remainder of 5.3 / 2 is -0.700000
        // remainder of 18.5 / 4.2 is 1.700000

        printf("fmod of 5.3 / 2 is %f\n", fmod(5.3, 2));
        printf("fmod of 18.5 / 4.2 is %f\n", fmod(18.5, 4.2));
        // fmod of 5.3 / 2 is 1.300000
        // fmod of 18.5 / 4.2 is 1.700000

        double param, fractpart, intpart, result;
        int n;

        param = 3.14159265;
        fractpart = modf(param, &intpart);
        printf("%f = %f + %f \n", param, intpart, fractpart);
        // 3.141593 = 3.000000 + 0.141593

        param = 0.95;
        n = 4;
        result = ldexp(param, n);
        printf("%f * 2^%d = %f\n", param, n, result);
        // 0.950000 * 2^4 = 15.200000

        param = 8.0;
        result = frexp(param, &n);
        printf("%f = %f * 2^%d\n", param, result, n);
        // 8.000000 = 0.500000 * 2^4
        param = 50;
        result = frexp(param, &n);
        printf("%f = %f * 2^%d\n", param, result, n);
    }
}
