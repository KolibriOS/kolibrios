#include "formula.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define EQUAL_DIGITS 1e-15 // apart by less of the larger one: equal
#define MAX_ARGS 3 // IF(); a list function adds its numbers up as it goes
#define NAME_MAX 15 // "$CZ$100", "isnumber" and room

// what a function takes besides 0, 1 or 2 numbers
enum { LIST = -1, IF = -2, IS_NUMBER = -3 };

static const char *p; // the rest of the formula
static int error;     // the first error met, ERR_NONE for none
static int bad_syntax; // no formula, whatever its parts give
static int (*ref_value)(const char *name, double *value);
static int (*range_values)(const char *from, const char *to, List *list);

static double expr(void);

static void fail(int e)
{
	if (!error)
		error = e;
}

#define ERROR_TEXT 8 // "#DIV/0!" and its zero

const char *error_text(int e)
{
	static const char names[ERRORS][ERROR_TEXT] = {
		"", "#DIV/0!", "#VALUE!", "#REF!", "#NAME?", "#NUM!", "#ERROR!"
	};

	return names[e];
}

// d - d is 0 for a finite number; for an infinity or NaN it is NaN, which
// equals nothing
static int is_finite(double d)
{
	return d - d == 0.0;
}

double parse_number(const char *s, int *len, int comma)
{
	const char *q = s, *e;
	double whole = 0.0, frac = 0.0, div = 1.0, scale = 1.0, d;
	int digits = 0, minus, exp_minus, exp = 0;

	while (*q == ' ')
		q++;
	minus = *q == '-';
	if (minus)
		q++;
	for (; isdigit((unsigned char)*q); q++, digits++)
		whole = whole * 10.0 + (*q - '0');
	if (*q == '.' || (comma && *q == ','))
		for (q++; isdigit((unsigned char)*q); q++, digits++) {
			frac = frac * 10.0 + (*q - '0');
			div *= 10.0;
		}
	d = whole + frac / div;
	// an exponent: "e", a sign, digits; "2e" is no number with one
	if (digits && (*q == 'e' || *q == 'E')) {
		e = q + 1;
		exp_minus = *e == '-';
		if (*e == '-' || *e == '+')
			e++;
		if (isdigit((unsigned char)*e)) {
			for (q = e; isdigit((unsigned char)*q); q++)
				if (exp < 1000) // past it the number is 0 or infinite anyway
					exp = exp * 10 + (*q - '0');
			for (; exp > 0; exp--)
				scale *= 10.0;
			d = exp_minus ? d / scale : d * scale;
		}
	}
	*len = digits && is_finite(d) ? q - s : 0; // not so long it is infinite
	return minus ? -d : d;
}

// a value that is not a finite number fails the formula
static double check(double d)
{
	if (!is_finite(d))
		fail(ERR_NUM);
	return d;
}

/* ============================ functions =========================== */

// the table below holds own functions only: the address of a libc.obj one
// (dllimport) in a static table is its import slot, not its code. Each
// takes two numbers, b unused by those of one number or none
static double f_sin(double d, double b) { return sin(d); }
static double f_cos(double d, double b) { return cos(d); }
static double f_tan(double d, double b) { return tan(d); }
static double f_cot(double d, double b) { return 1.0 / tan(d); }
static double f_asin(double d, double b) { return asin(d); }
static double f_acos(double d, double b) { return acos(d); }
static double f_atan(double d, double b) { return atan(d); }
static double f_radians(double degrees, double b) { return degrees * M_PI / 180.0; }
static double f_degrees(double radians, double b) { return radians * 180.0 / M_PI; }
static double f_exp(double d, double b) { return exp(d); }
static double f_ln(double d, double b) { return log(d); }
static double f_log(double d, double b) { return log10(d); }
static double f_sqrt(double d, double b) { return sqrt(d); }
static double f_abs(double d, double b) { return fabs(d); }
static double f_not(double d, double b) { return d == 0.0; }
static double f_pi(double a, double b) { return M_PI; }
static double f_rand(double a, double b) { return rand() / (RAND_MAX + 1.0); }

