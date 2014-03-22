
#ifndef __patterns_h__
#define __patterns_h__

/// \file
/// Pattern matching code.
///
/// General idea: have a class that can match function parameters
/// to a pattern, check for predicates on the arguments, and return
/// whether there was a match.
///
/// First the pattern is mapped onto the arguments. Then local variables
/// are set. Then the predicates are called. If they all return true,
/// Then the pattern matches, and the locals can stay (the body is expected
/// to use these variables).


#include "yacasbase.h"
#include "lisptype.h"
#include "grower.h"
#include "lispenvironment.h"


/// Abstract class for matching one argument to a pattern.
class YacasParamMatcherBase : public YacasBase
{
public:
    /// Destructor.
    /// This function contains no code.
    virtual ~YacasParamMatcherBase();

    /// Check whether some expression matches to the pattern.
    /// \param aEnvironment the underlying Lisp environment.
    /// \param aExpression the expression to test.
    /// \param arguments (input/output) actual values of the pattern
    /// variables for \a aExpression.
    virtual LispBoolean ArgumentMatches(LispEnvironment& aEnvironment,
                                        LispPtr& aExpression,
                                        LispPtr* arguments)=0;
};

/// Class for matching an expression to a given atom.
class MatchAtom : public YacasParamMatcherBase
{
public:
    MatchAtom(LispString * aString);
    virtual LispBoolean ArgumentMatches(LispEnvironment& aEnvironment,
                                        LispPtr& aExpression,
                                        LispPtr* arguments);
protected:
    LispString * iString;
};

/// Class for matching an expression to a given number.
class MatchNumber : public YacasParamMatcherBase
{
public:
    MatchNumber(BigNumber* aNumber);
    virtual LispBoolean ArgumentMatches(LispEnvironment& aEnvironment,
                                        LispPtr& aExpression,
                                        LispPtr* arguments);
protected:
    RefPtr<BigNumber> iNumber;
};

/// Class for matching against a list of YacasParamMatcherBase objects.
class MatchSubList : public YacasParamMatcherBase
{
public:
  MatchSubList(YacasParamMatcherBase** aMatchers, LispInt aNrMatchers);
  ~MatchSubList();
  virtual LispBoolean ArgumentMatches(LispEnvironment& aEnvironment,
                                      LispPtr& aExpression,
                                      LispPtr* arguments);
private:
  MatchSubList(const MatchSubList& aOther) : iMatchers(NULL),iNrMatchers(0)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  }
  MatchSubList& operator=(const MatchSubList& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
protected:
  YacasParamMatcherBase** iMatchers;
  LispInt iNrMatchers;
};

/// Class for matching against a pattern variable.
class MatchVariable : public YacasParamMatcherBase
{
public:
    MatchVariable(LispInt aVarIndex);

    /// Matches an expression against the pattern variable.
    /// \param aEnvironment the underlying Lisp environment.
    /// \param aExpression the expression to test.
    /// \param arguments (input/output) actual values of the pattern
    /// variables for \a aExpression.
    ///
    /// If entry #iVarIndex in \a arguments is still empty, the
    /// pattern matches and \a aExpression is stored in this
    /// entry. Otherwise, the pattern only matches if the entry equals
    /// \a aExpression.
    virtual LispBoolean ArgumentMatches(LispEnvironment& aEnvironment,
                                        LispPtr& aExpression,
                                        LispPtr* arguments);
protected:
    /// Index of variable in YacasPatternPredicateBase::iVariables.
    LispInt iVarIndex;
};


/// Class that matches function arguments to a pattern.
/// This class (specifically, the Matches() member function) can match
/// function parameters to a pattern, check for predicates on the
/// arguments, and return whether there was a match.

