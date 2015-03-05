/** \file lispenvironment.h
 *  General environment access.
 *
 */


#ifndef __lispenvironment_h__
#define __lispenvironment_h__

#include "yacasbase.h"
#include "lispobject.h"
#include "lisphash.h"
#include "lispevalhash.h"
#include "lispcleanupstack.h"
#include "deffile.h"
#include "lispio.h"
#include "stringio.h"
#include "lispglobals.h"
#include "xmltokenizer.h"
#include "errors.h"

class CCompressedArchive; /* defined in archiver.h */

class LispDefFiles;
class InputDirectories : public CDeletingArrayGrower<LispString *, ArrOpsDeletingPtr<LispString> >
{
};

class LispInput;
class LispOutput;
class LispPrinter;
class LispOperators;
class LispUserFunctions;
class LispUserFunction;
class LispMultiUserFunction;
class LispEvaluatorBase;
class BasicEvaluator;
class DefaultDebugger;
class LispEnvironment;


/// The Lisp environment.
/// This huge class is the central class of the Yacas program. It
/// implements a dialect of Lisp.

class LispEnvironment : public YacasBase
{
public:
  /// \name Constructor and destructor
  //@{
  LispEnvironment(YacasCoreCommands &aCoreCommands,
                  LispUserFunctions& aUserFunctions,
                  LispGlobal& aGlobals,
                  LispHashTable& aHashTable,
                  LispOutput* aOutput,
                  LispPrinter& aPrinter,
                  LispOperators &aPreFixOperators,
                  LispOperators &aInFixOperators,
                  LispOperators &aPostFixOperators,
                  LispOperators &aBodiedOperators,
                  LispInput*    aCurrentInput,
                  LispInt aStackSize);
  ~LispEnvironment();
  //@}

public:
  /// \name Lisp variables
  //@{

  /// Assign a value to a Lisp variable.
  /// \param aString name of the variable
  /// \param aValue value to be assigned to \a aString
  ///
  /// If there is a local variable with the name \a aString, the
  /// object \a aValue is assigned to it. Otherwise, a
  /// LispGlobalVariable is constructed, and it is associated with
  /// \a aValue in #iGlobals.
  /// \sa FindLocal
  void SetVariable(LispString * aString, LispPtr& aValue, LispBoolean aGlobalLazyVariable);

  /// In debug mode, DebugModeVerifySettingGlobalVariables raises a warning if a global variable is set.
  void DebugModeVerifySettingGlobalVariables(LispPtr & aVariable, LispBoolean aGlobalLazyVariable);

  /// Get the value assigned to a variable.
  /// \param aVariable name of the variable
  /// \param aResult (on exit) value of \a aVariable
  ///
  /// - If there is a local variable with the name \a aString,
  ///   \a aResult is set to point to the value assigned to this local
  ///   variable.
  /// - If there is a global variable \a aString and its
  ///   #iEvalBeforeReturn is false, its value is returned via
  ///   \a aResult.
  /// - If there is a global variable \a aString and its
  ///   #iEvalBeforeReturn is true, its value is evaluated. The
  ///   result is assigned back to the variable, its
  ///   #iEvalBeforeReturn is set to false, and a copy of the result
  ///   is returned in \a aResult.
  /// - Otherwise, \a aResult is set to #NULL.
  void GetVariable(LispString * aVariable,LispPtr& aResult);

  void UnsetVariable(LispString * aString);
  void PushLocalFrame(LispBoolean aFenced);
  void PopLocalFrame();
  void NewLocal(LispString * aVariable,LispObject* aValue);
  void CurrentLocals(LispPtr& aResult);
  //@}

public:
  /// \name Lisp functions
  //@{

  /// Return the #iCoreCommands attribute.
  inline YacasCoreCommands& CoreCommands();

  /// Add a command to the list of core commands.
  /// \param aEvaluatorFunc C function evaluating the core command
  /// \param aString name of the command
  /// \param aNrArgs number of arguments
  /// \param aFlags flags, see YacasEvaluator::FunctionFlags
  void SetCommand(YacasEvalCaller aEvaluatorFunc, LispChar * aString,LispInt aNrArgs,LispInt aFlags);