// a to a multiple of step, the one below and the one above; of a step of
// 0 FLOOR() is #DIV/0! and CEILING() 0, as in Excel
static double f_floor(double a, double step)
{
	if (step == 0.0)
		fail(ERR_DIV0);
	return floor(a / step) * step;
}

static double f_ceiling(double a, double step)
{
	return step == 0.0 ? 0.0 : ceil(a / step) * step;
}

// the remainder with the sign of b, as Excel's MOD(): MOD(-7, 3) is 2
static double f_mod(double a, double b)
{
	if (b == 0.0)
		fail(ERR_DIV0);
	return a - floor(a / b) * b;
}

// a to digits decimals, digits whole (to tens for -1), a half away from
// 0. a * 10^digits is off by its last bit: 2.675 * 100 is 267.4999...,
// and a half so close to it is a half, as the 15 digits Excel shows are
static double f_round(double a, double digits)
{
	double scale = pow(10.0, digits < 0.0 ? ceil(digits) : floor(digits));
	double x = fabs(a * scale), r = round(x);

	if (r < x && x - r >= 0.5 - x * EQUAL_DIGITS)
		r += 1.0;
	return (a < 0.0 ? -r : r) / scale;
}

void list_add(List *list, double d)
{
	if (list->n == 0 || d < list->min)
		list->min = d;
	if (list->n == 0 || d > list->max)
		list->max = d;
	list->sum += d;
	list->nonzero += d != 0.0;
	list->n++;
}

// of no numbers SUM(), MIN() and MAX() give 0, AVERAGE() #DIV/0!, AND(),
// OR() and XOR() #VALUE!, as in Excel
static double f_sum(const List *l) { return l->sum; }
static double f_min(const List *l) { return l->min; }
static double f_max(const List *l) { return l->max; }

static double f_average(const List *l)
{
	if (!l->n)
		fail(ERR_DIV0);
	return l->sum / l->n;
}

static double f_and(const List *l)
{
	if (!l->n)
		fail(ERR_VALUE);
	return l->nonzero == l->n;
}

static double f_or(const List *l)
{
	if (!l->n)
		fail(ERR_VALUE);
	return l->nonzero > 0;
}

static double f_xor(const List *l) // an odd number of them true
{
	if (!l->n)
		fail(ERR_VALUE);
	return l->nonzero % 2;
}

#define FUNCTION_NAME 9 // "isnumber" and its zero

static const struct {
	char name[FUNCTION_NAME];
	signed char args; // 0, 1 or 2 numbers, or LIST, IF, IS_NUMBER
	void *f;  // double f(double a, double b) or f(const List *)
} functions[] = {
	{ "sin",      1,         f_sin },
	{ "cos",      1,         f_cos },
	{ "tan",      1,         f_tan },
	{ "cot",      1,         f_cot },
	{ "asin",     1,         f_asin },
	{ "acos",     1,         f_acos },
	{ "atan",     1,         f_atan },
	{ "radians",  1,         f_radians },
	{ "degrees",  1,         f_degrees },
	{ "exp",      1,         f_exp },
	{ "ln",       1,         f_ln },
	{ "log",      1,         f_log },    // of base 10
	{ "sqrt",     1,         f_sqrt },
	{ "abs",      1,         f_abs },
	{ "not",      1,         f_not },
	{ "round",    2,         f_round },  // ROUND(2.567, 2)
	{ "floor",    2,         f_floor },  // FLOOR(7, 5): 5
	{ "ceiling",  2,         f_ceiling },
	{ "mod",      2,         f_mod },
	{ "pi",       0,         f_pi },     // PI(), as every function with its brackets
	{ "rand",     0,         f_rand },   // 0 to 1, never 1
	{ "sum",      LIST,      f_sum },
	{ "average",  LIST,      f_average },
	{ "min",      LIST,      f_min },
	{ "max",      LIST,      f_max },
	{ "and",      LIST,      f_and },
	{ "or",       LIST,      f_or },
	{ "xor",      LIST,      f_xor },
	{ "if",       IF,        NULL },     // IF(condition, then[, else])
	{ "isnumber", IS_NUMBER, NULL },     // 1 for a number; 0 for an empty cell, a text, an error
};

#define FUNCTIONS (int)(sizeof(functions) / sizeof(functions[0]))