class YacasPatternPredicateBase : public YacasBase
{
public:
    /// Constructor.
    /// \param aEnvironment the underlying Lisp environment
    /// \param aPattern Lisp expression containing the pattern
    /// \param aPostPredicate Lisp expression containing the
    /// postpredicate
    ///
    /// The function MakePatternMatcher() is called for every argument
    /// in \a aPattern, and the resulting pattern matchers are
    /// collected in #iParamMatchers. Additionally, \a aPostPredicate
    /// is copied, and the copy is added to #iPredicates.
    YacasPatternPredicateBase(LispEnvironment& aEnvironment,
                              LispPtr& aPattern,
                              LispPtr& aPostPredicate);

    /// Destructor.
    /// This function contains no code.
    ~YacasPatternPredicateBase();

    /// Try to match the pattern against \a aArguments.
    /// First, every argument in \a aArguments is matched against the
    /// corresponding YacasParamMatcherBase in #iParamMatches. If any
    /// match fails, Matches() returns false. Otherwise, a temporary
    /// LispLocalFrame is constructed, then SetPatternVariables() and
    /// CheckPredicates() are called, and then the LispLocalFrame is
    /// immediately deleted. If CheckPredicates() returns false, this
    /// function also returns false. Otherwise, SetPatternVariables()
    /// is called again, but now in the current LispLocalFrame, and
    /// this function returns true.
    LispBoolean Matches(LispEnvironment& aEnvironment,
                        LispPtr& aArguments);

    /// Try to match the pattern against \a aArguments.
    /// This function does the same as Matches(LispEnvironment&,LispPtr&),
    /// but differs in the type of the arguments.
    LispBoolean Matches(LispEnvironment& aEnvironment,
                        LispPtr* aArguments);

protected:
    /// Construct a pattern matcher out of a Lisp expression.
    /// The result of this function depends on the value of \a aPattern:
    /// - If \a aPattern is a number, the corresponding MatchNumber is
    ///   constructed and returned.
    /// - If \a aPattern is an atom, the corresponding MatchAtom is
    ///   constructed and returned.
    /// - If \a aPattern is a list of the form <tt>( _ var )<tt>,
    ///   where \c var is an atom, LookUp() is called on \c var. Then
    ///   the correspoding MatchVariable is constructed and returned.
    /// - If \a aPattern is a list of the form <tt>( _ var expr )<tt>,
    ///   where \c var is an atom, LookUp() is called on \c var. Then,
    ///   \a expr is appended to #iPredicates. Finally, the
    ///   correspoding MatchVariable is constructed and returned.
    /// - If \a aPattern is a list of another form, this function
    ///   calls itself on any of the entries in this list. The
    ///   resulting YacasParamMatcherBase objects are collected in a
    ///   MatchSubList, which is returned.
    /// - Otherwise, this function returns #NULL.
    YacasParamMatcherBase* MakeParamMatcher(LispEnvironment& aEnvironment, LispObject* aPattern);

    /// Look up a variable name in #iVariables
    /// \returns index in #iVariables array where \a aVariable
    /// appears.
    ///
    /// If \a aVariable is not in #iVariables, it is added.
    LispInt LookUp(LispString * aVariable);

protected:
    /// Set local variables corresponding to the pattern variables.
    /// This function goes through the #iVariables array. A local
    /// variable is made for every entry in the array, and the
    /// corresponding argument is assigned to it.
    void SetPatternVariables(LispEnvironment& aEnvironment, LispPtr* arguments);

    /// Check whether all predicates are true.
    /// This function goes through all predicates in #iPredicates, and
    /// evaluates them. It returns #LispFalse if at least one
    /// of these results IsFalse(). An error is raised if any result
    /// neither IsTrue() nor IsFalse().
    LispBoolean CheckPredicates(LispEnvironment& aEnvironment);

protected:
    /// List of parameter matches, one for every parameter.
    CDeletingArrayGrower<YacasParamMatcherBase*, ArrOpsDeletingPtr<YacasParamMatcherBase> > iParamMatchers;

    /// List of variables appearing in the pattern.
    CArrayGrower<LispString *, ArrOpsCustomPtr<LispString> > iVariables;

    /// List of predicates which need to be true for a match.
    CArrayGrower<LispPtr, ArrOpsCustomObj<LispPtr> > iPredicates;
};


#endif
