#include "func.h"
#include "parser.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

// token types
#define DELIMITER 1
#define VARIABLE  2
#define NUMBER    3
#define FUNCTION  4

static double epsilon = 1e-6;

// parser state
static char token[80];
static int token_type;
static char *prog;
static int code; // error code

variable_callback *find_var;

typedef struct double_list {
	double val;
	int code; // error code of this argument
	struct double_list *next;
} double_list;

// forward declarations for the internal recursive-descent evaluator
static int get_token(void);
static int look_up(char *s);
static void level1(double *hold);
static void level1_5(double *hold);
static void level2(double *hold);
static void level3(double *hold);
static void level4(double *hold);
static void level5(double *hold);
static void level6(double *hold);
static void primitive(double *hold);
static void calc_function(double *hold);
static void arith(char op, double *r, double *h);
static void logic(char *op, double *r, double *h);
static void unary(char op, double *r);
static void putback(void);
static int isdelim(char c);
static int iswhite(char c);

/* ===== math functions exposed to formulas ===== */

// division that flags overflow when the divisor is ~0
static double safediv(double num, double den)
{
	if (fabs(den) < epsilon) {
		serror(ERR_OVERFLOW);
		return 0.0;
	}
	return num / den;
}

static double tg(double d)
{
	return safediv(sin(d), cos(d));
}

static double ctg(double d)
{
	return safediv(cos(d), sin(d));
}

// log with a domain check (libc log would return -inf)
static double f_log(double x)
{
	if (x <= 0) {
		serror(ERR_OVERFLOW);
		return 0.0;
	}
	return log(x);
}

// sqrt with a domain check (libc sqrt would return nan)
static double f_sqrt(double x)
{
	if (x < 0) {
		serror(ERR_BADPARAM);
		return 0.0;
	}
	return sqrt(x);
}

static double func_pi(void)
{
	return M_PI;
}

static double func_eps(void)
{
	return epsilon;
}

static double func_if(double_list *p)
{
	double_list *a, *b, *c;
	if (!p) {
		serror(ERR_BADPARAM);
		return 0.0;
	}
	a = p;
	b = a->next;
	if (!b) {
		serror(ERR_BADPARAM);
		return 0.0;
	}
	c = b->next;
	if (!c || c->next) {
		serror(ERR_BADPARAM);
		return 0.0;
	}
	if (a->val != 0.0) {
		if (b->code)
			code = b->code;
		return b->val;
	} else {
		if (c->code)
			code = c->code;
		return c->val;
	}
}

static double sum(double_list *p)
{
	double res = 0.0;
	while (p) {
		res += p->val;
		if (p->code)
			code = p->code;
		p = p->next;
	}
	return res;
}

// shared reducer for min()/max()
static double minmax(double_list *p, int want_max)
{
	double res;
	if (!p) {
		serror(ERR_BADPARAM);
		return 0.0;
	}
	res = p->val;
	for (p = p->next; p; p = p->next) {
		if (p->code)
			code = p->code;
		if (want_max ? p->val > res : p->val < res)
			res = p->val;
	}
	return res;
}

static double func_min(double_list *p)
{
	return minmax(p, 0);
}

static double func_max(double_list *p)
{
	return minmax(p, 1);
}

static double avg(double_list *p)
{
	double res = 0.0;
	int count = 0;
	while (p) {
		if (p->code)
			code = p->code;
		res += p->val;
		count++;
		p = p->next;
	}
	if (count == 0)
		return 0.0;
	return res / count;
}

static double func_isnull(char *str)
{
	int c;
	if (code != 0)
		return 0.0;
	find_var(str); // called for its side effect on 'code'
	c = code;
	code = 0;
	return c != 0 ? 1.0 : 0.0;
}

static double logic_xor(double a, double b)
{
	if (a == 0.0)
		return (b == 0.0) ? 0.0 : 1.0;
	return (b == 0.0) ? 1.0 : 0.0;
}

static double logic_and(double a, double b)
{
	if (a == 0.0)
		return 0.0;
	return (b == 0.0) ? 0.0 : 1.0;
}

static double logic_or(double a, double b)
{
	if (a == 0.0)
		return (b == 0.0) ? 0.0 : 1.0;
	return 1.0;
}

static double rand_seed;
static double func_rand(double max)
{
	double q = (257.0 * rand_seed + 739.0);
	rand_seed = q - 65536.0 * floor(q / 65536.0);
	if (max == 0.0)
		return 0.0;
	return q - max * floor(q / max);
}