  void RemoveCommand(LispChar * aString);
  void RemoveCoreCommand(LispChar * aString);

  inline  LispHashTable& HashTable();
  LispUserFunction* UserFunction(LispPtr& aArguments);
  LispUserFunction* UserFunction(LispString * aName,LispInt aArity);

  /// Return LispMultiUserFunction with given name.
  /// \param aArguments name of the multi user function
  ///
  /// The table of user functions, #iUserFunctions, is consulted. If
  /// a user function with the given name exists, it is returned.
  /// Otherwise, a new LispMultiUserFunction is constructed, added
  /// to #iUserFunctions, and returned.
  LispMultiUserFunction* MultiUserFunction(LispString * aArguments);

  LispDefFiles& DefFiles();
  void DeclareRuleBase(LispString * aOperator, LispPtr& aParameters,
                       LispInt aListed);
  void DeclareMacroRuleBase(LispString * aOperator, LispPtr& aParameters,
                       LispInt aListed);
  void DefineRule(LispString * aOperator,LispInt aArity,
                          LispInt aPrecedence, LispPtr& aPredicate,
                          LispPtr& aBody);
  void DefineRulePattern(LispString * aOperator,LispInt aArity,
                          LispInt aPrecedence, LispPtr& aPredicate,
                          LispPtr& aBody);


  void UnFenceRule(LispString * aOperator,LispInt aArity);
  void Retract(LispString * aOperator,LispInt aArity);
  void HoldArgument(LispString *  aOperator,LispString * aVariable);
  //@}

  LispString * FindCachedFile(LispChar * aFileName);

public:
  /// \name Precision
  //@{

  /// set precision to a given number of decimal digits
  void SetPrecision(LispInt aPrecision);
  inline LispInt Precision(void) const;
  inline LispInt BinaryPrecision(void) const;
  //@}

public:
  inline void SetPrettyPrinter(LispString * aPrettyPrinter);
  inline LispString * PrettyPrinter(void);

  inline void SetPrettyReader(LispString * aPrettyReader);
  inline LispString * PrettyReader(void);

public:
  LispInt GetUniqueId();
public:
  LispPrinter& CurrentPrinter();

public:
  /// \name Operators
  //@{
  LispOperators& PreFix();
  LispOperators& InFix();
  LispOperators& PostFix();
  LispOperators& Bodied();
  //@}

public:
  /// \name Input and output
  //@{
  LispInput* CurrentInput();
  void SetCurrentInput(LispInput* aInput);
public:
  LispOutput* CurrentOutput();
  void SetCurrentOutput(LispOutput* aOutput);
public:
  void SetUserError(LispChar * aErrorString);
  LispChar * ErrorString(LispInt aError);
  //@}

protected:
  /// current precision for user interaction, in decimal and in binary
  LispInt iPrecision;
  LispInt iBinaryPrecision;
public:
  InputDirectories iInputDirectories;
  DeletingLispCleanup iCleanup;
  LispInt iEvalDepth;
  LispInt iMaxEvalDepth;
  CCompressedArchive *iArchive;
  LispEvaluatorBase* iEvaluator;

public: // Error information when some error occurs.
  InputStatus iInputStatus;
  LispInt iSecure;
public: // pre-found
  RefPtr<LispObject> iTrue;
  RefPtr<LispObject> iFalse;

  RefPtr<LispObject> iEndOfFile;
  RefPtr<LispObject> iEndStatement;
  RefPtr<LispObject> iProgOpen;
  RefPtr<LispObject> iProgClose;
  RefPtr<LispObject> iNth;
  RefPtr<LispObject> iBracketOpen;
  RefPtr<LispObject> iBracketClose;
  RefPtr<LispObject> iListOpen;
  RefPtr<LispObject> iListClose;
  RefPtr<LispObject> iComma;
  RefPtr<LispObject> iList;
  RefPtr<LispObject> iProg;

  LispInt iLastUniqueId;

public: // Error reporting
  LispString iError;
  StringOutput iErrorOutput;
  DefaultDebugger* iDebugger;

private:
  LispPtr *FindLocal(LispString * aVariable);

private:

