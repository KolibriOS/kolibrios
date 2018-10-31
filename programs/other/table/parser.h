
#pragma once

// error codes
#define ERR_BADFUNCTION -1
#define ERR_BADNUMER -2
#define ERR_GENERAL -3
#define ERR_NOBRACKET -4
#define ERR_BADVARIABLE -5
#define ERR_OVERFLOW -6
#define ERR_BADPARAM -7

typedef double variable_callback(char *s);

void set_exp(char *exp);
// puts the token back to line
void putback(double *hold);
// gets the expression. This function is used externally
int get_exp(double *hold);

// logic binary
void level1(double *hold);

// unary !
void level1_5(double *hold);

// works with +-
void level2(double *hold);
// works with */%
void level3(double *hold);
// works with ^
void level4(double *hold);
// works with ()
void level5(double *hold);
// works with elementary tokens
void level6(double *hold);
// gets value of number, function or variable
void primitive(double *hold);
// performs arithmetical operation
void arith(char op, double *r, double *h);

void logic(char *op, double *r, double *h);


// performs unary (one-operand) operation
void unary(char op, double *r);
// gets variable value by name
extern variable_callback *find_var;

extern double rand_seed;

// stops execution of parser and return error code
void serror(int code);
// checks the function table to see if such a function exists
int look_up(char *s);

bool strcmp(char *s1, char *s2);
bool strncmp(char *s1, char *s2, int n);

unsigned int chrnum(char* text, char symbol);

extern double epsilon;


int isdelim(char c);
int isdigit(char c);
int isalpha2(char c);
int iswhite(char c);




