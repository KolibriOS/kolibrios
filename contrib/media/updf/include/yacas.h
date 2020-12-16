/// \file
/// Definitions of DefaultYacasEnvironment and CYacas.

#ifndef __yacas_h__
#define __yacas_h__

#include "yacasbase.h"
#include "lispstring.h"
#include "stringio.h"
#include "tokenizer.h"
#include "lisphash.h"
#include "lispevalhash.h"
#include "infixparser.h"
#include "stdfileio.h"
#include "lispatom.h"
#include "lispeval.h"
#include "mathenvironment.h"
#include "lispglobals.h"
#include "lisperror.h"
#include "lispuserfunc.h"


/// The default environment for a Yacas session.
/// This class constructs a LispEnvironment (to be found in
/// #iEnvironment), and defines the Yacas core functions. The core
/// functions are listed in corefunctions.h . Examples of core
/// functions are \c Head, \c Set and \c Eval.

class DefaultYacasEnvironment : public YacasBase
{
public:
  DefaultYacasEnvironment();
  DefaultYacasEnvironment(LispOutput* aOutput, LispInt aStackSize);
  virtual ~DefaultYacasEnvironment();
  LispEnvironment& getEnv() {return iEnvironment;}

private:
  DefaultYacasEnvironment(const DefaultYacasEnvironment& aOther)
    : output(NULL),hash(),printer(),coreCommands(),globals(),prefixoperators(),infixoperators(),postfixoperators(),bodiedoperators(),infixprinter(aOther.infixprinter),userFunctions(),
      iEnvironment(coreCommands,userFunctions,
                 globals,hash,output,infixprinter,
                 prefixoperators,infixoperators,
                 postfixoperators,bodiedoperators,&input,0),
    input(aOther.input.Status())
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  }
  DefaultYacasEnvironment& operator=(const DefaultYacasEnvironment& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }

private:
  LispOutput* output;
  LispHashTable hash;
  LispPrinter printer;

  YacasCoreCommands coreCommands;
  LispGlobal globals;

  //Define the default operators.
  LispOperators prefixoperators;
  LispOperators infixoperators;
  LispOperators postfixoperators;
  LispOperators bodiedoperators;
  InfixPrinter infixprinter;

  LispUserFunctions userFunctions;

  LispEnvironment iEnvironment;

public:
  CachedStdUserInput input;
};


/// The Yacas engine.
/// This is the only class that applications need to use. It can
/// evaluate Yacas expressions. Every instance has its own Yacas
/// environment, in which the expressions are evaluated.



class CYacas : public YacasBase
{
public:

    /// Pseudo-constructor.
    /// \return A new instance of CYacas, with output connected to \p aOutput.
    /// If aOutput is NULL, connect to a new instance of  StdUserOutput.
    LISPIMPORT static CYacas* NewL(LispOutput* aOutput=0, LispInt aStackSize=50000);

    /// Destructor.
    LISPIMPORT virtual ~CYacas();

    /// Return the underlying Yacas environment.
    inline DefaultYacasEnvironment& getDefEnv() {return environment;}

    /// Evaluate a Yacas expression.
    /// First, \p aExpression is parsed by an InfixParser. Then it is
    /// evaluated in the underlying Lisp environment. Finally, the
    /// result is printed to #iResultOutput via the pretty printer or,
    /// if this is not defined, via an InfixPrinter.
    virtual void Evaluate(const LispChar * aExpression);

    /// Return the result of the expression.
    /// This is stored in #iResult.
    virtual LispChar * Result();

    /// Return the error message produced by the last evaluation.
    /// The error is retrieved from #environment.
    virtual LispChar * Error();

    /// Whether an error occured during the last evaluation.
    inline LispBoolean IsError();

private:
 
    /// Constructor.
    /// The output of #environment is directed to \p aOutput.
    CYacas(LispOutput* aOutput,LispInt aStackSize);

private:

    /// The underlying Yacas environment
    DefaultYacasEnvironment environment;

    /// String containing the result of the last evaluation
    LispString iResult;

    /// Stream pointing to #iResult.
    StringOutput iResultOutput;
};

inline LispBoolean CYacas::IsError()
{
    return (Error()[0] != '\0');
}

#endif


