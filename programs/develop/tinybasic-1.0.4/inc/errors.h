/*
 * Tiny BASIC
 * Error Handling Header
 *
 * Copyright (C) Damian Gareth Walker 2019
 * Created: 15-Aug-2019
 */


#ifndef __ERRORS_H__
#define __ERRORS_H__


/*
 * Data Definitions
 */

/* error codes */
typedef enum {
  E_NONE, /* no error; everything is fine */
  E_INVALID_LINE_NUMBER, /* line number is invalid */
  E_UNRECOGNISED_COMMAND, /* command was not recognised */
  E_INVALID_VARIABLE, /* variable expected but something else encountered */
  E_INVALID_ASSIGNMENT, /* = expected but something else encountered */
  E_INVALID_EXPRESSION, /* an invalid expression was encountered */
  E_MISSING_RIGHT_PARENTHESIS, /* Encountered "(" without corresponding ")" */
  E_INVALID_PRINT_OUTPUT, /* failed to parse print output */
  E_BAD_COMMAND_LINE, /* error on invocation */
  E_FILE_NOT_FOUND, /* cannot open source file */
  E_INVALID_OPERATOR, /* unrecognised operator */
  E_THEN_EXPECTED, /* didn't find the expected THEN after an IF */
  E_UNEXPECTED_PARAMETER, /* more parameters encountered than expected */
  E_RETURN_WITHOUT_GOSUB, /* return encountered without a GOSUB */
  E_DIVIDE_BY_ZERO, /* an attempt to divide by zero */
  E_OVERFLOW, /* integer is out of range */
  E_MEMORY, /* out of memory */
  E_TOO_MANY_GOSUBS, /* recursive GOSUBs exceeded the stack size */
  E_LAST /* placeholder */
} ErrorCode;

/* error handler structure */
typedef struct error_handler ErrorHandler;
typedef struct error_handler {
  void *data; /* private data */
  void (*set_code) (ErrorHandler *, ErrorCode, int, int);
  ErrorCode (*get_code) (ErrorHandler *);
  int (*get_line) (ErrorHandler *);
  int (*get_label) (ErrorHandler *);
  char *(*get_text) (ErrorHandler *);
  void (*destroy) (ErrorHandler *);
} ErrorHandler;


/*
 * Constructors
 */


/*
 * Principal constructor
 * returns:
 *   ErrorHandler*   the new error handler object
 */
ErrorHandler *new_ErrorHandler (void);


#endif
