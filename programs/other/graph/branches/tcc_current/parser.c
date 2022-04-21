#include <math.h>
#include <string.h>

#include <sys/ksys.h>

#include "func.h"
#include "parser.h"

#define nullptr 0

// token types
#define DELIMITER 1
#define VARIABLE 2
#define NUMBER 3
#define FUNCTION 4
#define FINISHED 10

// error codes
#define ERR_BADFUNCTION -1
#define ERR_BADNUMBER -2
#define ERR_GENERAL -3
#define ERR_NOBRACKET -4
#define ERR_BADVARIABLE -5
#define ERR_OVERFLOW -6

//I guess it's not the best idea but we need pointers to functions anyway
double sin_p(double d) {
	return sin(d);
}

double cos_p(double d) {
	return cos(d);
}

double exp_p(double d) {
	return exp(d);
}

double sqrt_p(double d) {
	return sqrt(d);
}

double log_p(double d) {
	return log(d);
}

double tan_p(double d) {
	return tan(d);
}

double ctg(double d) {
	return 1 / tan(d);
}

double asin_p(double d) {
	return asin(d);
}

double acos_p(double d) {
	return acos(d);
}

double atan_p(double d) {
	return atan(d);
}

double fabs_p(double d) {
	return fabs(d);
}

double sgn(double d) {
	if (d < 0) return -1;
	return 1;
}

double arcctg(double d) {
	return atan(1 / d);
}

double sec(double d) {
	return 1 / sin(d);
}

double cosec(double d) {
	return 1 / cos(d);
}

double log2_p(double d) {
	return log2(d);
}

double log10_p(double d) {
	return log10(d);
}

double log3(double d) {
	return log(d) / log(3);
}

// represents general mathematical function
typedef double(*matfunc)(double);

// used to link function name to the function
typedef struct {
	char name[10];
	matfunc f;
} func;

// the list of functions
#define MAX_FUNCS 26
func functions[MAX_FUNCS] = {
	"", (matfunc)nullptr,
	"sin", (matfunc)&sin_p,
	"cos", (matfunc)&cos_p,
	"exp", (matfunc)&exp_p,
	"sqrt", (matfunc)&sqrt_p,
	"log", (matfunc)&log_p,
	"tg", (matfunc)&tan_p,
	"tan", (matfunc)&tan_p,
	"ctg", (matfunc)&ctg,
	"cot", (matfunc)&ctg,
	"arcsin", (matfunc)&asin_p,
	"arccos", (matfunc)&acos_p,
	"arctg", (matfunc)&atan_p,
	"arctan", (matfunc)&atan_p,
	"abs", (matfunc)&fabs_p,
	"sgn", (matfunc)&sgn,
	"arcctg", (matfunc)&arcctg,
	"arccot", (matfunc)&arcctg,
	"sec", (matfunc)&sec,
	"cosec", (matfunc)&cosec,
	"lb", (matfunc)&log2_p,
	"log2", (matfunc)&log2_p,
	"lg", (matfunc)&log10_p,
	"log10", (matfunc)&log10_p,
	"log3", (matfunc)&log3,
	"ln", (matfunc)&log_p
};

// all delimiters
const char* delim="+-*^/%=;(),><";		// not bad words

// structure for most parser functions
char token[80];
int token_type;
char* prog;
double x_value;
int code;    // error code

int isdelim(char c) {
	//return strchr(delim, c) != 0;
	for (int i = 0; i < 14; i++)
		if (c == delim[i])
			return 1;
	return 0;
}

int isdigit(char c) {
	return (c >= '0' && c <= '9');
}

