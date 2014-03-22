/** \file lispeval.h
 *  Evaluation of expressions.
 *
 */

#ifndef __lispeval_h__
#define __lispeval_h__

#include "yacasbase.h"
#include "lispobject.h"
#include "lispenvironment.h"

/*
void InternalEval(LispEnvironment& aEnvironment, LispPtr& aResult, LispPtr& aExpression);
*/


class UserStackInformation : public YacasBase
{
public:
    UserStackInformation()
      : iOperator(),iExpression(),iRulePrecedence(-1),iSide(0)
#ifdef YACAS_DEBUG
      , iFileName("(no file)"),iLine(0)
#endif // YACAS_DEBUG
    {
    }
    LispPtr iOperator;
    LispPtr iExpression;
    LispInt iRulePrecedence;
    LispInt iSide; // 0=pattern, 1=body
    DBG_( LispChar * iFileName; )
    DBG_( LispInt iLine; )
};

/// Abstract evaluator for Lisp expressions.
/// Eval() is a pure virtual function, to be provided by the derived class.
/// The other functions are stubs.

class LispEvaluatorBase : public YacasBase
{
public:
  LispEvaluatorBase() : iBasicInfo() {}
  virtual ~LispEvaluatorBase();
  virtual void Eval(LispEnvironment& aEnvironment, LispPtr& aResult, LispPtr& aExpression)=0;
  virtual void ResetStack();
  virtual UserStackInformation& StackInformation();
  virtual void ShowStack(LispEnvironment& aEnvironment, LispOutput& aOutput);
private:
  UserStackInformation iBasicInfo;
};

/// The basic evaluator for Lisp expressions.

class BasicEvaluator : public LispEvaluatorBase
{
public:
  /// Evaluate a Lisp expression
  /// \param aEnvironment the Lisp environment, in which the
  /// evaluation should take place.
  /// \param aResult the result of the evaluation.
  /// \param aExpression the expression to evaluate.
  ///
  /// First, the evaluation depth is checked. An error is raised if
  /// the maximum evaluation depth is exceeded.
  ///
  /// The next step is the actual evaluation. \a aExpression is a
  /// LispObject, so we can distinguish three cases.
  ///   - If \a aExpression is a string starting with \c " , it is
  ///     simply copied in \a aResult. If it starts with another
  ///     character (this includes the case where it represents a
  ///     number), the environment is checked to see whether a
  ///     variable with this name exists. If it does, its value is
  ///     copied in \a aResult, otherwise \a aExpression is copied.
  ///   - If \a aExpression is a list, the head of the list is
  ///     examined. If the head is not a string. InternalApplyPure()
  ///     is called. If the head is a string, it is checked against
  ///     the core commands; if there is a check, the corresponding
  ///     evaluator is called. Then it is checked agaist the list of
  ///     user function with GetUserFunction() . Again, the
  ///     corresponding evaluator is called if there is a check. If
  ///     all fails, ReturnUnEvaluated() is called.
  ///   - Otherwise (ie. if \a aExpression is a generic object), it is
  ///     copied in \a aResult.
  ///
  /// \note The result of this operation must be a unique (copied)
  /// element! Eg. its Next might be set...
  ///
  /// The LispPtr it can be stored in to is passed in as argument, so it
  /// does not need to be constructed by the calling environment.
  virtual void Eval(LispEnvironment& aEnvironment, LispPtr& aResult, LispPtr& aExpression);
  virtual ~BasicEvaluator();
};

class TracedEvaluator : public BasicEvaluator
{
public:
  TracedEvaluator() :  BasicEvaluator(),errorStr(),errorOutput(errorStr){}
  virtual void Eval(LispEnvironment& aEnvironment, LispPtr& aResult, LispPtr& aExpression);
protected:
  LispString errorStr;
  StringOutput errorOutput;
};


class TracedStackEvaluator : public BasicEvaluator
{
public:
  TracedStackEvaluator() : objs() {}
  virtual ~TracedStackEvaluator();
  virtual void Eval(LispEnvironment& aEnvironment, LispPtr& aResult, LispPtr& aExpression);
  virtual void ResetStack();
  virtual UserStackInformation& StackInformation();
  virtual void ShowStack(LispEnvironment& aEnvironment, LispOutput& aOutput);
private:
  void PushFrame();
  void PopFrame();
private:
  CArrayGrower<UserStackInformation*, ArrOpsCustomPtr<UserStackInformation> > objs;
};



/* GetUserFunction : get user function, possibly loading the required
   files to read in the function definition */
LispUserFunction* GetUserFunction(LispEnvironment& aEnvironment,
                                  LispPtr* subList);


/* Tracing functions */
void TraceShowEnter(LispEnvironment& aEnvironment,
                    LispPtr& aExpression);
void TraceShowLeave(LispEnvironment& aEnvironment, LispPtr& aResult,
                    LispPtr& aExpression);
void TraceShowArg(LispEnvironment& aEnvironment,LispPtr& aParam,
                  LispPtr& aValue);


void ShowExpression(LispString& outString, LispEnvironment& aEnvironment,
                    LispPtr& aExpression);


class YacasDebuggerBase : public YacasBase
{
public:
  virtual ~YacasDebuggerBase();
  virtual void Start() = 0;
  virtual void Finish() = 0;
  virtual void Enter(LispEnvironment& aEnvironment,
                     LispPtr& aExpression) = 0;
  virtual void Leave(LispEnvironment& aEnvironment, LispPtr& aResult,
                     LispPtr& aExpression) = 0;
  virtual void Error(LispEnvironment& aEnvironment) = 0;
  virtual LispBoolean Stopped() = 0;
};

class DefaultDebugger : public YacasDebuggerBase
{
public:
  inline DefaultDebugger(LispPtr& aEnter, LispPtr& aLeave, LispPtr& aError)
    : iEnter(aEnter), iLeave(aLeave), iError(aError), iTopExpr(),iTopResult(),iStopped(LispFalse),defaultEval() {};
  virtual void Start();
  virtual void Finish();
  virtual void Enter(LispEnvironment& aEnvironment,
                      LispPtr& aExpression);
  virtual void Leave(LispEnvironment& aEnvironment, LispPtr& aResult,
                      LispPtr& aExpression);
  virtual void Error(LispEnvironment& aEnvironment);
  virtual LispBoolean Stopped();
  LispPtr iEnter;
  LispPtr iLeave;
  LispPtr iError;
  LispPtr iTopExpr;
  LispPtr iTopResult;
  LispBoolean iStopped;
protected:
  BasicEvaluator defaultEval;
};


#endif



