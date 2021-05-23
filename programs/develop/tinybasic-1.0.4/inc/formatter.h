/*
 * Tiny BASIC
 * Listing Output Header
 *
 * Released as Public Domain by Damian Gareth Walker, 2019
 * Created: 18-Sep-2019
 */


#ifndef __FORMATTER_H__
#define __FORMATTER_H__


/* included headers */
#include "errors.h"
#include "statement.h"


/*
 * Data Declarations
 */


/* Formatter Class */
typedef struct formatter_data FormatterData;
typedef struct formatter Formatter;
typedef struct formatter {

  /* Properties */
  FormatterData *priv; /* private data */
  char *output; /* the formatted output */

  /*
   * Create a formatted version of the program
   * params:
   *   Formatter*     the formatter
   *   ProgramNode*   the syntax tree
   * returns:
   *   char*          the formatted BASIC program
   */
  void (*generate) (Formatter *, ProgramNode *);

  /*
   * Destroy the formatter when no longer needed
   * params:
   *   Formatter*   the doomed formatter
   */
  void (*destroy) (Formatter *);

} Formatter;


/*
 * Function Declarations
 */


/*
 * The Formatter constructor
 * returns:
 *   Formatter*   the new formatter
 */
Formatter *new_Formatter (ErrorHandler *errors);


#endif
