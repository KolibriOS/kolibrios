

#ifndef __lispuserfunc_h__
#define __lispuserfunc_h__

#include "yacasbase.h"
#include "lispobject.h"
#include "lispenvironment.h"
#include "lisphash.h"
#include "grower.h"
#include "evalfunc.h"

/// Abstract class providing the basic user function API.
/// Instances of this class are associated to the name of the function
/// via an associated hash table. When obtained, they can be used to
/// evaluate the function with some arguments.

class LispUserFunction : public EvalFuncBase
{
public:
    LispUserFunction() : iFenced(LispTrue),iTraced(LispFalse) {};
    virtual ~LispUserFunction();
    virtual void Evaluate(LispPtr& aResult,LispEnvironment& aEnvironment,
                  LispPtr& aArguments)=0;
    virtual void HoldArgument(LispString * aVariable) = 0;
    virtual void DeclareRule(LispInt aPrecedence, LispPtr& aPredicate,
                             LispPtr& aBody) = 0;
    virtual void DeclareRule(LispInt aPrecedence, LispPtr& aBody) = 0;
    virtual void DeclarePattern(LispInt aPrecedence, LispPtr& aPredicate,
                             LispPtr& aBody) = 0;
    virtual LispPtr& ArgList() = 0;

public: //unfencing
    inline void UnFence() {iFenced = LispFalse;};
    inline LispBoolean Fenced() {return iFenced;};
public: //tracing
    inline void Trace() {iTraced = LispTrue;};
    inline void UnTrace() {iTraced = LispFalse;};
    inline LispBoolean Traced() {return iTraced;};
private:
    LispBoolean iFenced;
    LispBoolean iTraced;
};


/// User function with a specific arity.
/// This is still an abstract class, but the arity (number of
/// arguments) of the function is now fixed.

class LispArityUserFunction : public LispUserFunction
{
public:
    virtual LispInt Arity() const = 0;
    virtual LispInt IsArity(LispInt aArity) const = 0;
};


class LispDefFile;


/// Set of LispArityUserFunction's.
/// By using this class, you can associate multiple functions (with
/// different arities) to one name. A specific LispArityUserFunction
/// can be selected by providing its name. Additionally, the name of
/// the file in which the function is defined, can be specified.

class LispMultiUserFunction : public YacasBase
{
public:
  /// Constructor.
  LispMultiUserFunction() : iFunctions(),iFileToOpen(NULL) {};

  /** When adding a multi-user function to the association hash table, the copy constructor is used.
   *  We should at least make sure that iFunctions is empty, so there is no copying needed (for efficiency).
   *  Casually having a copy-constructor on CDeletingArrayGrower should be avoided, to make sure it is
   *  not used accidentally.
   */
  inline LispMultiUserFunction(const LispMultiUserFunction& aOther) : iFunctions(), iFileToOpen(NULL)
  {
    LISPASSERT(aOther.iFileToOpen == 0);
    LISPASSERT(aOther.iFunctions.Size() == 0);
  }
  inline LispMultiUserFunction& operator=(const LispMultiUserFunction& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(aOther.iFileToOpen == 0);
    LISPASSERT(aOther.iFunctions.Size() == 0);

    LISPASSERT(iFileToOpen == 0);
    LISPASSERT(iFunctions.Size() == 0);
    return *this;
  }

  /// Return user function with given arity.
  LispUserFunction* UserFunc(LispInt aArity);

  /// Destructor.
  virtual ~LispMultiUserFunction();

  /// Specify that some argument should be held.
  virtual void HoldArgument(LispString * aVariable);

  /// Add another LispArityUserFunction to #iFunctions.
  virtual void DefineRuleBase(LispArityUserFunction* aNewFunction);

  /// Delete tuser function with given arity.
  virtual void DeleteBase(LispInt aArity);

private:
  /// Set of LispArityUserFunction's provided by this LispMultiUserFunction.
  CDeletingArrayGrower<LispArityUserFunction*, ArrOpsDeletingPtr<LispArityUserFunction> > iFunctions;

public:
  /// File to read for the definition of this function.
  LispDefFile* iFileToOpen;
};


/// Associated hash of LispMultiUserFunction objects.

class LispUserFunctions : public LispAssociatedHash<LispMultiUserFunction>
{
};


#endif