  class LispLocalVariable : public YacasBase
  {
  public:
    LispLocalVariable(LispString * aVariable,
                      LispObject* aValue)
      : iNext(NULL), iVariable(aVariable),iValue(aValue)
    {
      ++aVariable->iReferenceCount;
    };
    ~LispLocalVariable()
    {
      --iVariable->iReferenceCount;
    }
  private:
    LispLocalVariable(const LispLocalVariable& aOther) : iNext(NULL), iVariable(NULL),iValue(NULL)
    {
      // copy constructor not written yet, hence the assert
      LISPASSERT(0);
    }
    LispLocalVariable& operator=(const LispLocalVariable& aOther)
    {
      // copy constructor not written yet, hence the assert
      LISPASSERT(0);
      return *this;
    }

  public:
    LispLocalVariable* iNext;
    LispString * iVariable;
    LispPtr iValue;
  };
  class LocalVariableFrame : public YacasBase
  {
  public:
    LocalVariableFrame(LocalVariableFrame *aNext,
                       LispLocalVariable* aFirst)
        : iNext(aNext), iFirst(aFirst), iLast(aFirst) { }
    void Add(LispLocalVariable* aNew)
    {
      aNew->iNext = iFirst;
      iFirst = aNew;
    }
    ~LocalVariableFrame()
    {
      LispLocalVariable* t = iFirst;
      LispLocalVariable* next;
      while (t != iLast)
      {
        next = t->iNext;
        delete t;
        t = next;
      }
    }

  private:
    LocalVariableFrame(const LocalVariableFrame& aOther) : iNext(NULL),iFirst(NULL),iLast(NULL)
    {
      // copy constructor not written yet, hence the assert
      LISPASSERT(0);
    }
    LocalVariableFrame& operator=(const LocalVariableFrame& aOther)
    {
      // copy constructor not written yet, hence the assert
      LISPASSERT(0);
      return *this;
    }
  public:
    LocalVariableFrame *iNext;
    LispLocalVariable* iFirst;
    LispLocalVariable* iLast;
  };
public: //Well... only because I want to be able to show the stack to the outside world...
  LocalVariableFrame *iLocalsList;
  LispOutput*    iInitialOutput;
private:

  /// Hash of core commands with associated YacasEvaluator
  YacasCoreCommands& iCoreCommands;

  LispUserFunctions& iUserFunctions;
  LispHashTable& iHashTable;
  LispDefFiles   iDefFiles;
  LispPrinter&   iPrinter;
  LispOutput*    iCurrentOutput;

  /// Hash of global variables with their values
  LispGlobal&    iGlobals;

  LispOperators& iPreFixOperators;
  LispOperators& iInFixOperators;
  LispOperators& iPostFixOperators;
  LispOperators& iBodiedOperators;

  LispInput* iCurrentInput;

  LispChar * theUserError;

  LispString * iPrettyReader;
  LispString * iPrettyPrinter;
public:
  LispTokenizer iDefaultTokenizer;
  CommonLispTokenizer iCommonLispTokenizer;
  XmlTokenizer  iXmlTokenizer;
  LispTokenizer* iCurrentTokenizer;

public:
  /** YacasArgStack implements a stack of pointers to objects that can be used to pass
  *  arguments to functions, and receive results back.
  */
  class YacasArgStack
  {
  public:
    YacasArgStack(LispInt aStackSize) : iStack(),iStackCnt(0)
    {
      iStack.ResizeTo( aStackSize );
    }
    inline LispInt GetStackTop() const {return iStackCnt;}
    inline void RaiseStackOverflowError() const
    {
      RaiseError("Argument stack reached maximum. Please extend argument stack with --stack argument on the command line.");
    }
    inline void PushArgOnStack(LispObject* aObject)
    {
      if (iStackCnt >= iStack.Size())
      {
        RaiseStackOverflowError();
      }
      //LISPASSERT(iStackCnt>=0 /*&& iStackCnt<iStack.Size()*/);
      iStack[iStackCnt] = (aObject);
      iStackCnt++;
    }
    inline void PushNulls(LispInt aNr)
    {
      LispInt aStackCnt = iStackCnt + aNr;
      if (aStackCnt > iStack.Size() || aStackCnt < 0)
      {
        RaiseStackOverflowError();
      }
      iStackCnt = aStackCnt;
    }
    inline LispPtr& GetElement(LispInt aPos)
    {
      LISPASSERT(0<=aPos && aPos<iStackCnt);
      //LISPASSERT(aPos>=0 && aPos<iStack.Size());
      return iStack[aPos];
    }
    inline void PopTo(LispInt aTop)
    {
      LISPASSERT(0<=aTop && aTop<=iStackCnt);
      while (iStackCnt>aTop)
      {
        iStackCnt--;
        iStack[iStackCnt] = (NULL);
      }
    }
  protected:
    // Invariants:
    //    0 <= iStackCnt <= iStack.Size()
    //    iStack[iStackCnt..iStack.Size()-1] = NULL
    LispPtrArray iStack;
    LispInt iStackCnt;    // number of items on the stack
  };
  YacasArgStack iStack;

private:
 
