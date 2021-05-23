/*
 * Tiny BASIC
 * Token handling functions
 *
 * Copyright (C) Damian Walker 2019
 * Created: 15-Aug-2019
 */


/* includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"


/*
 * Internal data structures
 */


/* Private data */
typedef struct {
  TokenClass class; /* class of token */
  int line; /* line on which token was found */
  int pos; /* position within the line on which token was found */
  char *content; /* representation of token */
} Private;


/*
 * Internal Data
 */


/* Convenience variables */
static Token *this; /* the token object */
static Private *data; /* the private data */


/*
 * Public methods
 */


/*
 * Return the class of the token
 * params:
 *   Token*   token   the token object
 * returns:
 *   TokenClass       the class of the token
 */
static TokenClass get_class (Token *token) {
  this = token;
  data = token->data;
  return data->class;
}

/*
 * Return the line on which the token begins
 * params:
 *   Token*   token   the token object
 * returns:
 *   int              the line on which the token begins
 */
static int get_line (Token *token) {
  this = token;
  data = token->data;
  return data->line;
}

/*
 * Return the character position on which the token begins
 * params:
 *   Token*   token   the token object
 * returns:
 *   int              the position on which the token begins
 */
static int get_pos (Token *token) {
  this = token;
  data = token->data;
  return data->pos;
}

/*
 * Return the content of the token begins
 * params:
 *   Token*   token   the token object
 * returns:
 *   char*            the text content of the token
 */
static char *get_content (Token *token) {
  this = token;
  data = token->data;
  return data->content;
}

/*
 * Set the token class
 * params:
 *   Token*       token   the token to set
 *   TokenClass   class   the class
 */
static void set_class (Token *token, TokenClass class) {
  this = token;
  data = this->data;
  data->class = class;
}

/*
 * Set the token line and position
 * params:
 *   Token*   token   the token to set
 *   int      line    the line on which the token began
 *   int      pos     the position on which the token began
 */
static void set_line_pos (Token *token, int line, int pos) {
  this = token;
  data = this->data;
  data->line = line;
  data->pos = pos;;
}

/*
 * Set the token's text content
 * params:
 *   Token*   token     the token to alter
 *   char*    content   the text content to set
 */
static void set_content (Token *token, char *content) {
  this = token;
  data = this->data;
  if (data->content)
    free (data->content);
  data->content = malloc (strlen (content) + 1);
  strcpy (data->content, content);
}

/*
 * Set all of the values of an existing token in a single function call.
 * params:
 *   Token*   token   the token to update
 *   TokenClass   class     class of token to initialise
 *   int          line      line on which the token occurred
 *   int          pos       character position on which the token occurred
 *   char*        content   the textual content of the token
 */
static void initialise (Token *token, TokenClass class, int line, int pos,
  char *content) {

  /* set convenience variables */
  this = token;
  data = this->data;

  /* initialise the easy members */
  data->class = class ? class : TOKEN_NONE;
  data->line = line ? line : 0;
  data->pos = pos ? pos : 0;

  /* initialise the content */
  if (content)
    set_content (this, content);
}

/*
 * Token destructor
 * params:
 *   Token*   token   the doomed token
 */
static void destroy (Token *token) {
  if ((this = token)) {
    data = this->data;
    if (data->content)
      free (data->content);
    free (data);
    free (this);
  }
}


/*
 * Constructors
 */


/*
 * Token constructor without values to initialise
 * returns:
 *   Token*   the created token
 */
Token *new_Token (void) {

  /* create the structure */
  this = malloc (sizeof (Token));
  this->data = data = malloc (sizeof (Private));

  /* initialise properties */
  data->class = TOKEN_NONE;
  data->line = data->pos = 0;
  data->content = NULL;

  /* set up methods */
  this->initialise = initialise;
  this->get_class = get_class;
  this->get_line = get_line;
  this->get_pos = get_pos;
  this->get_content = get_content;
  this->set_class = set_class;
  this->set_line_pos = set_line_pos;
  this->set_content = set_content;
  this->destroy = destroy;

  /* return the created structure */
  return this;
}

/*
 * Token constructor with values to initialise
 * params:
 *   TokenClass   class     class of token to initialise
 *   int          line      line on which the token occurred
 *   int          pos       character position on which the token occurred
 *   char*        content   the textual content of the token
 * returns:
 *   Token*                 the created token
 */
Token *new_Token_init (TokenClass class, int line, int pos, char *content) {

  /* create a blank token */
  this = new_Token ();
  this->initialise (this, class, line, pos, content);

  /* return the new token */
  return this;
}
