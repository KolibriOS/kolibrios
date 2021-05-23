/*
 * Tiny BASIC Interpreter and Compiler Project
 * Language Options Header
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 18-Aug-2019
 */


#ifndef __OPTIONS_H__
#define __OPTIONS_H__


/*
 * Data Definitions
 */


/* line number options */
typedef enum {
  LINE_NUMBERS_OPTIONAL, /* they are optional numeric labels */
  LINE_NUMBERS_IMPLIED, /* missing line numbers are implied */
  LINE_NUMBERS_MANDATORY /* every line requires a number in sequence */
} LineNumberOption;

/* comment options */
typedef enum {
  COMMENTS_ENABLED, /* comments and blank lines are allowed */
  COMMENTS_DISABLED /* comments and blank lines are not allowed */
} CommentOption;

/* language options */
typedef struct language_options LanguageOptions;
typedef struct language_options {
  void *data; /* private data */
  void (*set_line_numbers) (LanguageOptions *, LineNumberOption);
  void (*set_line_limit) (LanguageOptions *, int);
  void (*set_comments) (LanguageOptions *, CommentOption);
  void (*set_gosub_limit) (LanguageOptions *, int);
  LineNumberOption (*get_line_numbers) (LanguageOptions *);
  int (*get_line_limit) (LanguageOptions *);
  CommentOption (*get_comments) (LanguageOptions *);
  int (*get_gosub_limit) (LanguageOptions *);
  void (*destroy) (LanguageOptions *);
} LanguageOptions;


/*
 * Function Declarations
 */


/*
 * Constructor for language options
 * returns:
 *   LanguageOptions*   the new language options object
 */
LanguageOptions *new_LanguageOptions (void);


#endif
