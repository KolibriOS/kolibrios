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

typedef double (*function_t)(double);
int isalpha(char c);
double convert(const char* s, int* len);

#endif /* FUNC_H */