  inline LispEnvironment(const LispEnvironment& aOther)
    :
    iPrecision(0),  // default user precision of 10 decimal digits
    iBinaryPrecision(0),  // same as 34 bits
    iInputDirectories(),
    iCleanup(),
    iEvalDepth(0),
    iMaxEvalDepth(0),
    iArchive(NULL),
    iEvaluator(NULL),
    iInputStatus(),
    iSecure(0),
    iTrue(),
    iFalse(),
    iEndOfFile(),
    iEndStatement(),
    iProgOpen(),
    iProgClose(),
    iNth(),
    iBracketOpen(),
    iBracketClose(),
    iListOpen(),
    iListClose(),
    iComma(),
    iList(),
    iProg(),
    iLastUniqueId(0),
    iError(),
    iErrorOutput(aOther.iErrorOutput),
    iDebugger(NULL),
    iLocalsList(NULL),
    iInitialOutput(aOther.iInitialOutput),
    iCoreCommands(aOther.iCoreCommands),
    iUserFunctions(aOther.iUserFunctions),
    iHashTable(aOther.iHashTable),
    iDefFiles(),
    iPrinter(aOther.iPrinter),
    iCurrentOutput(aOther.iCurrentOutput),
    iGlobals(aOther.iGlobals),
    iPreFixOperators(aOther.iPreFixOperators),
    iInFixOperators(aOther.iInFixOperators),
    iPostFixOperators(aOther.iPostFixOperators),
    iBodiedOperators(aOther.iBodiedOperators),
    iCurrentInput(aOther.iCurrentInput),
    theUserError(NULL),
    iPrettyReader(NULL),
    iPrettyPrinter(NULL),
    iDefaultTokenizer(),
    iCommonLispTokenizer(),
    iXmlTokenizer(),
    iCurrentTokenizer(NULL),
    iStack(0)
  {
    // copy constructor has not been made yet, hence the assert
    LISPASSERT(0);
  }
  LispEnvironment& operator=(const LispEnvironment& aOther)
  {
    // copy constructor has not been made yet, hence the assert
    LISPASSERT(0);
    return *this;
  }

};

inline LispInt LispEnvironment::Precision(void) const
{
    return iPrecision;
}

inline LispInt LispEnvironment::BinaryPrecision(void) const
{
  return iBinaryPrecision;
}



inline YacasCoreCommands& LispEnvironment::CoreCommands()
{
    return iCoreCommands;
}


inline LispHashTable& LispEnvironment::HashTable()
{
    return iHashTable;
}



// Local lisp stack, unwindable by the exception handler
class LispLocalFrame : public LispBase
{
public:
    LispLocalFrame(LispEnvironment& aEnvironment, LispBoolean aFenced)
        : iEnvironment(aEnvironment)
    {
        iEnvironment.PushLocalFrame(aFenced);
        SAFEPUSH(iEnvironment,*this);
    };
    virtual ~LispLocalFrame()
    {
        SAFEPOP(iEnvironment);
        Delete();
    };
    virtual void Delete();
private:
    LispEnvironment& iEnvironment;
};



