/*
 * Tiny BASIC
 * Tokenisation Header
 *
 * Copyright (C) Damian Walker 2019
 * Created: 04-Aug-2019
 */


#ifndef __TOKENISER_H__
#define __TOKENISER_H__


/* pre-requisite headers */
#include "token.h"


/*
 * Structure Defnitions
 */


/* Token stream */
typedef struct token_stream TokenStream;
typedef struct token_stream {
  void *data; /* private data */
  Token *(*next) (TokenStream *);
  int (*get_line) (TokenStream *);
  void (*destroy) (TokenStream *);
} TokenStream;


/*
 * Constructor Declarations
 */


/*
 * Constructor for TokenStream
 * params:
 *   FILE*   input   Input file
 * returns:
 *   TokenStream*    The new token stream
 */
TokenStream *new_TokenStream (FILE *input);


#endif