int isalpha2(char c) {
	return ((c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z'));
}

int iswhite(char c) {
	return (c==' ' || c=='\t');
}

void serror(int code) {
	/*switch (code) {
		case ERR_BADFUNCTION:
			_ksys_debug_puts("ERR_BADFUNCTION\n");
			break;
		case ERR_BADNUMBER:
			_ksys_debug_puts("ERR_BADNUMBER\n");
			break;
		case ERR_NOBRACKET:
			_ksys_debug_puts("ERR_NOBRACKET\n");
			break;
		case ERR_BADVARIABLE:
			_ksys_debug_puts("ERR_BADVARIABLE\n");
			break;
		case ERR_OVERFLOW:
			_ksys_debug_puts("ERR_OVERFLOW\n");
			break;
	}*/
	//TODO (vitalkrilov): for what?: "::code = code;"
	// longjmp(j, code);
}

void set_exp(char* exp, double x) {
	prog = exp;
	x_value = x;
}

int get_token() {
	int tok;
	char* temp;
	(token_type) = 0;
	tok = 0;
	temp = (token);

	if (*(prog) == '\0') {
		*(token) = 0;
		tok = FINISHED;
		return ((token_type) = DELIMITER);
	}
	while (iswhite(*(prog))) ++(prog);
	if (isdelim(*(prog))) {
		*temp = *(prog);
		(prog)++;
		temp++;
		*temp = 0;
		return ((token_type) = DELIMITER);
	}
	if (isdigit(*(prog))) {
		while (!isdelim(*(prog)))
			*temp++=*(prog)++;
		*temp = '\0';
		return ((token_type) = NUMBER);
	}
	if (isalpha2(*(prog))) {
		while (!isdelim(*(prog)))
			*temp++=*(prog)++;
		(token_type) = VARIABLE;
	}
	*temp = '\0';
	if ((token_type) == VARIABLE) {
		tok = look_up((token));
		if (tok)
			(token_type) = FUNCTION;
	}
	return (token_type);
}

int sign(double d) {
  if (d > 0.0)
    return 1.0;
  if (d < 0.0)
    return -1.0;
  return 0.0;
}

void putback() {
	char* t;
	t = (token);
	for (;*t;t++)
		(prog)--;
}

int get_exp(double* hold) {
  int res;
  code = 0;
//  if (res = setjmp(j) != 0)
//    return code;
	get_token();
	if (!*(token)) {
		return 0;
	}
	level2( hold);
	putback();
  return 0;
}

void level2(double* hold) {
	char op;
	double h;

	level3( hold);
	while ((op=*(token)) == '+' || op == '-') {
		get_token();
		level3( &h);
		arith(op, hold, &h);
	}
}

void level3(double* hold) {
	char op;
	double h;

	level4( hold);
	while ((op=*(token)) == '*' || op == '/' || op == '%') {
		get_token();
		level4( &h);
		arith( op, hold, &h);
	}
}

void level4(double* hold) {
	double h;
	level5( hold);

	if (*(token) == '^') {
		get_token();
		level5( &h);
		arith( '^', hold, &h);
	}
}

void level5(double* hold) {
	char op;

	op = 0;
	if (((token_type) == DELIMITER) && *(token) == '+' || *(token) == '-') {
		op = *(token);
		get_token();
	}
	level6( hold);

	if (op)	unary(op, hold);
}

void level6(double* hold) {
	if ((*(token) == '(') && ((token_type) == DELIMITER)) {
		get_token();
		level2( hold);
		if (*(token) != ')')
      serror( ERR_NOBRACKET);
		get_token();
	}
	else
		primitive( hold);
}

void calc_function(double* hold) {
	double d;
	int i;

	i = look_up(token);

	if (i == 0)
		serror(ERR_BADFUNCTION);	// error

	get_token();
	if (*(token) != '(')
		serror(ERR_NOBRACKET);	// error
	get_token();
	level2(hold);
	get_token();

  d = functions[i].f(*hold);
  *hold = (functions[i].f)(*hold);
//  else
//    serror( ERR_OVERFLOW);

}

void primitive(double* hold) {
	switch (token_type) {
		case VARIABLE:
			*hold = find_var(token);
			get_token();
			return;
		case NUMBER:
	    	//*hold = atof((token));
	    	//if (sscanf(token, "%lf", hold) != 1)
				*hold = convert(token, nullptr);
			if (*hold == ERROR)
				serror( ERR_BADNUMBER);
			get_token();
			return;
		case FUNCTION:
			calc_function( hold);
			return;
		default:	// error
			return;
	}
}

void arith(char op, double* r, double* h) {
	double t;
	switch(op) {
		case '-':
			*r = *r - *h;
			break;
		case '+':
			*r = *r + *h;
			break;
		case '*':
			*r = *r * *h;
			break;
		case '/':
	    if (*h == 0)
	      serror( ERR_OVERFLOW);
	    else
			  *r = (*r) / (*h);
			break;
		case '%':
	    if (*h == 0)
	      serror( ERR_OVERFLOW);
	    else
			*r = fmod(*r, *h);
			if (*r < 0) *r += *h;
			break;
		case '^':
			*r = pow(*r, *h);
			break;
	}
}

void unary(char op, double* r) {
	if (op == '-')
		*r = -(*r);
}

double find_var(const char* s) {
	//return 0;	// not imp
	//int i;

	//for (i = 0; i < kvar; i++)
	//	if (strcmp(vars[i].name, s) == 0)
	//		return vars[i].value;

	if (s[1] == '\0' && (s[0] == 'x' || s[0] == 'X'))
	//if (strcmp(s,"x") == 0 || strcmp(s,"X") == 0)
		return x_value;

	serror( ERR_BADVARIABLE);
  return 0; // to make compiler very happy

	//printf("\nPlease enter value for variable \"%s\": ", s);
	//scanf("%lf", &vars[kvar].value);
	//strcpy(vars[kvar].name, s);
	//kvar++;
	//return vars[kvar - 1].value;
}

int look_up(const char* s) {
	int i;

	for (i = 0; i < MAX_FUNCS; i++)
		if (strcmp(s, functions[i].name) == 0)
			return i;
	return 0;	// search command/function name
}