#define INF_ARGS -1
#define STR_ARG  -2

// pointer flavours for the different function arities
typedef double (*matfunc0)(void);
typedef double (*matfunc)(double);
typedef double (*matfunc2)(double, double);
typedef double (*matfunc_inf)(double_list *);
typedef double (*matfunc_str)(char *);

// links a function name to its implementation
typedef struct {
	char name[10];
	int args;
	void *f;
} func;

#define MAX_FUNC 28
static const func functions[MAX_FUNC] = {
	{ "", 1, NULL }, // empty name marks "not found"
	{ "sin", 1, &sin },
	{ "cos", 1, &cos },
	{ "exp", 1, &exp },
	{ "sqrt", 1, &f_sqrt },
	{ "log", 1, &f_log },
	{ "tg", 1, &tg },
	{ "ctg", 1, &ctg },
	{ "arcsin", 1, &asin },
	{ "arccos", 1, &acos },
	{ "arctg", 1, &atan },
	{ "abs", 1, &fabs },
	{ "pow", 2, &pow },
	{ "if", INF_ARGS, &func_if },
	{ "sum", INF_ARGS, &sum },
	{ "isnull", STR_ARG, &func_isnull },
	{ "min", INF_ARGS, &func_min },
	{ "max", INF_ARGS, &func_max },
	{ "avg", INF_ARGS, &avg },
	{ "ceil", 1, &ceil },
	{ "round", 1, &round },
	{ "floor", 1, &floor },
	{ "and", 2, &logic_and },
	{ "or", 2, &logic_or },
	{ "xor", 2, &logic_xor },
	{ "rand", 1, &func_rand },
	{ "pi", 0, &func_pi },
	{ "eps", 0, &func_eps }
};

#define MAXDELIM 17
static const char delim[MAXDELIM] = "+-*^/%=;(),><#! ";

static int isdelim(char c)
{
	int i;
	for (i = 0; i < MAXDELIM; i++)
		if (c == delim[i])
			return 1;
	return 0;
}

int isdigit2(char c)
{
	return (c >= '0' && c <= '9');
}

int isalpha2(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '$'));
}

