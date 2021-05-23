/*
 * Tiny BASIC Interpreter and Compiler Project
 * Common service routines module
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 20-Sep-2019
 */


#ifndef __COMMON_H__
#define __COMMON_H__


/*
 * Function Declarations
 */


/*
 * Portable case-insensitive comparison
 * params:
 *   char*   a   string to compare
 *   char*   b   string to compare to
 * returns:
 *   int         -1 if a<b, 0 if a==b, 1 if a>b
 */
int tinybasic_strcmp (char *a, char *b);


#endif
