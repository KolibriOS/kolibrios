/*
 * Tiny BASIC Interpreter and Compiler Project
 * Language Options Module
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 18-Aug-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"


/*
 * Data Definitions
 */


/* private data */
typedef struct {
  LineNumberOption line_numbers; /* mandatory, implied, optional */
  int line_limit; /* highest line number allowed */
  CommentOption comments; /* enabled, disabled */
  int gosub_limit; /* how many nested gosubs */
} Private;

/* convenience variables */
static LanguageOptions *this; /* object being worked on */
static Private *data; /* the object's private data */


/*
 * Public Methods
 */


/*
 * Set the line number option individually
 * params:
 *   LanguageOptions*    options        the options
 *   LineNumberOption    line_numbers   line number option to set
 */
static void set_line_numbers (LanguageOptions *options,
  LineNumberOption line_numbers) {
  this = options;
  data = this->data;
  data->line_numbers = line_numbers;
}

/*
 * Set the line number limit individually
 * params:
 *   LanguageOptions*    options        the options
 *   int                 line_limit     line number limit to set
 */
static void set_line_limit (LanguageOptions *options, int line_limit) {
  this = options;
  data = this->data;
  data->line_limit = line_limit;
}

/*
 * Set the comments option individually
 * params:
 *   LanguageOptions*    options    the options
 *   CommentOption       comments   comment option to set
 */
static void set_comments (LanguageOptions *options, CommentOption comments) {
  this = options;
  data = this->data;
  data->comments = comments;
}

/*
 * Set the GOSUB stack limit
 * params:
 *   LanuageOptions*   options   the options
 *   int               limit     the desired stack limit
 */
static void set_gosub_limit (LanguageOptions *options, int gosub_limit) {
  this = options;
  data = this->data;
  data->gosub_limit = gosub_limit;
}

/*
 * Return the line number setting
 * params:
 *   LanguageOptions*   options   the options
 * returns:
 *   LineNumberOption             the line number setting
 */
static LineNumberOption get_line_numbers (LanguageOptions *options) {
  this = options;
  data = this->data;
  return data->line_numbers;
}

/*
 * Return the line number limit
 * params:
 *   LanguageOptions*   options   the options
 * returns:
 *   int                          the line number setting
 */
static int get_line_limit (LanguageOptions *options) {
  this = options;
  data = this->data;
  return data->line_limit;
}

/*
 * Return the comments setting
 * params:
 *   LanguageOptions*   options   the options
 * returns:
 *   CommentOption                the line number setting
 */
static CommentOption get_comments (LanguageOptions *options) {
  this = options;
  data = this->data;
  return data->comments;
}

/*
 * Return the GOSUB stack limit setting
 * params:
 *   LanguageOptions*   options   the options
 * returns:
 *   int                          the current GOSUB stack limit
 */
static int get_gosub_limit (LanguageOptions *options) {
  this = options;
  data = this->data;
  return data->gosub_limit;
}

/*
 * Destroy the settings object
 * params:
 *   LanguageOptions*   options   the options
 */
static void destroy (LanguageOptions *options) {
  if (options) {
    if (options->data)
      free (options->data);
    free (options);
  }
}

/*
 * Constructor for language options
 * returns:
 *   LanguageOptions*   the new language options object
 */
LanguageOptions *new_LanguageOptions (void) {

  /* allocate memory */
  this = malloc (sizeof (LanguageOptions));
  data = this->data = malloc (sizeof (Private));

  /* initialise methods */
  this->set_line_numbers = set_line_numbers;
  this->set_line_limit = set_line_limit;
  this->set_comments = set_comments;
  this->set_gosub_limit = set_gosub_limit;
  this->get_line_numbers = get_line_numbers;
  this->get_line_limit = get_line_limit;
  this->get_comments = get_comments;
  this->get_gosub_limit = get_gosub_limit;
  this->destroy = destroy;

  /* initialise properties */
  data->line_numbers = LINE_NUMBERS_OPTIONAL;
  data->line_limit = 32767;
  data->comments = COMMENTS_ENABLED;
  data->gosub_limit = 64;

  /* return the new object */
  return this;
}
