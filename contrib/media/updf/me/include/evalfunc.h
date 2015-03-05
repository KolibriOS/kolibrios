
#ifndef __evalfuncbase_h__
#define __evalfuncbase_h__

#include "yacasbase.h"

// class EvalFuncBase defines the interface to 'something that can
// evaluate'
class LispEnvironment;
class EvalFuncBase : public YacasBase
{
public:
    virtual void Evaluate(LispPtr& aResult,LispEnvironment& aEnvironment,
                  LispPtr& aArguments)=0;
    virtual ~EvalFuncBase() {}
};

#endif
