#include <math.h>
#include <stdio.h>

int fun_sum(int *num, int len)
{
	int sum=0;
	for(int i=0; i<len; i++){
		sum+=num[i]; 
	}
	return sum;
}

int main() 
{
	printf("Enter 5 numbers separated by spaces:\n");
	int num[5];
	scanf("%d %d %d %d %d", &num[0], &num[1], &num[2], &num[3], &num[4]);
	printf("%d+%d+%d+%d+%d=%d\n\n", num[0], num[1], num[2], num[3], num[4], fun_sum(num, 5));

	puts("Math testing:\n");
	printf("sqrt(25) = %f\n", sqrt(25));
	printf("pow(2, 2) = %f\n", pow(2, 2));
	return 0;
}