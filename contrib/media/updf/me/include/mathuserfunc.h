
#ifndef __mathuserfunc_h__
#define __mathuserfunc_h__

#include "yacasbase.h"
#include "lispuserfunc.h"
#include "grower.h"

#include "patternclass.h"

/// A mathematical function defined by several rules.
/// This is the basic class which implements functions in Yacas.
/// Evaluation is done by consulting a set of rewriting rules. The
/// body of the first rule that matches, is evaluated and this gives
/// the result of evaluating the function.

class BranchingUserFunction : public LispArityUserFunction
{
public:
  /// Structure containing name of parameter and whether it is put on hold.
  class BranchParameter : public YacasBase
  {
  public:
    BranchParameter(LispString * aParameter = NULL, LispInt aHold=LispFalse)
        : iParameter(aParameter), iHold(aHold) {}
    LispString * iParameter;
    LispInt       iHold;
  };

  /// Abstract base class for rules.
  class BranchRuleBase : public YacasBase
  {
  public:
    virtual ~BranchRuleBase();
    virtual LispBoolean Matches(LispEnvironment& aEnvironment, LispPtr* aArguments) = 0;
    virtual LispInt Precedence() const = 0;
    virtual LispPtr& Body() = 0;
  };

  /// A rule with a predicate.
  /// This rule matches if the predicate evaluates to #LispTrue.
  class BranchRule : public BranchRuleBase
  {
  public:
    virtual ~BranchRule();
    BranchRule(LispInt aPrecedence,LispPtr& aPredicate,LispPtr& aBody) : iPrecedence(aPrecedence),iBody(aBody),iPredicate(aPredicate)
    {
    }

    /// Return true if the rule matches.
    /// #iPredicate is evaluated in \a Environment. If the result
    /// IsTrue(), this function returns true.
    virtual LispBoolean Matches(LispEnvironment& aEnvironment, LispPtr* aArguments);

    /// Access #iPrecedence.
    virtual LispInt Precedence() const;

    /// Access #iBody.
    virtual LispPtr& Body();
  protected:
    BranchRule() : iPrecedence(0),iBody(),iPredicate() {};
  protected:
    LispInt iPrecedence;
    LispPtr iBody;
    LispPtr iPredicate;
  };

  /// A rule that always matches.
  class BranchRuleTruePredicate : public BranchRule
  {
  public:
    BranchRuleTruePredicate(LispInt aPrecedence,LispPtr& aBody)
    {
      iPrecedence = aPrecedence;
      iBody = (aBody);
    }
    /// Return #LispTrue, always.
    virtual LispBoolean Matches(LispEnvironment& aEnvironment, LispPtr* aArguments);
  };

  /// A rule which matches if the corresponding PatternClass matches.
  class BranchPattern : public BranchRuleBase
  {
  public:
    /// Destructor.
    /// This function contains no code.
    virtual ~BranchPattern();

    /// Constructor.
    /// \param aPrecedence precedence of the rule
    /// \param aPredicate generic object of type \c Pattern
    /// \param aBody body of the rule
    BranchPattern(LispInt aPrecedence,LispPtr& aPredicate,LispPtr& aBody) : iPrecedence(aPrecedence),iBody(aBody),iPredicate(aPredicate),iPatternClass(NULL)
    {
      GenericClass *gen = aPredicate->Generic();
      DYNCAST(PatternClass,"\"Pattern\"",pat,gen)
      Check(pat,KLispErrInvalidArg);
      iPatternClass = pat;
    }

    /// Return true if the corresponding pattern matches.
    virtual LispBoolean Matches(LispEnvironment& aEnvironment, LispPtr* aArguments);

    /// Access #iPrecedence
    virtual LispInt Precedence() const;

    /// Access #iBody
    virtual LispPtr& Body();

  private:
    BranchPattern(const BranchPattern& aOther) : iPrecedence(0),iBody(),iPredicate(),iPatternClass(NULL)
    {
      // copy constructor not written yet, hence the assert
      LISPASSERT(0);
    }
    BranchPattern& operator=(const BranchPattern& aOther)
    {
      // copy constructor not written yet, hence the assert
      LISPASSERT(0);
      return *this;
    }

  protected:
    /// The precedence of this rule.
    LispInt iPrecedence;

