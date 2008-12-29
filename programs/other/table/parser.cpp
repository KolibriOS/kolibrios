

#include "func.h"
#include "parser.h"
//#include <math.h>
//#include <stdlib.h>
//#include <stdio.h>

// token types
#define DELIMITER 1
#define VARIABLE 2
#define NUMBER 3
#define FUNCTION 4
#define FINISHED 10

//#define allocmem(x) malloc(x)
//#define freemem(x) free(x)

double epsilon = 1e-6;

// structure for most parser functions

	char token[80];
	int token_type;
	char *prog;

	int code;    // error code


variable_callback *find_var;

struct double_list  
{
	double val;
	int code;			// код ошибки
	double_list *next; 
};

double tg(double d)
{
	double cosd = cos(d);
	if (fabs(cosd) < epsilon)
	{
		serror(ERR_OVERFLOW);
		return 0.0;
	}
	return sin(d) / cosd;
}

double ctg(double d)
{
	double sind = sin(d);
	if (fabs(sind) < epsilon)
	{
		serror(ERR_OVERFLOW);
		return 0.0;
	}
	return cos(d) / sind;
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
	if (x <= 0)
	{
		serror(ERR_OVERFLOW);
		//return 0.0;
		__asm {
			fldz
		}
	}
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
	if (x < 0)
	{
		serror(ERR_BADPARAM);
		__asm {
			fldz
		}
	}
	__asm {
		fld x
		fsqrt
	}
}

double atan(double x)
{
	serror(ERR_GENERAL);
	return 0.0; // в лом
}

double pow(double x, double y)
{
	return exp(y * log(x)); // 
}

double func_pi()
{
	return 3.14159265358979;
}

double func_eps()
{
	return epsilon;
}

double func_if(double_list *p)
{
	double_list *a, *b, *c;
	a = p;
	b = a->next;
	if (!b)
	{
		serror(ERR_BADPARAM);
		return 0.0;
	}
	c = b->next;
	if (!c || c->next)
	{
		serror(ERR_BADPARAM);
		return 0.0;
	}
	if (a->val != 0.0)
	{
		if (b->code)
			code = b->code;
		return b->val;
	}
	else
	{
		if (c->code)
			code = c->code;
		return c->val;
	}
}

double sum(double_list *p)
{
	double res = 0.0;
	while (p)
	{
		res += p->val;
		if (p->code)
			code = p->code;
		p = p->next;
	}
	return res;
}

double func_min(double_list *p)
{
	if (!p)
		serror(ERR_BADPARAM);
	double res = p->val;
	p = p->next;
	while (p)
	{
		if (p->code)
			code = p->code;
		if (p->val < res)
			res = p->val;
		p = p->next;
	}
	return res;
}

double func_max(double_list *p)
{
	if (!p)
		serror(ERR_BADPARAM);
	double res = p->val;
	p = p->next;
	while (p)
	{
		if (p->code)
			code = p->code;
		if (p->val > res)
			res = p->val;
		p = p->next;
	}
	return res;
}

double avg(double_list *p)
{
	double res = 0.0;
	int count = 0;
	while (p)
	{
		if (p->code)
			code = p->code;
		res += p->val;
		count++;
		p = p->next;
	}
	return res / count;
}

double func_isnull(char *str)
{
	if (code != 0)
		return 0.0;
	double tmp = find_var(str);
	int c = code;
	code = 0;
	if (c != 0)
		return 1.0;
	return 0.0;
}

const double HALF = 0.5;
double func_ceil(double val)	// хотел round, а получился ceil...
{
	int x;
	__asm fld val
	__asm fld HALF				// да, криворуко ^_^
	__asm fadd
	__asm fistp x
	__asm fild x
}

double func_round(double val)
{
	int x;
	__asm fld val
	__asm fld epsilon
	__asm fadd
	__asm fistp x
	__asm fild x
}

const double ALMOST_HALF = 0.5 - epsilon;
double func_floor(double val)
{
	int x;
	__asm fld val
	__asm fld ALMOST_HALF
	__asm fsub
	__asm fistp x
	__asm fild x
}

double logic_xor(double a, double b)
{
	if (a == 0.0)
		if (b == 0.0)
			return 0.0;
		else
			return 1.0;
	else
		if (b == 0.0)
			return 1.0;
		else
			return 0.0;
}

double logic_and(double a, double b)
{
	if (a == 0.0)
		return 0.0;
	else
		if (b == 0.0)
			return 0.0;
		else
			return 1.0;
}

double logic_or(double a, double b)
{
	if (a == 0.0)
		if (b == 0.0)
			return 0.0;
		else
			return 1.1;
	else
		return 1.0;
}

