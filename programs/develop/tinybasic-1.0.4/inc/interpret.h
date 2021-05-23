/*
 * Tiny BASIC Interpreter and Compiler Project
 * Interpreter header
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 23-Aug-2019
 */


#ifndef __INTERPRET_H__
#define __INTERPRET_H__


/* included headers */
#include "errors.h"
#include "options.h"
#include "statement.h"


/*
 * Data Declarations
 */


/* the interpreter object */
typedef struct interpreter_data InterpreterData;
typedef struct interpreter Interpreter;
typedef struct interpreter {

  /* Properties */
  InterpreterData *priv; /* private data */

  /*
   * Interpret the program
   * params:
   *   Interpreter*   the interpreter to use
   *   ProgramNode*   the program to interpret
   */
  void (*interpret) (Interpreter *, ProgramNode *);

  /*
   * Destructor
   * params:
   *   Interpreter*   the doomed interpreter
   */
  void (*destroy) (Interpreter *);

} Interpreter;


/*
 * Function Declarations
 */


/*
 * Constructor
 * returns:
 *   Interpreter*   the new interpreter
 */
Interpreter *new_Interpreter (ErrorHandler *errors, LanguageOptions *options);


#endif
