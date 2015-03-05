
/** \file lispglobals.h
 *  Storage of globals in a associated hash
 *
 */

#ifndef __lispglobals_h__
#define __lispglobals_h__

#include "yacasbase.h"
#include "lispobject.h"
#include "lisphash.h"


/// Value of a Lisp global variable.
/// The only special feature of this class is the attribute
/// #iEvalBeforeReturn, which defaults to #LispFalse. If this
/// attribute is set to #LispTrue, the value in #iValue needs to be
/// evaluated to get the value of the Lisp variable.
/// \sa LispEnvironment::GetVariable()

class LispGlobalVariable : public YacasBase
{
public:
    inline LispGlobalVariable(const LispGlobalVariable& aOther);
    LispGlobalVariable(LispPtr& aValue): iValue(aValue), iEvalBeforeReturn(LispFalse) {}
    inline LispGlobalVariable& operator=(const LispGlobalVariable& aOther);

    inline void SetEvalBeforeReturn(LispBoolean aEval);
    LispPtr iValue;
    LispBoolean iEvalBeforeReturn;
};

/// Associated hash of LispGlobalVariable objects

class LispGlobal : public LispAssociatedHash<LispGlobalVariable>
{
};




inline LispGlobalVariable::LispGlobalVariable(const LispGlobalVariable& aOther) : iValue(aOther.iValue), iEvalBeforeReturn(LispFalse)
{
}

inline void LispGlobalVariable::SetEvalBeforeReturn(LispBoolean aEval)
{
  iEvalBeforeReturn = aEval;
}


inline LispGlobalVariable& LispGlobalVariable::operator=(const LispGlobalVariable& aOther)
{
  iValue = (aOther.iValue);
  return *this;
}


#endif

