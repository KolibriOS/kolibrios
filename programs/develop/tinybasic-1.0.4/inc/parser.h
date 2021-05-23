/*
 * Tiny BASIC
 * Parser Header
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 08-Aug-2019
 */


#ifndef __PARSER_H__
#define __PARSER_H__


/* pre-requisite headers */
#include "statement.h"
#include "errors.h"
#include "options.h"
#include "tokeniser.h"


/*
 * Data Declarations
 */


/* the parser */
typedef struct parser_data ParserData;
typedef struct parser Parser;
typedef struct parser {

  /* Properties */
  ParserData *priv; /* parser's private data */

  /*
   * Public methods
   */

  /*
   * Parse the whole program
   * params:
   *   Parser*        The parser to use
   *   INPUT*         The input file to parse
   * returns:
   *   ProgramNode*   The parsed program
   */
  ProgramNode *(*parse) (Parser *);

  /*
   * Return the current source line we're parsing
   * params:
   *   Parser*   The parser to use
   * returns:
   *   int       the line returned
   */
  int (*get_line) (Parser *);

  /*
   * Return the label of the source line we're parsing
   * params:
   *   Parser*   The parser to use
    * returns:
   *   int       the label returned
   */
  int (*get_label) (Parser *);

  /*
   * Destroy this parser object
   * params:
   *   Parser*   the doomed parser
   */
  void (*destroy) (Parser *);

} Parser;


/*
 * Function Declarations
 */


/*
 * Constructor
 * params:
 *   ErrorHandler*      the error handler to use
 *   LanguageOptions*   the language options to use
 *   FILE*              the input file
 * returns:
 *   Parser*            the new parser
 */
Parser *new_Parser (ErrorHandler *, LanguageOptions *, FILE *);


#endif
