/*
 * Tiny BASIC
 * Listing Output Module
 *
 * Released as Public Domain by Damian Gareth Walker, 2019
 * Created: 18-Sep-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "formatter.h"
#include "expression.h"
#include "errors.h"
#include "parser.h"


/*
 * Data Definitions
 */


/* private formatter data */
typedef struct formatter_data {
  ErrorHandler *errors; /* the error handler */
} FormatterData;

/* convenience variables */
static Formatter *this; /* the object being worked on */

/*
 * Forward References
 */


/* factor_output() has a forward reference to output_expression() */
static char *output_expression (ExpressionNode *expression);

/* output_statement() has a forward reference from output_if() */
static char *output_statement (StatementNode *statement);


/*
 * Functions
 */


/*
 * Output a factor
 * params:
 *   FactorNode*   factor   the factor to output
 * return:
 *   char*                  the text representation of the factor
 */
static char *output_factor (FactorNode *factor) {

  /* local variables */
  char *factor_text = NULL, /* the text of the whole factor */
    *factor_buffer = NULL, /* temporary buffer for prepending to factor_text */
    *expression_text = NULL; /* the text of a subexpression */

  /* work out the main factor text */
  switch (factor->class) {
    case FACTOR_VARIABLE:
      factor_text = malloc (2);
      sprintf (factor_text, "%c", factor->data.variable + 'A' - 1);
      break;
    case FACTOR_VALUE:
      factor_text = malloc (7);
      sprintf (factor_text, "%d", factor->data.value);
      break;
    case FACTOR_EXPRESSION:
      if ((expression_text = output_expression (factor->data.expression))) {
        factor_text = malloc (strlen (expression_text) + 3);
        sprintf (factor_text, "(%s)", expression_text);
        free (expression_text);
      }
      break;
    default:
      this->priv->errors->set_code
	(this->priv->errors, E_INVALID_EXPRESSION, 0, 0);
  }

  /* apply a negative sign, if necessary */
  if (factor_text && factor->sign == SIGN_NEGATIVE) {
    factor_buffer = malloc (strlen (factor_text) + 2);
    sprintf (factor_buffer, "-%s", factor_text);
    free (factor_text);
    factor_text = factor_buffer;
  }

  /* return the final factor representation */
  return factor_text;
}

/*
 * Output a term
 * params:
 *   TermNode*   term   the term to output
 * returns:
 *   char*              the text representation of the term
 */
static char *output_term (TermNode *term) {

  /* local variables */
  char
    *term_text = NULL, /* the text of the whole term */
    *factor_text = NULL, /* the text of each factor */
    operator_char; /* the operator that joins the righthand factor */
  RightHandFactor *rhfactor; /* right hand factors of the expression */

  /* begin with the initial factor */
  if ((term_text = output_factor (term->factor))) {
    rhfactor = term->next;
    while (! this->priv->errors->get_code (this->priv->errors) && rhfactor) {

      /* ascertain the operator text */
      switch (rhfactor->op) {
      case TERM_OPERATOR_MULTIPLY:
        operator_char = '*';
        break;
      case TERM_OPERATOR_DIVIDE:
        operator_char = '/';
        break;
      default:
        this->priv->errors->set_code
	  (this->priv->errors, E_INVALID_EXPRESSION, 0, 0);
        free (term_text);
        term_text = NULL;
      }

      /* get the factor that follows the operator */
      if (! this->priv->errors->get_code (this->priv->errors)
        && (factor_text = output_factor (rhfactor->factor))) {
        term_text = realloc (term_text,
          strlen (term_text) + strlen (factor_text) + 2);
        sprintf (term_text, "%s%c%s", term_text, operator_char, factor_text);
        free (factor_text);
      }

      /* look for another term on the right of the expression */
      rhfactor = rhfactor->next;
    }
  }

  /* return the expression text */
  return term_text;

}

/*
 * Output an expression for a program listing
 * params:
 *   ExpressionNode*   expression   the expression to output
 * returns:
 *   char*                          new string containint the expression text
 */
