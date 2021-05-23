/*
 * Tiny BASIC Interpreter and Compiler Project
 * C Output Header
 *
 * Copyright (C) Damian Gareth Walker 2019
 * Created: 03-Oct-2019
 */


#ifndef __GENERATEC_H__
#define __GENERATEC_H__


/* included headers */
#include "errors.h"
#include "options.h"

/* forward references */
typedef struct c_program CProgram;

/* object structure */
typedef struct c_program {
  void *private_data; /* private data */
  char *c_output; /* the generated C code */
  void (*generate) (CProgram *, ProgramNode *); /* generate function */
  void (*destroy) (CProgram *); /* destructor */
} CProgram;


/*
 * Function Declarations
 */


/*
 * Constructor
 * params:
 *   ErrorHandler*      compiler_errors    the error handler
 *   LanguageOptions*   compiler_options   language options
 * changes:
 *   CProgram*       this                  the object being created
 *   Private*        data                  the object's private data
 * returns:
 *   CProgram*                             the created object
 */
CProgram *new_CProgram (ErrorHandler *compiler_errors,
  LanguageOptions *compiler_options);


#endif