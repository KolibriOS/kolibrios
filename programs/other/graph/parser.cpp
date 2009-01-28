

#include "func.h"
#include "parser.h"

// token types
#define DELIMITER 1
#define VARIABLE 2
#define NUMBER 3
#define FUNCTION 4
#define FINISHED 10

// error codes
#define ERR_BADFUNCTION -1
#define ERR_BADNUMER -2
#define ERR_GENERAL -3
#define ERR_NOBRACKET -4
#define ERR_BADVARIABLE -5
#define ERR_OVERFLOW -6

double tg(double d)
{
	return sin(d) / cos(d);
}

double ctg(double d)
{
	return cos(d) / sin(d);
}

double exp(double x)
{
	__asm {
		fld	x 
		FLDL2E
		FMUL 

		FLD st(0) 

		FLD1

		FXCH 
		FPREM 
		F2XM1 
		fadd 
		FSCALE
		FSTP st(1)
	}
	
}

double log(double x)
{
	//return 0.0;
	__asm {
		FLD1
		FLD     x
		FYL2X
		FLDLN2
		FMUL
	}
}

double sqrt(double x)
{
	__asm {
		fld x
		fsqrt
	}
}

double atan(double x)
{
	return 0.0; // в лом
}

double pow(double x, double y)
{
	return 0.0; // в лом, пускай считают черех exp и log
}


// represents general mathematical function
typedef double(*matfunc)(double);

// used to link function name to the function
typedef struct  
{
	char name[10];
	matfunc f;
} func;

// the list of functions
const int max_func = 12;
func functions[max_func] = 
{
	"", NULL,
	"sin", &sin,
	"cos", &cos,
	"exp", &exp,
	"sqrt", &sqrt,
	"log", &log,
	"tg", &tg,
	"ctg", &ctg,
	"arcsin", &asin,
	"arccos", &acos,
	"arctg", &atan,
	"abs", &fabs
};

// all delimiters
const char *delim="+-*^/%=;(),><";		// not bad words

// structure for most parser functions

	char token[80];
	int token_type;
	char *prog;
	double x_value;
  int code;    // error code

int isdelim(char c)
{
	//return strchr(delim, c) != 0;
	for (int i = 0; i < 14; i++)
		if (c == delim[i])
			return 1;
	return 0;
}

int isdigit(char c)
{
	return (c >= '0' && c <= '9');
}

int isalpha2(char c)
{
	return ((c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z'));
}

int iswhite(char c)
{
	return (c==' ' || c=='\t');
}


void serror(int code)
{
	::code = code;
//  longjmp(j, code);
}

void set_exp(char *exp, double x)
{
	prog = exp;
	x_value = x;
}

int get_token()
{
	int tok;
	char *temp;
	(token_type) = 0;
	tok = 0;
	temp = (token);

	if (*(prog) == '\0')
	{
		*(token) = 0;
		tok = FINISHED;
		return ((token_type) = DELIMITER);
	}
	while (iswhite(*(prog))) ++(prog);
	if (isdelim(*(prog)))
	{
		*temp = *(prog);
		(prog)++;
		temp++;
		*temp = 0;
		return ((token_type) = DELIMITER);
	}
	if (isdigit(*(prog)))
	{
		while (!isdelim(*(prog)))
			*temp++=*(prog)++;
		*temp = '\0';
		return ((token_type) = NUMBER);
	}
	if (isalpha2(*(prog)))
	{
		while (!isdelim(*(prog)))
			*temp++=*(prog)++;
		(token_type) = VARIABLE;
	}
	*temp = '\0';
	if ((token_type) == VARIABLE)
	{
		tok = look_up((token));
		if (tok)
			(token_type) = FUNCTION;
	}
	return (token_type);
}

int sign(double d)
{
  if (d > 0.0)
    return 1.0;
  if (d < 0.0)
    return -1.0;
  return 0.0;
}

void putback()
{
	char *t;
	t = (token);
	for (;*t;t++)
		(prog)--;
}

int get_exp(double *hold)
{
  int res;
  code = 0;
//  if (res = setjmp(j) != 0)
//    return code;
	get_token();
	if (!*(token))
	{
		return 0;
	}
	level2( hold);
	putback();
  return 0;
}

void level2(double *hold)
{
	char op;
	double h;

	level3( hold);
	while ((op=*(token)) == '+' || op == '-')
	{
		get_token();
		level3( &h);
		arith(op, hold, &h);
	}
}

void level3(double *hold)
{
	char op;
	double h;

	level4( hold);
	while ((op=*(token)) == '*' || op == '/' || op == '%')
	{
		get_token();
		level4( &h);
		arith( op, hold, &h);
	}
}

void level4(double *hold)
{
	double h;
	level5( hold);

	if (*(token) == '^')
	{
		get_token();
		level5( &h);
		arith( '^', hold, &h);
	}
}

void level5(double *hold)
{
	char op;

	op = 0;
	if (((token_type) == DELIMITER) && *(token) == '+' || *(token) == '-')
	{
		op = *(token);
		get_token();
	}
	level6( hold);

	if (op)
		unary(op, hold);
}

void level6(double *hold)
{
	if ((*(token) == '(') && ((token_type) == DELIMITER))
	{
		get_token();
		level2( hold);
		if (*(token) != ')')
      serror( ERR_NOBRACKET);
		get_token();
	}
	else
		primitive( hold);
}

void calc_function(double *hold)
{
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

void primitive(double *hold)
{
	switch (token_type)
	{
	case VARIABLE:
		*hold = find_var(token);
		get_token();
		return;
	case NUMBER:
    //
		//*hold = atof((token));
    //if (sscanf(token, "%lf", hold) != 1)
	*hold = convert(token);
	if (*hold == ERROR)
      serror( ERR_BADNUMER);
		get_token();
		return;
	case FUNCTION:
		calc_function( hold);
		return;
	default:	// error
		return;
	}
}

void arith(char op, double *r, double *h)
{
	double t;
	switch(op)
	{
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
		t = (*r) / (*h);
		*r = *r - (t * (*h));
		break;
	case '^':
		*r = pow(*r, *h);
		break;
	}
}

void unary(char op, double *r)
{
	if (op == '-')
		*r = -(*r);
}

double find_var(char *s)
{
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

bool strcmp(char *s1, char *s2)
{
	int i;

	for (i = 0;;i++)
	{
		if (s1[i] == '\0')
			if (s2[i] == '\0')
				return 0;
			else
				return 1;
		else
			if (s2[i] == '\0')
				return 1;
			else
			{
				if (s1[i] != s2[i])
					return 1;
			}
	}
}

int look_up(char *s)
{
	int i;

	for (i = 0; i < max_func; i++)
		if (strcmp(s, functions[i].name) == 0)
			return i;
	return 0;	// search command/function name
}

/*
void delete_white(char *buf)
{
	int len = strlen(buf);
	char *d = (char *)malloc(len + 1);
	char *t = buf;
	strcpy(d, buf);
	d[len] = '\0';

	int i;

	for (i = 0; i < len; i++)
		if (!iswhite(d[i]))
			*t++=d[i];
	*t++='\0';
	free(d);
}
*/

/*
void main(void)
{

	 = (parser_struct)malloc(sizeof(parser_struct));
	double a;
	char buffer[256];


	printf("Enter expression: ");
	memset(buffer, 0, 256);
	gets(buffer);

	prog = buffer;
	delete_white(buffer);

	a = 0;
	x_value = 3;
	get_exp( &a);

	printf("result: %lg\n", a);
	getch();

}
*/