double rand_seed;
double func_rand(double max)
{
	double q = (257.0 * rand_seed + 739.0);	// числа от балды. надо вставить правильные.
	rand_seed = q - 65536.0 * func_floor(q / 65536.0); // для хорошего распределения
	return q - max * func_floor(q / max);	 // для модуля
}

double func_case(double_list *p)
{
	if (!p || !p->next)
	{
		serror(ERR_BADPARAM);
		return 0.0;
	}
	double x = p->val;
	int count = (int)p->next->val;
	int i, k;

	double_list *cur = p->next->next;
	k = count;
	for (i = 0; i < count; i++)
	{
		if (!cur)
		{
			serror(ERR_BADPARAM);
			return 0.0;
		}
		if (fabs(x - cur->val) < epsilon)
		{
			if (k != count + 1)
			{
				serror(ERR_GENERAL);
				return 0.0;
			}
			k = i;
		}
		cur = cur->next;
	}

	for (i = 0; i < k; i++)
	{
		if (!cur)
		{
			serror(ERR_BADPARAM);
			return 0.0;
		}
		cur = cur->next;
	}
	if (!cur)					// проверки бип. достали бип.
	{
		serror(ERR_BADPARAM);
		return 0.0;
	}
	if (cur->code)
		code = cur->code;
	return cur->val;
}

#define INF_ARGS -1
#define STR_ARG -2

// represents general mathematical function
typedef double(*matfunc0)();
typedef double(*matfunc)(double);
typedef double(*matfunc2)(double,double);
typedef double(*matfunc3)(double,double,double);
typedef double(*matfunc_inf)(double_list*);
typedef double(*matfunc_str)(char*);

// used to link function name to the function
typedef struct  
{
	char name[10];
	int args;
	void * f;
} func;

// the list of functions
const int max_func = 29;
func functions[max_func] = 
{
	"", 1, NULL,								// не помню, с какой целью
	"sin", 1, &sin,
	"cos", 1, &cos,
	"exp", 1, &exp,
	"sqrt", 1, &sqrt,
	"log", 1, &log,
	"tg", 1, &tg,
	"ctg", 1, &ctg,
	"arcsin", 1, &asin,
	"arccos", 1, &acos,
	"arctg", 1, &atan,							// не реализовано. возвращает ошибку ERR_GENERAL
	"abs", 1, &fabs,
	"pow", 2, &pow,
	"if", INF_ARGS, &func_if,
	"sum",INF_ARGS,&sum,
	"isnull",STR_ARG,&func_isnull,				// слегка ч/ж
	"min",INF_ARGS,&func_min,
	"max",INF_ARGS,&func_max,
	"avg",INF_ARGS,&avg,
	"ceil",1,&func_ceil,
	"round",1,&func_round,
	"floor",1,&func_floor,
	"and",2,&logic_and,
	"or",2,&logic_or,
	"xor",2,&logic_xor,
	"rand",1,&func_rand,
	"case",INF_ARGS,&func_case,
	"pi",0,&func_pi,
	"eps",0,&func_eps
};

// all delimiters
#define MAXDELIM 17
const char delim[MAXDELIM]="+-*^/%=;(),><#! ";		// not bad words


int isdelim(char c)
{
	//return strchr(delim, c) != 0;
	for (int i = 0; i < MAXDELIM; i++)
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
		|| (c >= 'A' && c <= 'Z') || (c=='$'));
}

int iswhite(char c)
{
	return (c==' ' || c=='\t');
}


void serror(int acode)
{
	if (acode != 0)
		code = acode;
}

void set_exp(char *exp)
{
	prog = exp;
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
		char t=*temp = *(prog);
		(prog)++;
		temp++;
		if ((t == '>' || t == '<' || t == '!') && (*prog) && (*prog == '='))
		{
			*temp = *(prog);
			(prog)++;
			temp++;
		}
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

double sign(double d)
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
	code = 0;

	get_token();
	if (!*(token))
	{
		return 0;
	}
	level1( hold);
	putback();
  return code==0;
}

void level1(double *hold)
{
	char op[2];
	double h;

	level1_5( hold);
	while (op[0] = *token, op[1] = (*(token+1)) ? *(token + 1) : 0,
		*op == '<' || *op == '>' || *op == '=' || *op == '#' || *op == '!')
	{
		get_token();
		level1_5( &h);
		logic(op, hold, &h);
	}
}

void level1_5(double *hold)
{
	char op;

	op = 0;
	if (((token_type) == DELIMITER) && *(token) == '!')
	{
		op = *(token);
		get_token();
	}
	level2( hold);

	if (op)
	{
		if (*hold == 0.0)
			*hold = 1.0;
		else
			*hold = 0.0;
	}
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
		level1( hold);
		if (*(token) != ')')
			  serror( ERR_NOBRACKET);
		get_token();
	}
	else
		primitive( hold);
}

