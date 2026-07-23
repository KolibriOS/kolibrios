#pragma once

// error codes
#define ERR_BADFUNCTION -1
#define ERR_BADNUMER    -2
#define ERR_GENERAL     -3
#define ERR_NOBRACKET   -4
#define ERR_BADVARIABLE -5
#define ERR_OVERFLOW    -6
#define ERR_BADPARAM    -7

// number parser, shared with calc.c (accepts '.' or ',' as the decimal point)
#define ERROR     -1 // convert(): bad character
#define ERROR_END -2 // convert(): empty / no number
extern int convert_error;
double convert(char *s, int *len);

typedef double variable_callback(char *s);

// resolves a variable name to its value; set by the host before get_exp()
extern variable_callback *find_var;

// set the expression to evaluate
void set_exp(char *exp);
// evaluate it; writes result to *hold, returns nonzero on success
int get_exp(double *hold);
// raise a parser error (stores the code)
void serror(int code);

// count occurrences of symbol in text
unsigned int chrnum(char *text, char symbol);
// char classifiers (named to avoid clashing with <ctype.h> macros)
int isdigit2(char c);
int isalpha2(char c);
