#ifndef FORMULA_FORMULA_H
#define FORMULA_FORMULA_H

// The formulas as Excel and LibreOffice Calc write them: numbers, cell
// references and ranges A1:B3, + - * / ^, % after a number (a hundredth),
// comparisons = <> < > <= >=, brackets and functions such as SUM(A1, 2),
// their names in any case, their arguments apart by ',' or ';'. A
// comparison gives 1 or 0, numbers equal in the 15 digits a double holds
// being equal; a value that is not a finite number is an error.

// the number at the start of s: spaces, '-', digits, '.' and digits, an
// exponent "e-5"; also ',' for the point if comma; *len its length, 0 if
// s starts with none or it is too large to be finite
double parse_number(const char *s, int *len, int comma);

// the length of a name or a number at s, letters, digits and '$'; 0 if
// none starts there
int word_len(const char *s);

// what is wrong with a formula, as Excel shows it; a formula Excel would
// not take at all, as Google Sheets shows it
enum {
	ERR_NONE,
	ERR_DIV0,   // a division by 0, in MOD() and FLOOR() too; AVERAGE() of none
	ERR_VALUE,  // a text where a number is wanted, AND() of no numbers
	ERR_REF,    // a reference to a cell gone, or to a cell that depends on itself
	ERR_NAME,   // a name that is no function and no cell
	ERR_NUM,    // no number comes out: SQRT(-1), 10^999
	ERR_SYNTAX, // no formula
	ERRORS
};

const char *error_text(int error); // "#DIV/0!"; error not ERR_NONE

// the numbers of a list function: SUM(), AVERAGE(), AND() and the like
typedef struct {
	double sum, min, max;
	int n, nonzero;
} List;

void list_add(List *list, double d);

// a formula without its '=': ERR_NONE and its value into *value, or its
// error. ref() gives the value of a cell reference, 0 for an empty cell,
// or an error: ERR_VALUE for a text, ERR_NAME for no reference, the error
// of the cell; range() adds the numbers of the cells from..to to a list,
// texts and empty cells skipped, or gives an error like ref()
int compute(const char *formula, double *value, int (*ref)(const char *name, double *value),
            int (*range)(const char *from, const char *to, List *list));

#endif