static char *output_expression (ExpressionNode *expression) {

  /* local variables */
  char
    *expression_text = NULL, /* the text of the whole expression */
    *term_text = NULL, /* the text of each term */
    operator_char; /* the operator that joins the righthand term */
  RightHandTerm *rhterm; /* right hand terms of the expression */

  /* begin with the initial term */
  if ((expression_text = output_term (expression->term))) {
    rhterm = expression->next;
    while (! this->priv->errors->get_code (this->priv->errors) && rhterm) {

      /* ascertain the operator text */
      switch (rhterm->op) {
      case EXPRESSION_OPERATOR_PLUS:
        operator_char = '+';
        break;
      case EXPRESSION_OPERATOR_MINUS:
        operator_char = '-';
        break;
      default:
        this->priv->errors->set_code
	  (this->priv->errors, E_INVALID_EXPRESSION, 0, 0);
        free (expression_text);
        expression_text = NULL;
      }

      /* get the terms that follow the operators */
      if (! this->priv->errors->get_code (this->priv->errors)
        && (term_text = output_term (rhterm->term))) {
        expression_text = realloc (expression_text,
          strlen (expression_text) + strlen (term_text) + 2);
        sprintf (expression_text, "%s%c%s", expression_text, operator_char,
          term_text);
        free (term_text);
      }

      /* look for another term on the right of the expression */
      rhterm = rhterm->next;
    }
  }

  /* return the expression text */
  return expression_text;

}

/*
 * LET statement output
 * params:
 *   LetStatementNode*   letn   data for the LET statement
 * returns:
 *   char*                      the LET statement text
 */
static char *output_let (LetStatementNode *letn) {

  /* local variables */
  char
    *let_text = NULL, /* the LET text to be assembled */
    *expression_text = NULL; /* the text of the expression */

  /* assemble the expression */
  expression_text = output_expression (letn->expression);

  /* assemble the final LET text, if we have an expression */
  if (expression_text) {
    let_text = malloc (7 + strlen (expression_text));
    sprintf (let_text, "LET %c=%s", 'A' - 1 + letn->variable, expression_text);
    free (expression_text);
  }

  /* return it */
  return let_text;
}


/*
 * IF statement output
 * params:
 *   IfStatementNode*   ifn   data for the IF statement
 * returns:
 *   char*                    the IF statement text
 */
static char *output_if (IfStatementNode *ifn) {

  /* local variables */
  char
    *if_text = NULL, /* the LET text to be assembled */
    *left_text = NULL, /* the text of the left expression */
    *op_text = NULL, /* the operator text */
    *right_text = NULL, /* the text of the right expression */
    *statement_text = NULL; /* the text of the conditional statement */

  /* assemble the expressions and conditional statement */
  left_text = output_expression (ifn->left);
  right_text = output_expression (ifn->right);
  statement_text = output_statement (ifn->statement);

  /* work out the operator text */
  op_text = malloc (3);
  switch (ifn->op) {
    case RELOP_EQUAL: strcpy (op_text, "="); break;
    case RELOP_UNEQUAL: strcpy (op_text, "<>"); break;
    case RELOP_LESSTHAN: strcpy (op_text, "<"); break;
    case RELOP_LESSOREQUAL: strcpy (op_text, "<="); break;
    case RELOP_GREATERTHAN: strcpy (op_text, ">"); break;
    case RELOP_GREATEROREQUAL: strcpy (op_text, ">="); break;
  }

  /* assemble the final IF text, if we have everything we need */
  if (left_text && op_text && right_text && statement_text) {
    if_text = malloc (3 + strlen (left_text) + strlen (op_text) +
      strlen (right_text) + 6 + strlen (statement_text) + 1);
    sprintf (if_text, "IF %s%s%s THEN %s", left_text, op_text, right_text,
      statement_text);
  }

  /* free up the temporary bits of memory we've reserved */
  if (left_text) free (left_text);
  if (op_text) free (op_text);
  if (right_text) free (right_text);
  if (statement_text) free (statement_text);

  /* return it */
  return if_text;
}


