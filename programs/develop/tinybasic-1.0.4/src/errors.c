/*
 * Tiny BASIC
 * Error Handling Module
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 18-Aug-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"


/*
 * Internal Data Structures
 */


/* Private data */
typedef struct {
  ErrorCode error; /* the last error encountered */
  int line; /* the source line on which the error occurred */
  int label; /* the label for the source line */
} Private;


/*
 * Internal Data
 */


/* convenience variables */
ErrorHandler *this; /* object being worked on */
Private *data; /* private data of object being worked on */

/* global variables */
static char *messages[E_LAST] = { /* the error messages */
  "Successful",
  "Invalid line number",
  "Unrecognised command",
  "Invalid variable",
  "Invalid assignment",
  "Invalid expression",
  "Missing )",
  "Invalid PRINT output",
  "Bad command line",
  "File not found",
  "Invalid operator",
  "THEN expected",
  "Unexpected parameter",
  "RETURN without GOSUB",
  "Divide by zero",
  "Overflow",
  "Out of memory",
  "Too many gosubs"
};


/*
 * Public Methods
 */


/*
 * Record an error encountered
 * globals:
 *   ErrorCode   error       the last error encountered
 *   int         line        the source line
 *   int         label       the line's label
 * params:
 *   ErrorCode   new_error   the error code to set
 *   int         new_line    the source line to set
 *   int         new_label   the label to set
 */
static void set_code (ErrorHandler *errors, ErrorCode new_error, int new_line,
  int new_label) {

  /* initialise */
  this = errors;
  data = this->data;

  /* set the properties */
  data->error = new_error;
  data->line = new_line;
  data->label = new_label;
}

/*
 * Return the last error code encountered
 * params:
 *   ErrorHandler*   errors   the error handler
 * returns:
 *   ErrorCode                the last error encountered
 */
static ErrorCode get_code (ErrorHandler *errors) {
  this = errors;
  data = this->data;
  return data->error;
}

/*
 * Return the last error line encountered
 * params:
 *   ErrorHandler*   errors   the error handler
 * returns:
 *   int                      the source line of the last error
 */
static int get_line (ErrorHandler *errors) {
  this = errors;
  data = this->data;
  return data->line;
}

/*
 * Return the last error label encountered
 * params:
 *   ErrorHandler*   errors   the error handler
 * returns:
 *   int                      the line label of the last error
 */
static int get_label (ErrorHandler *errors) {
  this = errors;
  data = this->data;
  return data->label;
}

/*
 * Generate an error message
 * params:
 *   ErrorHandler*   errors     the error handler
 * globals:
 *   char*           messages   a list of error messages
 * returns:
 *   char*                      the full error message
 */
static char *get_text (ErrorHandler *errors) {

  /* local variables */
  char
    *message, /* the complete message */
    *line_text, /* source line N */
    *label_text; /* label N */

  /* initialise the error object */
  this = errors;
  data = this->data;

  /* get the source line, if there is one */
  line_text = malloc (20);
  if (data->line)
    sprintf (line_text, ", source line %d", data->line);
  else
    strcpy (line_text, "");

  /* get the source label, if there is one */
  label_text = malloc (19);
  if (data->label)
    sprintf (label_text, ", line label %d", data->label);
  else
    strcpy (label_text, "");

  /* put the error message together */
  message = malloc (strlen (messages[data->error]) + strlen (line_text)
    + strlen (label_text) + 1);
  strcpy (message, messages[data->error]);
  strcat (message, line_text);
  strcat (message, label_text);
  free (line_text);
  free (label_text);

  /* return the assembled error message */
  return message;
}

/*
 * ErrorHandler destructor
 * params:
 *   ErrorHandler*   errors   the doomed error handler
 */
static void destroy (ErrorHandler *errors) {
  if ((this = errors)) {
    data = this->data;
    free (data);
    free (this);
  }
}


/*
 * Constructors
 */


/*
 * Principal constructor
 * returns:
 *   ErrorHandler*   the new error handler object
 */
ErrorHandler *new_ErrorHandler (void) {

    /* allocate memory */
    this = malloc (sizeof (ErrorHandler));
    this->data = data = malloc (sizeof (Private));

    /* initialise the methods */
    this->set_code = set_code;
    this->get_code = get_code;
    this->get_line = get_line;
    this->get_label = get_label;
    this->get_text = get_text;
    this->destroy = destroy;

    /* initialise the properties */
    data->error = E_NONE;
    data->line = 0;
    data->label = 0;

    /* return the new object */
    return this;
}