class LispSecureFrame : public LispBase
{
public:
  LispSecureFrame(LispEnvironment& aEnvironment)
      : iEnvironment(aEnvironment),iPreviousSecure(aEnvironment.iSecure)
  {
    iEnvironment.iSecure = 1;
    SAFEPUSH(iEnvironment,*this);
  };
  virtual ~LispSecureFrame()
  {
    SAFEPOP(iEnvironment);
    Delete();
  };
  virtual void Delete();
private:
  LispEnvironment& iEnvironment;
  LispInt iPreviousSecure;
};


// LispLocalInput takes ownership over the LispInput class
class LispLocalInput : public LispBase
{
public:
  LispLocalInput(LispEnvironment& aEnvironment, LispInput* aInput)
      : iEnvironment(aEnvironment),iPreviousInput(iEnvironment.CurrentInput())
  {
    iEnvironment.SetCurrentInput(aInput);
    SAFEPUSH(iEnvironment,*this);
  };
  virtual ~LispLocalInput()
  {
    SAFEPOP(iEnvironment);
    Delete();
  };
  virtual void Delete();
private:
  LispLocalInput(const LispLocalInput& aOther): iEnvironment(aOther.iEnvironment),iPreviousInput(iEnvironment.CurrentInput())
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  };
  LispLocalInput& operator=(const LispLocalInput& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  };

private:
  LispEnvironment& iEnvironment;
  LispInput* iPreviousInput;
};


// LispLocalInput takes ownership over the LispInput class
class LispLocalOutput : public LispBase
{
public:
  LispLocalOutput(LispEnvironment& aEnvironment, LispOutput* aOutput)
      : iEnvironment(aEnvironment), iPreviousOutput(iEnvironment.CurrentOutput())
  {
    iPreviousOutput = iEnvironment.CurrentOutput();
    iEnvironment.SetCurrentOutput(aOutput);
    SAFEPUSH(iEnvironment,*this);
  };
  virtual ~LispLocalOutput()
  {
    SAFEPOP(iEnvironment);
    Delete();
  };
  virtual void Delete();
private:
  LispLocalOutput(const LispLocalOutput& aOther): iEnvironment(aOther.iEnvironment), iPreviousOutput(iEnvironment.CurrentOutput())
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  }
  LispLocalOutput& operator=(const LispLocalOutput& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
private:
  LispEnvironment& iEnvironment;
  LispOutput* iPreviousOutput;
};


class LispLocalEvaluator : public YacasBase
{
public:
  LispLocalEvaluator(LispEnvironment& aEnvironment,LispEvaluatorBase* aNewEvaluator);
  ~LispLocalEvaluator();

private:
  LispLocalEvaluator(const LispLocalEvaluator& aOther) : iPreviousEvaluator(NULL), iEnvironment(aOther.iEnvironment)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  }
  LispLocalEvaluator& operator=(const LispLocalEvaluator& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }

private:
  LispEvaluatorBase* iPreviousEvaluator;
  LispEnvironment& iEnvironment;
};

class LispLocalTrace : public YacasBase
{
public:
  LispLocalTrace(LispUserFunction* aUserFunc);
  ~LispLocalTrace();
private:
  LispLocalTrace(const LispLocalTrace& aOther) : iUserFunc(NULL)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  }
  LispLocalTrace& operator=(const LispLocalTrace& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
private:
  LispUserFunction* iUserFunc;
};

class LocalArgs : public YacasBase
{
public:
  LocalArgs(LispPtr* aPtrs) : iPtrs(aPtrs) {};
  ~LocalArgs()
  {
    if (iPtrs)
      delete[] iPtrs;
  }
private:
  LocalArgs(const LocalArgs& aOther) : iPtrs(NULL)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  }
  LocalArgs& operator=(const LocalArgs& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
private:
  LispPtr* iPtrs;
};



inline void LispEnvironment::SetPrettyReader(LispString * aPrettyReader)
{
  iPrettyReader = aPrettyReader;
}
inline LispString * LispEnvironment::PrettyReader(void)
{
  return iPrettyReader;
}

inline void LispEnvironment::SetPrettyPrinter(LispString * aPrettyPrinter)
{
  iPrettyPrinter = aPrettyPrinter;
}
inline LispString * LispEnvironment::PrettyPrinter(void)
{
  return iPrettyPrinter;
}


#endif