/*
 * GOTO statement output
 * params:
 *   GotoStatementNode*   goton   data for the GOTO statement
 * returns:
 *   char*                        the GOTO statement text
 */
static char *output_goto (GotoStatementNode *goton) {

  /* local variables */
  char
    *goto_text = NULL, /* the GOTO text to be assembled */
    *expression_text = NULL; /* the text of the expression */

  /* assemble the expression */
  expression_text = output_expression (goton->label);

  /* assemble the final LET text, if we have an expression */
  if (expression_text) {
    goto_text = malloc (6 + strlen (expression_text));
    sprintf (goto_text, "GOTO %s", expression_text);
    free (expression_text);
  }

  /* return it */
  return goto_text;
}


/*
 * GOSUB statement output
 * params:
 *   GosubStatementNode*   gosubn   data for the GOSUB statement
 * returns:
 *   char*                        the GOSUB statement text
 */
static char *output_gosub (GosubStatementNode *gosubn) {

  /* local variables */
  char
    *gosub_text = NULL, /* the GOSUB text to be assembled */
    *expression_text = NULL; /* the text of the expression */

  /* assemble the expression */
  expression_text = output_expression (gosubn->label);

  /* assemble the final LET text, if we have an expression */
  if (expression_text) {
    gosub_text = malloc (7 + strlen (expression_text));
    sprintf (gosub_text, "GOSUB %s", expression_text);
    free (expression_text);
  }

  /* return it */
  return gosub_text;
}


/*
 * END statement output
 * returns:
 *   char*   A new string with the text "END"
 */
static char *output_end (void) {
  char *end_text; /* the full text of the END command */
  end_text = malloc (4);
  strcpy (end_text, "END");
  return end_text;
}


/*
 * RETURN statement output
 * returns:
 *   char*   A new string with the text "RETURN"
 */
static char *output_return (void) {
  char *return_text; /* the full text of the RETURN command */
  return_text = malloc (7);
  strcpy (return_text, "RETURN");
  return return_text;
}


/*
 * PRINT statement output
 * params:
 *   PrintStatementNode*   printn   data for the PRINT statement
 * returns:
 *   char*                          the PRINT statement text
 */
static char *output_print (PrintStatementNode *printn) {

  /* local variables */
  char
    *print_text, /* the PRINT text to be assembled */
    *output_text = NULL; /* the text of the current output item */
  OutputNode *output; /* the current output item */

  /* initialise the PRINT statement */
  print_text = malloc (6);
  strcpy (print_text, "PRINT");

  /* add the output items */
  if ((output = printn->first)) {
    do {

      /* add the separator */
      print_text = realloc (print_text, strlen (print_text) + 2);
      strcat (print_text, output == printn->first ? " " : ",");

      /* format the output item */
      switch (output->class) {
      case OUTPUT_STRING:
        output_text = malloc (strlen (output->output.string) + 3);
        sprintf (output_text, "%c%s%c", '"', output->output.string, '"');
        break;
      case OUTPUT_EXPRESSION:
        output_text = output_expression (output->output.expression);
        break;
      }

      /* add the output item */
      print_text = realloc (print_text,
        strlen (print_text) + strlen (output_text) + 1);
      strcat (print_text, output_text);
      free (output_text);

    /* look for the next output item */
    } while ((output = output->next));
  }

  /* return the assembled text */
  return print_text;
}

/*
 * INPUT statement output
 * params:
 *   InputStatementNode*   inputn   the input statement node to show
 * returns:
 *   char *                         the text of the INPUT statement
 */
