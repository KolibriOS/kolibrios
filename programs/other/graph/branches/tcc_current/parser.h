#ifndef PARSER_H
#define PARSER_H

void set_exp(char* exp, double x);

void putback(double* hold); // puts the token back to line

int get_exp(double* hold); // gets the expression. This function is used externally

void level2(double* hold); // works with +-

void level3(double* hold); // works with */%

void level4(double* hold); // works with ^

void level5(double* hold); // works with ()

void level6(double* hold); // works with elementary tokens

void primitive(double* hold); // gets value of number, function or variable

void arith(char op, double* r, double* h); // performs arithmetical operation

void unary(char op, double* r); // performs unary (one-operand) operation

double find_var(const char* s); // gets variable value by name

void serror(int code); // stops execution of parser and return error code

int look_up(const char* s); // checks the function table to see if such a function exists

#endif /* PARSER_H */