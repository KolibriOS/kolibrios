#include "func.h"
#include <math.h> // For fabs()

char debuf[50] = "";

double textwidth(const char* s, int len) {
	int i;
	for (i = 0; i < len; i++)
		if (s[i] == 0)
			break;
	return i * 6;
}

double textheight(const char* s, int len) {
	return 8.0;
}

int isalpha(char c) {
	return (c==' ' || c=='\n' || c=='\t' || c=='\r');
}

// эта функция - велосипед. но проще было написать чем найти.
double convert(const char* s, int* len) {
	int i;
	double sign, res, tail, div;
	res = 0.0;
	i = 0;
	while (s[i] && isalpha(s[i])) i++;
	if (len) *len=i;
	if (s[i] == '\0')
		return ERROR_END;

	sign=1.0;
	if (s[i] == '-') {
		sign=-1.0;
		i++;
	}
	while (s[i] && s[i] >= '0' && s[i] <= '9') {
		res *= 10.0;
		res += s[i] - '0';
		i++;
	}
	if (len) *len=i;
	if (!s[i] || isalpha(s[i]))
		return sign*res;
	if (s[i] != '.' && s[i] != ',')
		return ERROR;
	i++;
	if (len) *len=i;
	if (!s[i])
		return sign*res;

	div = 1.0;
	tail = 0.0;
	while (s[i] && s[i] >= '0' && s[i] <= '9') {
		tail *= 10.0;
		tail += s[i] - '0';
		div *= 10.0;
		i++;
	}
	res += tail/div;
	if (len) *len=i;
	return sign*res;
}

int isequal(double a, double b) {
	return fabs(a-b) < EQUALITY_VAL;
}