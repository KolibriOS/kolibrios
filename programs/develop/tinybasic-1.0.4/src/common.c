/*
 * Tiny BASIC Interpreter and Compiler Project
 * Common service routines module
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 20-Sep-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"


/*
 * Top Level Routines
 */


/*
 * Portable case-insensitive comparison
 * params:
 *   char*   a   string to compare
 *   char*   b   string to compare to
 * returns:
 *   int         -1 if a<b, 0 if a==b, 1 if a>b
 */
int tinybasic_strcmp (char *a, char *b) {
  do {
    if (toupper (*a) != toupper (*b))
      return (toupper (*a) > toupper (*b))
        - (toupper (*a) < toupper (*b));
    else {
      a++;
      b++;
    }
  } while (*a && *b);
  return 0;
}