/* ============================= parsing ============================ */

static void skip_spaces(void) // cell_set() makes tabs spaces
{
	while (*p == ' ')
		p++;
}

// steps over op if the formula goes on with it; the spaces before it
// skipped either way
static int take(const char *op)
{
	int n = strlen(op);

	skip_spaces();
	if (strncmp(p, op, n) != 0)
		return 0;
	p += n;
	return 1;
}

// between arguments: ',' or, as in Excel of a language with a decimal
// comma, ';'
static int take_separator(void)
{
	return take(",") || take(";");
}

int word_len(const char *s)
{
	int n = 0;

	while (isalnum((unsigned char)s[n]) || s[n] == '$')
		n++;
	return n;
}

// a name at p into name in lower case: a letter or '$', then letters,
// digits and '$'; 0 if none starts there, or it is longer than NAME_MAX
static int take_name(char *name)
{
	int n, i;

	skip_spaces();
	if (!isalpha((unsigned char)*p) && *p != '$')
		return 0;
	n = word_len(p);
	if (n > NAME_MAX)
		return 0;
	for (i = 0; i < n; i++)
		name[i] = tolower((unsigned char)p[i]);
	name[n] = '\0';
	p += n;
	return 1;
}

// the index of a function by its name, or -1
static int find_function(const char *name)
{
	int i;

	for (i = 0; i < FUNCTIONS; i++)
		if (strcmp(name, functions[i].name) == 0)
			return i;
	return -1;
}

// the arguments of a list function after its bracket, its value: numbers,
// and ranges and references, whose texts and empty cells are skipped. A
// range has no spaces round its ':', as the references of a formula move
// only so
static double list_call(double (*f)(const List *))
{
	List list = { 0.0, 0.0, 0.0, 0, 0 };
	char from[NAME_MAX + 1], to[NAME_MAX + 1];
	const char *start;

	do {
		start = p;
		if (take_name(from)) {
			if (p[0] == ':' && p[1] != ' ') {
				p++;
				if (take_name(to))
					fail(range_values(from, to, &list));
				else
					bad_syntax = 1;
				continue;
			}
			skip_spaces();
			if (*p == ',' || *p == ';' || *p == ')') { // a reference alone
				fail(range_values(from, from, &list));
				continue;
			}
		}
		p = start; // a number
		list_add(&list, expr());
	} while (take_separator());
	if (!take(")"))
		bad_syntax = 1;
	return check(f(&list));
}

// ISNUMBER() of a reference alone after its bracket, taken with its
// bracket: 1 for a number, 0 for an empty cell, a text, an error; -1 and
// nothing taken for no reference alone
static int reference_is_number(void)
{
	List list = { 0.0, 0.0, 0.0, 0, 0 };
	char name[NAME_MAX + 1];
	const char *start = p;

	if (take_name(name) && take(")"))
		return range_values(name, name, &list) == ERR_NONE && list.n == 1;
	p = start;
	return -1;
}

// a function after its name and its bracket
static double call(int i)
{
	double a[MAX_ARGS] = { 0.0, 0.0, 0.0 };
	int bad[MAX_ARGS], n = 0, k, args = functions[i].args, outer = error;
	void *f = functions[i].f;

	if (args == IS_NUMBER && (k = reference_is_number()) >= 0)
		return k;
	if (args == LIST)
		return list_call((double (*)(const List *))f);

	// the arguments, each failing on its own: IF() needs only two of them
	if (!take(")")) {
		do {
			if (n == MAX_ARGS) {
				bad_syntax = 1;
				return 0.0;
			}
			error = ERR_NONE;
			a[n] = expr();
			bad[n++] = error;
		} while (take_separator());
		if (!take(")"))
			bad_syntax = 1;
	}
	error = outer;
	if (args == IS_NUMBER && n == 1) // of what an expression gives
		return bad[0] == ERR_NONE;
	if (args == IF && (n == 2 || n == 3)) {
		k = a[0] != 0.0 ? 1 : 2;
		fail(bad[0]);
		if (k == n) // no else: 0, Excel's FALSE
			return 0.0;
		fail(bad[k]);
		return a[k];
	}
	for (k = 0; k < n; k++)
		fail(bad[k]);
	if (n != args) { // a wrong number of arguments
		bad_syntax = 1;
		return 0.0;
	}
	return check(((double (*)(double, double))f)(a[0], a[1]));
}

