#ifndef FUNC_H
#define FUNC_H

#define ERROR 8888888888.9
#define ERROR_END 8888888888.7

extern char debuf[50];

typedef struct {
	double x, y;
} TCoord;

double textwidth(const char* s, int len);
double textheight(const char* s, int len);

int isalpha(char c);
double convert(const char* s, int* len);

static inline
int roundi(double a) {
	return round(a);
}

#define EQUALITY_VAL 0.000001
int isequal(double a, double b);

#endif /* FUNC_H */