static char *output_input (InputStatementNode *inputn) {

  /* local variables */
  char
    *input_text, /* the INPUT text to be assembled */
    var_text[3]; /* text representation of each variable with separator */
  VariableListNode *variable; /* the current output item */

  /* initialise the INPUT statement */
  input_text = malloc (6);
  strcpy (input_text, "INPUT");

  /* add the output items */
  if ((variable = inputn->first)) {
    do {
      sprintf (var_text, "%c%c",
        (variable == inputn->first) ? ' ' : ',',
        variable->variable + 'A' - 1);
      input_text = realloc (input_text, strlen (input_text) + 3);
      strcat (input_text, var_text);
    } while ((variable = variable->next));
  }

  /* return the assembled text */
  return input_text;
}

/*
 * Statement output
 * params:
 *   StatementNode*   statement   the statement to output
 * returns:
 *   char*                        a string containing the statement line
 */
static char *output_statement (StatementNode *statement) {

  /* local variables */
  char *output = NULL; /* the text output */

  /* return null output for comments */
  if (! statement)
    return NULL;

  /* build the statement itself */
  switch (statement->class) {
    case STATEMENT_LET:
      output = output_let (statement->statement.letn);
      break;
    case STATEMENT_IF:
      output = output_if (statement->statement.ifn);
      break;
    case STATEMENT_GOTO:
      output = output_goto (statement->statement.goton);
      break;
    case STATEMENT_GOSUB:
      output = output_gosub (statement->statement.gosubn);
      break;
    case STATEMENT_RETURN:
      output = output_return ();
      break;
    case STATEMENT_END:
      output = output_end ();
     break;
    case STATEMENT_PRINT:
      output = output_print (statement->statement.printn);
      break;
    case STATEMENT_INPUT:
      output = output_input (statement->statement.inputn);
      break;
    default:
      output = malloc (24);
      strcpy (output, "Unrecognised statement.");
  }

  /* return the listing line */
  return output;
}

/*
 * Program Line Output
 * params:
 *   ProgramLineNode*   program_line     the line to output
 */
static void generate_line (ProgramLineNode *program_line) {

  /* local variables */
  char
    label_text [7], /* line label text */
    *output = NULL, /* the rest of the output */
    *line_text = NULL; /* the assembled line */

  /* initialise the line label */
  if (program_line->label)
    sprintf (label_text, "%5d ", program_line->label);
  else
    strcpy (label_text, "      ");

  /* build the statement itself */
  output = output_statement (program_line->statement);

  /* if this wasn't a comment, add it to the program */
  if (output) {
    line_text = malloc (strlen (label_text) + strlen (output) + 2);
    sprintf (line_text, "%s%s\n", label_text, output);
    free (output);
    this->output = realloc (this->output,
      strlen (this->output) + strlen (line_text) + 1);
    strcat (this->output, line_text);
    free (line_text);
  }
}


/*
 * Public Methods
 */


/*
 * Create a formatted version of the program
 * params:
 *   Formatter*     fomatter   the formatter
 *   ProgramNode*   program    the syntax tree
 */
static void generate (Formatter *formatter, ProgramNode *program) {

  /* local variables */
  ProgramLineNode *program_line; /* line to process */

  /* initialise this object */
  this = formatter;

  /* generate the code for the lines */
  program_line = program->first;
  while (program_line) {
    generate_line (program_line);
    program_line = program_line->next;
  }
}

/*
 * Destroy the formatter when no longer needed
 * params:
 *   Formatter*   formatter   the doomed formatter
 */
static void destroy (Formatter *formatter) {
  if (formatter) {
    if (formatter->output)
      free (formatter->output);
    if (formatter->priv)
      free (formatter->priv);
    free (formatter);
  }
}


/*
 * Constructors
 */


/*
 * The Formatter constructor
 * params:
 *   ErrorHandler   *errors   the error handler object
 * returns:
 *   Formatter*               the new formatter
 */
Formatter *new_Formatter (ErrorHandler *errors) {

  /* allocate memory */
  this = malloc (sizeof (Formatter));
  this->priv = malloc (sizeof (FormatterData));

  /* initialise methods */
  this->generate = generate;
  this->destroy = destroy;

  /* initialise properties */
  this->output = malloc (sizeof (char));
  *this->output = '\0';
  this->priv->errors = errors;

  /* return the new object */
  return this;
}
