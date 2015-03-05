
#ifndef __patternclass_h__
#define __patternclass_h__

#include "lisptype.h"
#include "lispobject.h"
#include "genericobject.h"
#include "patterns.h"

/// Wrapper for YacasPatternPredicateBase.
/// This class allows a YacasPatternPredicateBase to be put in a
/// LispGenericObject.
class PatternClass : public GenericClass
{
public:
  PatternClass(YacasPatternPredicateBase* aPatternMatcher);
  ~PatternClass();

  LispBoolean Matches(LispEnvironment& aEnvironment,
                      LispPtr& aArguments);
  LispBoolean Matches(LispEnvironment& aEnvironment,
                      LispPtr* aArguments);
public: //From GenericClass
  virtual LispChar * Send(LispArgList& aArgList);
  virtual LispChar * TypeName();

private:
  PatternClass(const PatternClass& aOther): iPatternMatcher(NULL)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  }
  PatternClass& operator=(const PatternClass& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
protected:
  YacasPatternPredicateBase* iPatternMatcher;
};




#endif

