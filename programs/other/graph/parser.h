

void set_exp(char *exp, double x);
// puts the token back to line
void putback(double *hold);
// gets the expression. This function is used externally
int get_exp(double *hold);
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
// performs unary (one-operand) operation
void unary(char op, double *r);
// gets variable value by name
double find_var(char *s);
// stops execution of parser and return error code
void serror(int code);
// checks the function table to see if such a function exists
int look_up(char *s);
