#include<stdio.h>
int main() {
	int t1 = 176401255;
	float f = 0.25f;
	int t2a = (int)(t1 * f); // must be 44100313
	int t2b = (int)(t1 * (float)0.25f);
	printf("t2a=%d t2b=%d \n",t2a,t2b);
	return 0;
}
