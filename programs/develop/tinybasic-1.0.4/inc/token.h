/*
 * Tiny BASIC
 * Token Handling Header
 *
 * Copyright (C) Damian Walker 2019
 * Created: 15-Aug-2019
 */


#ifndef __TOKEN_H__
#define __TOKEN_H__


/*
 * Type Declarations
 */


/* token classes */
typedef enum
  {
   TOKEN_NONE, /* no token has yet been identified */
   TOKEN_EOF, /* end of file */
   TOKEN_EOL, /* end of line */
   TOKEN_WORD, /* an identifier or keyword - token to be removed */
   TOKEN_NUMBER, /* a numeric constant */
   TOKEN_SYMBOL, /* a legal symbol - token to be removed */
   TOKEN_STRING, /* a string constant */
   TOKEN_LET, /* the LET keyword */
   TOKEN_IF, /* the IF keyword */
   TOKEN_THEN, /* the THEN keyword */
   TOKEN_GOTO, /* the GOTO keyword */
   TOKEN_GOSUB, /* the GOSUB keyword */
   TOKEN_RETURN, /* the RETURN keyword */
   TOKEN_END, /* the END keyword */
   TOKEN_PRINT, /* the PRINT keyword */
   TOKEN_INPUT, /* the INPUT keyword */
   TOKEN_REM, /* the REM keyword */
   TOKEN_VARIABLE, /* a single letter A..Z */
   TOKEN_PLUS, /* addition or unary positive */
   TOKEN_MINUS, /* subtraction or unary negative */
   TOKEN_MULTIPLY, /* multiplication */
   TOKEN_DIVIDE, /* division */
   TOKEN_LEFT_PARENTHESIS, /* open parenthesis */
   TOKEN_RIGHT_PARENTHESIS, /* close parenthesis */
   TOKEN_EQUAL, /* = */
   TOKEN_UNEQUAL, /* <> or >< */
   TOKEN_LESSTHAN, /* < */
   TOKEN_LESSOREQUAL, /* <= */
   TOKEN_GREATERTHAN, /* > */
   TOKEN_GREATEROREQUAL, /* >= */
   TOKEN_COMMA, /* comma separator */
   TOKEN_ILLEGAL /* unrecognised characters */
  } TokenClass;

/* token structure */
typedef struct token Token;
typedef struct token
{
  void *data; /* private data */
  TokenClass (*get_class) (Token *);
  int (*get_line) (Token *);
  int (*get_pos) (Token *);
  char *(*get_content) (Token *);
  void (*set_class) (Token *, TokenClass);
  void (*set_line_pos) (Token *, int, int);
  void (*set_content) (Token *, char *);
  void (*initialise) (Token *, TokenClass, int, int, char *);
  void (*destroy) (Token *); /* destructor */
} Token;


/*
 * Function Declarations
 */


/*
 * Token constructor without values to initialise
 * returns:
 *   Token*   the created token
 */
Token *new_Token (void);

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
Token *new_Token_init (TokenClass class, int line, int pos, char *content);


#endif