void calc_function(double *hold)
{
  double_list *args = NULL, *last = NULL, *t;		
  double d;
	int i,argc=0,save_code;

	save_code = code;
	code = 0;
	i = look_up(token);

	if (i == 0)
		serror(ERR_BADFUNCTION);	// error

	get_token();
	if (*(token) != '(')
		serror(ERR_NOBRACKET);	// error
	//get_token();
	if (functions[i].args == STR_ARG)
	{
		get_token();
		d = ((matfunc_str)(functions[i].f))(token);
		*hold = d;
		get_token();
		if (save_code)
			code = save_code;
		return;
	}

	//last = args = (double_list*)malloc(sizeof(double_list));
	//args->next = NULL;
	//level1(&args->val);
	//get_token();
	argc=0;
	do 
	{
		get_token();
		if (*token == ')')
			break;
		t = (double_list*)allocmem(sizeof(double_list));
		code = 0;
		level1(&t->val);
		t->code = code;
		t->next = NULL;
		if (last)
			last->next = t;
		else
			args = t;
		last = t;
		argc++;
	} while (*token == ',');

	code = save_code;

	if (argc != functions[i].args && functions[i].args >= 0)
	{
		serror(ERR_BADPARAM);
	}
	else
	{
		switch (functions[i].args)
		{
			case 0:
				d = ((matfunc0)(functions[i].f))();
				break;
			case 1:
				d = ((matfunc)(functions[i].f))(args->val);
				break;			
			case 2:
				d = ((matfunc2)(functions[i].f))(args->val,args->next->val);
				break;			
			case 3:
				d = ((matfunc3)(functions[i].f))(args->val,args->next->val,args->next->next->val);
				break;		
			case INF_ARGS:
				d = ((matfunc_inf)(functions[i].f))(args);
				break;
		}
	}

	t = args;
	while (t)
	{
		args = t->next;
		freemem(t);
		t = args;
	}
	
  *hold = d;
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
		*hold = atof((token));
    //if (sscanf(token, "%lf", hold) != 1)
	*hold = convert(token);
	if (convert_error == ERROR)
      serror( ERR_BADNUMER);
		get_token();
		return;
	case FUNCTION:
		calc_function( hold);
		if (*token != ')')
			serror(ERR_NOBRACKET);
		get_token();
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
	    if (fabs(*h) < epsilon)
			serror( ERR_OVERFLOW);
		else
		  *r = (*r) / (*h);
		break;
	case '%':
	    if (fabs(*h) < epsilon)
			serror( ERR_OVERFLOW);
		else
		{
			t = func_floor ((*r) / (*h));
			*r = *r - (t * (*h));
		}
		break;
	case '^':
		*r = pow(*r, *h);
		break;
	}
}

void logic(char *op, double *r, double *h)
{
	double t;
	switch (*op)
	{
	case '<':
		if (*(op+1) && *(op+1) == '=')
			t = *r <= *h + epsilon ? 1.0 : 0.0;
		else				
			t = *r < *h - epsilon? 1.0 : 0.0;	
		break;
	case '>':
		if (*(op+1) && *(op+1) == '=')
			t = *r >= *h - epsilon ? 1.0 : 0.0;
		else				
			t = *r > *h + epsilon ? 1.0 : 0.0;
		break;
	case '=':
		t = fabs(*r - *h) <= epsilon ? 1.0 : 0.0;
		break;
	case '#':
		t = fabs(*r - *h) > epsilon ? 1.0 : 0.0;
		break;
	case '!':
		if (*(op+1) && *(op+1) == '=')
			t = fabs(*r - *h) > epsilon ? 1.0 : 0.0;
		else
			serror(ERR_GENERAL);
		break;
	}
	*r = t;
}


void unary(char op, double *r)
{
	if (op == '-')
		*r = -(*r);
}

bool strcmp(char *s1, char *s2)
{
	int i;

	if (s1 == NULL)
		if (s2 == NULL)
			return 0;
		else
			return 1;
	else
		if (s2 == NULL)
			return 1;

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
	return 0;
}


bool strncmp(char *s1, char *s2, int n)
{
	int i;

	if (s1 == NULL)
		if (s2 == NULL)
			return 0;
		else
			return 1;
	else
		if (s2 == NULL)
			return 1;

	for (i = 0;i<n;i++)
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
	return 0;
}

int look_up(char *s)
{
	int i;

	for (i = 0; i < max_func; i++)
		if (strcmp(s, functions[i].name) == 0)
			return i;
	return 0;	// search command/function name
}