    /// The body of this rule.
    LispPtr iBody;

    /// Generic object of type \c Pattern containing #iPatternClass
    LispPtr iPredicate;

    /// The pattern that decides whether this rule matches.
    PatternClass *iPatternClass;
  };

  /// Constructor.
  /// \param aParameters linked list constaining the names of the arguments
  ///
  /// #iParamList and #iParameters are set from \a aParameters.
  BranchingUserFunction(LispPtr& aParameters);

  /// Destructor.
  /// There is no code inside this function.
  virtual ~BranchingUserFunction();

  /// Evaluate the function on given arguments.
  /// \param aResult (on output) the result of the evaluation
  /// \param aEnvironment the underlying Lisp environment
  /// \param aArguments the arguments to the function
  ///
  /// First, all arguments are evaluated by the evaluator associated
  /// to \a aEnvironment, unless the \c iHold flag of the
  /// corresponding parameter is true. Then a new LispLocalFrame is
  /// constructed, in which the actual arguments are assigned to the
  /// names of the formal arguments, as stored in \c iParameter. Then
  /// all rules in #iRules are tried one by one. The body of the
  /// first rule that matches is evaluated, and the result is put in
  /// \a aResult. If no rule matches, \a aResult will recieve a new
  /// expression with evaluated arguments.
  virtual void Evaluate(LispPtr& aResult,LispEnvironment& aEnvironment, LispPtr& aArguments);

  /// Put an argument on hold.
  /// \param aVariable name of argument to put un hold
  ///
  /// The \c iHold flag of the corresponding argument is set. This
  /// implies that this argument is not evaluated by Evaluate().
  virtual void HoldArgument(LispString * aVariable);

  /// Return true if the arity of the function equals \a aArity.
  virtual LispInt IsArity(LispInt aArity) const;

  /// Return the arity (number of arguments) of the function.
  LispInt Arity() const;

  /// Add a BranchRule to the list of rules.
  /// \sa InsertRule()
  virtual void DeclareRule(LispInt aPrecedence, LispPtr& aPredicate, LispPtr& aBody);

  /// Add a BranchRuleTruePredicate to the list of rules.
  /// \sa InsertRule()
  virtual void DeclareRule(LispInt aPrecedence, LispPtr& aBody);

  /// Add a BranchPattern to the list of rules.
  /// \sa InsertRule()
  void DeclarePattern(LispInt aPrecedence, LispPtr& aPredicate, LispPtr& aBody);

  /// Insert any BranchRuleBase object in the list of rules.
  /// This function does the real work for DeclareRule() and
  /// DeclarePattern(): it inserts the rule in #iRules, while
  /// keeping it sorted. The algorithm is \f$O(\log n)\f$, where
  /// \f$n\f$ denotes the number of rules.
  void InsertRule(LispInt aPrecedence,BranchRuleBase* newRule);

  /// Return the argument list, stored in #iParamList
  virtual LispPtr& ArgList();

protected:
  /// List of arguments, with corresponding \c iHold property.
  CArrayGrower<BranchParameter, ArrOpsPOD<BranchParameter> > iParameters;

  /// List of rules, sorted on precedence.
  CDeletingArrayGrower<BranchRuleBase*, ArrOpsDeletingPtr<BranchRuleBase> >     iRules;

  /// List of arguments
  LispPtr iParamList;
};

class ListedBranchingUserFunction : public BranchingUserFunction
{
public:
  ListedBranchingUserFunction(LispPtr& aParameters);
  virtual LispInt IsArity(LispInt aArity) const;
  virtual void Evaluate(LispPtr& aResult,LispEnvironment& aEnvironment, LispPtr& aArguments);
};


class MacroUserFunction : public BranchingUserFunction
{
public:
  MacroUserFunction(LispPtr& aParameters);
  virtual void Evaluate(LispPtr& aResult,LispEnvironment& aEnvironment, LispPtr& aArguments);
};


class ListedMacroUserFunction : public MacroUserFunction
{
public:
  ListedMacroUserFunction(LispPtr& aParameters);
  virtual LispInt IsArity(LispInt aArity) const;
  virtual void Evaluate(LispPtr& aResult,LispEnvironment& aEnvironment, LispPtr& aArguments);
};




#endif

