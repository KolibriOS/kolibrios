/** \file lispevalhash.h
 *  Storage of executable commands
 *
 */

#ifndef __lispevalhash_h__
#define __lispevalhash_h__

#include "yacasbase.h"
#include "lispobject.h"
#include "lisphash.h"
#include "evalfunc.h"


// new-style evaluator, passing arguments onto the stack in LispEnvironment
typedef void (*YacasEvalCaller)(LispEnvironment& aEnvironment,LispInt aStackTop);
class YacasEvaluator : public EvalFuncBase
{
public:
  // FunctionFlags can be orred when passed to the constructor of this function
  enum FunctionFlags
  {
    Function=0,   // Function: evaluate arguments
    Macro=1,      // Function: don't evaluate arguments
    Fixed = 0,    // fixed number of arguments
    Variable = 2  // variable number of arguments
  };
  YacasEvaluator(YacasEvalCaller aCaller,LispInt aNrArgs, LispInt aFlags)
    : iCaller(aCaller), iNrArgs(aNrArgs), iFlags(aFlags)
  {
  }
  virtual void Evaluate(LispPtr& aResult,LispEnvironment& aEnvironment,
                          LispPtr& aArguments);
private:
  YacasEvalCaller iCaller;
  LispInt iNrArgs;
  LispInt iFlags;
};

class YacasCoreCommands : public LispAssociatedHash<YacasEvaluator>
{
};



#endif
