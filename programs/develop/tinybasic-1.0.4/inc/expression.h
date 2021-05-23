/*
 * Tiny BASIC
 * Expression Handling Header
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 16-Aug-2019
 */


#ifndef __EXPRESSION_H__
#define __EXPRESSION_H__


/* Forward Declarations */
typedef struct expression_node ExpressionNode;
typedef struct right_hand_term RightHandTerm;
typedef struct term_node TermNode;
typedef struct right_hand_factor RightHandFactor;


/*
 * Data Definitions - Factors
 */


/* The different classes of factor */
typedef enum {
  FACTOR_NONE, /* class as yet undetermined */
  FACTOR_VARIABLE, /* a variable */
  FACTOR_VALUE, /* an integer value */
  FACTOR_EXPRESSION /* a bracketed expression */
} FactorClass;

/* The signs */
typedef enum {
  SIGN_POSITIVE,
  SIGN_NEGATIVE
} SignClass;

/* A factor */
typedef struct {
  FactorClass class; /* what kind of factor is this? */
  SignClass sign; /* which sign, positive or negative? */
  union {
    int variable;
    int value;
    ExpressionNode *expression;
  } data;
} FactorNode;


/* The term operators */
typedef enum {
  TERM_OPERATOR_NONE, /* single-factor term */
  TERM_OPERATOR_MULTIPLY, /* multiply */
  TERM_OPERATOR_DIVIDE /* divide */
} TermOperator;

/* the operator and right-hand part of a term */
typedef struct right_hand_factor {
  TermOperator op; /* the operator to apply: muliply or divide */
  FactorNode *factor; /* the factor to multiply or divide by */
  RightHandFactor *next; /* the next part of the term, if any */
} RightHandFactor;

/* A term */
typedef struct term_node {
  FactorNode *factor; /* the first factor of an expression */
  RightHandFactor *next; /* the right side term, if any */
} TermNode;

/* Expression Operator */
typedef enum {
  EXPRESSION_OPERATOR_NONE, /* single-term expression */
  EXPRESSION_OPERATOR_PLUS, /* addition */
  EXPRESSION_OPERATOR_MINUS /* substraction */
} ExpressionOperator;

/* the operator and right-hand part of an expression */
typedef struct right_hand_term {
  ExpressionOperator op; /* the operator to apply: plus or minus */
  TermNode *term; /* the term to add or subtract */
  RightHandTerm *next; /* next part of the expression, if any */
} RightHandTerm;

/* An expression */
typedef struct expression_node {
  TermNode *term; /* The first term of an expression */
  RightHandTerm *next; /* the right side expression, if any */
} ExpressionNode;


/*
 * Function Declarations
 */


/*
 * Constructor for a factor
 * returns:
 *   FactorNode*   the new factor
 */
FactorNode *factor_create (void);

/*
 * Destructor for a factor
 * params:
 *   FactorNode*   factor   the doomed factor
 */
void factor_destroy (FactorNode *factor);

/*
 * Constructor for a right-hand factor of a term
 * returns:
 *   RightHandFactor*   the new RH factor of a term
 */
RightHandFactor *rhfactor_create (void);

/*
 * Recursive destructor for a right-hand factor of a term
 * params:
 *   RightHandFactor*   rhterm   the doomed RH factor of a term
 */
void rhfactor_destroy (RightHandFactor *rhterm);

/*
 * Constructor for a term
 * returns:
 *   TermNode*   the new term
 */
TermNode *term_create (void);

/*
 * Destructor for a term
 * params:
 *   TermNode*   term   the doomed term
 */
void term_destroy (TermNode *term);

/*
 * Constructor for a right-hand expression
 * returns:
 *   RightHandTerm*   the RH term of an expression
 */
RightHandTerm *rhterm_create (void);

/*
 * Recursive destructor for a right-hand term of an expression
 * params:
 *   RightHandTerm*   rhexpression   the doomed RH expression
 */
void rhterm_destroy (RightHandTerm *rhterm);

/*
 * Constructor for an expression
 * returns:
 *   ExpressionNode*   the new expression
 */
ExpressionNode *expression_create (void);

/*
 * Destructor for a factor
 * params:
 *   FactorNode*   factor   the doomed factor
 */
void factor_destroy (FactorNode *factor);

/*
 * Destructor for a expression
 * params:
 *   ExpressionNode*   expression   the doomed expression
 */
void expression_destroy (ExpressionNode *expression);


#endif
