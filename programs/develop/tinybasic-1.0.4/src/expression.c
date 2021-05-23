/*
 * Tiny BASIC
 * Expression Handling Module
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 16-Aug-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "expression.h"
#include "errors.h"


/*
 * Functions for Dealing with Factors
 */


/*
 * Constructor for a factor
 * returns:
 *   FactorNode*   the new factor
 */
FactorNode *factor_create (void) {

  /* local variables */
  FactorNode *factor; /* the new factor */

  /* allocate memory and initialise members */
  factor = malloc (sizeof (FactorNode));
  factor->class = FACTOR_NONE;
  factor->sign = SIGN_POSITIVE;

  /* return the factor */
  return factor;
}

/*
 * Destructor for a factor
 * params:
 *   FactorNode*   factor   the doomed factor
 */
void factor_destroy (FactorNode *factor) {
  if (factor->class == FACTOR_EXPRESSION && factor->data.expression) {
    expression_destroy (factor->data.expression);
  }
  free (factor);
}


/*
 * Functions for Dealing with Terms
 */


/*
 * Constructor for a right-hand factor of a term
 * returns:
 *   RightHandFactor*   the new RH factor of a term
 */
RightHandFactor *rhfactor_create (void) {

  /* local variables */
  RightHandFactor *rhfactor; /* the RH factor of a term to create */

  /* allocate memory and initialise members */
  rhfactor = malloc (sizeof (RightHandFactor));
  rhfactor->op = TERM_OPERATOR_NONE;
  rhfactor->factor = NULL;
  rhfactor->next = NULL;

  /* return the new RH term */
  return rhfactor;
}

/*
 * Recursive destructor for a right-hand factor of a term
 * params:
 *   RightHandFactor*   rhfactor   the doomed RH factor of a term
 */
void rhfactor_destroy (RightHandFactor *rhfactor) {
  if (rhfactor->next)
    rhfactor_destroy (rhfactor->next);
  if (rhfactor->factor)
    factor_destroy (rhfactor->factor);
  free (rhfactor);
}

/*
 * Constructor for a term
 * returns:
 *   TermNode*   the new term
 */
TermNode *term_create (void) {

  /* local variables */
  TermNode *term; /* the new term */

  /* allocate memory and initialise members */
  term = malloc (sizeof (TermNode));
  term->factor = NULL;
  term->next = NULL;

  /* return the new term */
  return term;
}

/*
 * Destructor for a term
 * params:
 *   TermNode*   term   the doomed term
 */
void term_destroy (TermNode *term) {

  /* destroy the consituent parts of the term */
  if (term->factor)
    factor_destroy (term->factor);
  if (term->next)
    rhfactor_destroy (term->next);

  /* destroy the term itself */
  free (term);
}


/*
 * Functions for dealing with Expressions
 */


/*
 * Constructor for a right-hand expression
 * returns:
 *   RightHandTerm*   the RH term of an expression
 */
RightHandTerm *rhterm_create (void) {

  /* local variables */
  RightHandTerm *rhterm; /* the new RH expression */

  /* allocate memory and initialise members */
  rhterm = malloc (sizeof (RightHandTerm));
  rhterm->op = EXPRESSION_OPERATOR_NONE;
  rhterm->term = NULL;
  rhterm->next = NULL;

  /* return the new right-hand expression */
  return rhterm;
}

/*
 * Recursive destructor for a right-hand term of an expression
 * params:
 *   RightHandTerm*   rhterm   the doomed RH expression
 */
void rhterm_destroy (RightHandTerm *rhterm) {
  if (rhterm->next)
    rhterm_destroy (rhterm->next);
  if (rhterm->term)
    term_destroy (rhterm->term);
  free (rhterm);
}

/*
 * Constructor for an expression
 * returns:
 *   ExpressionNode*   the new expression
 */
ExpressionNode *expression_create (void) {

  /* local variables */
  ExpressionNode *expression; /* the new expression */

  /* allocate memory and initialise members */
  expression = malloc (sizeof (ExpressionNode));
  expression->term = NULL;
  expression->next = NULL;

  /* return the new expression */
  return expression;
}

/*
 * Destructor for a expression
 * params:
 *   ExpressionNode*   expression   the doomed expression
 */
void expression_destroy (ExpressionNode *expression) {

  /* destroy the consituent parts of the expression */
  if (expression->term)
    term_destroy (expression->term);
  if (expression->next)
    rhterm_destroy (expression->next);

  /* destroy the expression itself */
  free (expression);
}