static int iswhite(char c)
{
	return (c == ' ' || c == '\t');
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

static int get_token(void)
{
	char *temp;
	token_type = 0;
	temp = token;

	if (*prog == '\0') {
		*token = 0;
		return (token_type = DELIMITER);
	}
	while (iswhite(*prog))
		++prog;
	if (isdelim(*prog)) {
		char t = *temp = *prog;
		prog++;
		temp++;
		if ((t == '>' || t == '<' || t == '!') && (*prog) && (*prog == '=')) {
			*temp = *prog;
			prog++;
			temp++;
		}
		*temp = 0;
		return (token_type = DELIMITER);
	}
	if (isdigit2(*prog)) {
		while (!isdelim(*prog))
			*temp++ = *prog++;
		*temp = '\0';
		return (token_type = NUMBER);
	}
	if (isalpha2(*prog)) {
		while (!isdelim(*prog))
			*temp++ = *prog++;
		token_type = VARIABLE;
	}
	*temp = '\0';
	if (token_type == VARIABLE) {
		if (look_up(token))
			token_type = FUNCTION;
	}
	return token_type;
}

static void putback(void)
{
	char *t;
	t = token;
	for (; *t; t++)
		prog--;
}

int get_exp(double *hold)
{
	code = 0;

	get_token();
	if (!*token)
		return 0;
	level1(hold);
	putback();
	return code == 0;
}

static void level1(double *hold)
{
	char op[2];
	double h;

	level1_5(hold);
	while (op[0] = *token, op[1] = (*(token + 1)) ? *(token + 1) : 0,
	       *op == '<' || *op == '>' || *op == '=' || *op == '#' || *op == '!') {
		get_token();
		level1_5(&h);
		logic(op, hold, &h);
	}
}

static void level1_5(double *hold)
{
	char op;

	op = 0;
	if ((token_type == DELIMITER) && *token == '!') {
		op = *token;
		get_token();
	}
	level2(hold);

	if (op)
		*hold = (*hold == 0.0) ? 1.0 : 0.0;
}

static void level2(double *hold)
{
	char op;
	double h;

	level3(hold);
	while ((op = *token) == '+' || op == '-') {
		get_token();
		level3(&h);
		arith(op, hold, &h);
	}
}

static void level3(double *hold)
{
	char op;
	double h;

	level4(hold);
	while ((op = *token) == '*' || op == '/' || op == '%') {
		get_token();
		level4(&h);
		arith(op, hold, &h);
	}
}

static void level4(double *hold)
{
	double h;
	level5(hold);

	if (*token == '^') {
		get_token();
		level5(&h);
		arith('^', hold, &h);
	}
}

static void level5(double *hold)
{
	char op;

	op = 0;
	if ((token_type == DELIMITER) && (*token == '+' || *token == '-')) {
		op = *token;
		get_token();
	}
	level6(hold);

	if (op)
		unary(op, hold);
}

static void level6(double *hold)
{
	if ((*token == '(') && (token_type == DELIMITER)) {
		get_token();
		level1(hold);
		if (*token != ')')
			serror(ERR_NOBRACKET);
		get_token();
	} else
		primitive(hold);
}

static void calc_function(double *hold)
{
	double_list *args = NULL, *last = NULL, *t;
	double d = 0.0; // stays 0 on the arg-count-mismatch path
	int i, argc = 0, save_code;

	save_code = code;
	code = 0;
	i = look_up(token);

	if (i == 0)
		serror(ERR_BADFUNCTION);

	get_token();
	if (*token != '(')
		serror(ERR_NOBRACKET);
	if (functions[i].args == STR_ARG) {
		get_token();
		d = ((matfunc_str)(functions[i].f))(token);
		*hold = d;
		get_token();
		if (save_code)
			code = save_code;
		return;
	}

	argc = 0;
	do {
		get_token();
		if (*token == ')')
			break;
		t = (double_list *)malloc(sizeof(double_list));
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

	if (argc != functions[i].args && functions[i].args >= 0) {
		serror(ERR_BADPARAM);
	} else {
		switch (functions[i].args) {
		case 0:
			d = ((matfunc0)(functions[i].f))();
			break;
		case 1:
			d = ((matfunc)(functions[i].f))(args->val);
			break;
		case 2:
			d = ((matfunc2)(functions[i].f))(args->val, args->next->val);
			break;
		case INF_ARGS:
			d = ((matfunc_inf)(functions[i].f))(args);
			break;
		}
	}

	t = args;
	while (t) {
		args = t->next;
		free(t);
		t = args;
	}

	*hold = d;
}

static void primitive(double *hold)
{
	switch (token_type) {
	case VARIABLE:
		*hold = find_var(token);
		get_token();
		return;
	case NUMBER:
		*hold = convert(token, NULL);
		if (convert_error == ERROR)
			serror(ERR_BADNUMER);
		get_token();
		return;
	case FUNCTION:
		calc_function(hold);
		if (*token != ')')
			serror(ERR_NOBRACKET);
		get_token();
		return;
	default:
		return;
	}
}

static void arith(char op, double *r, double *h)
{
	double t;
	switch (op) {
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
		*r = safediv(*r, *h);
		break;
	case '%':
		if (fabs(*h) < epsilon)
			serror(ERR_OVERFLOW);
		else {
			t = floor((*r) / (*h));
			*r = *r - (t * (*h));
		}
		break;
	case '^':
		*r = pow(*r, *h);
		break;
	}
}

static void logic(char *op, double *r, double *h)
{
	double t = 0.0;
	switch (*op) {
	case '<':
		if (*(op + 1) && *(op + 1) == '=')
			t = *r <= *h + epsilon ? 1.0 : 0.0;
		else
			t = *r < *h - epsilon ? 1.0 : 0.0;
		break;
	case '>':
		if (*(op + 1) && *(op + 1) == '=')
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
		if (*(op + 1) && *(op + 1) == '=')
			t = fabs(*r - *h) > epsilon ? 1.0 : 0.0;
		else
			serror(ERR_GENERAL);
		break;
	}
	*r = t;
}

static void unary(char op, double *r)
{
	if (op == '-')
		*r = -(*r);
}

static int look_up(char *s)
{
	int i;
	for (i = 0; i < MAX_FUNC; i++)
		if (strcmp(s, functions[i].name) == 0)
			return i;
	return 0;
}

unsigned int chrnum(char *text, char symbol)
{
	int num = 0;
	int i = 0;
	while (text[i]) {
		if (text[i] == symbol)
			num++;
		i++;
	}
	return num;
}