// a number, a reference, a function or a bracket
static double primary(void)
{
	char name[NAME_MAX + 1];
	double d = 0.0;
	int len, i, e;

	if (take("(")) {
		d = expr();
		if (!take(")"))
			bad_syntax = 1;
		return d;
	}
	for (e = ERR_DIV0; e < ERR_SYNTAX; e++) // an error as a value: "=#REF!", which a reference gone leaves
		if (take(error_text(e))) {
			fail(e);
			return 0.0;
		}
	if (isdigit((unsigned char)*p) || *p == '.') {
		d = parse_number(p, &len, 0);
		p += len;
		if (len == 0 || isalnum((unsigned char)*p) || *p == '.') // "12abc", "1.2.3"
			bad_syntax = 1;
		return d;
	}
	if (!take_name(name)) {
		bad_syntax = 1;
		return 0.0;
	}
	// a reference, or a name alone such as PI without its brackets; a
	// function has its bracket right after its name
	if (*p != '(') {
		fail(ref_value(name, &d));
		return d;
	}
	p++;
	i = find_function(name);
	if (i >= 0)
		return call(i);
	fail(ERR_NAME); // no such function: its arguments read past
	if (!take(")"))
		list_call(f_sum);
	return 0.0;
}

static double unary(void)
{
	double d;

	if (take("-"))
		return -unary();
	if (take("+"))
		return unary();
	d = primary();
	while (take("%"))
		d /= 100.0;
	return d;
}

static double power(void)
{
	double d = unary();

	while (take("^"))
		d = check(pow(d, unary()));
	return d;
}

static double product(void)
{
	double d = power(), m;

	for (;;) {
		if (take("*"))
			d *= power();
		else if (take("/")) {
			m = power();
			if (m == 0.0)
				fail(ERR_DIV0);
			d /= m;
		} else
			return d;
		check(d);
	}
}

static double sum(void)
{
	double d = product();

	for (;;) {
		if (take("+"))
			d += product();
		else if (take("-"))
			d -= product();
		else
			return d;
		check(d);
	}
}

// the comparisons and what each gives as a is less than b, equal or
// greater; "<=" and "<>" before "<"
static const struct {
	char op[3];
	char less, equal, greater;
} comparisons[] = {
	{ "<=", 1, 1, 0 },
	{ ">=", 0, 1, 1 },
	{ "<>", 1, 0, 1 },
	{ "<",  1, 0, 0 },
	{ ">",  0, 0, 1 },
	{ "=",  0, 1, 0 },
};

#define COMPARISONS (int)(sizeof(comparisons) / sizeof(comparisons[0]))

// the index of the comparison the formula goes on with, or -1
static int take_comparison(void)
{
	int i;

	for (i = 0; i < COMPARISONS; i++)
		if (take(comparisons[i].op))
			return i;
	return -1;
}

// what comparison i gives of a and b; numbers equal in their 15 digits
// are equal: 0.1 + 0.2 = 0.3
static double compare(int i, double a, double b)
{
	double size_a = fabs(a), size_b = fabs(b), larger = size_a > size_b ? size_a : size_b;

	if (fabs(a - b) <= larger * EQUAL_DIGITS)
		return comparisons[i].equal;
	return a < b ? comparisons[i].less : comparisons[i].greater;
}

// comparisons, the lowest: "a + 1 > b" compares the sum
static double expr(void)
{
	double d = sum();
	int i;

	while ((i = take_comparison()) >= 0)
		d = compare(i, d, sum());
	return d;
}

int compute(const char *formula, double *value, int (*ref)(const char *name, double *value),
            int (*range)(const char *from, const char *to, List *list))
{
	p = formula;
	error = ERR_NONE;
	bad_syntax = 0;
	ref_value = ref;
	range_values = range;
	*value = expr(); // its last take() skipped the spaces after it
	if (bad_syntax || *p != '\0')
		return ERR_SYNTAX;
	return error;
}
