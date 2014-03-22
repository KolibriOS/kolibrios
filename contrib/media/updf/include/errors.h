
#ifndef __errors_h__
#define __errors_h__

void CheckArgType(LispInt aArgNr, LispPtr& aArguments, LispEnvironment& aEnvironment, LispInt aError);
#define CHK_ARG(_pred,_argnr)           {if (!(_pred))                              CheckArgType(_argnr,aArguments, aEnvironment,KLispErrInvalidArg);}
#define CHK_ARG_CORE(_pred,_argnr)      {if (!(_pred))                              CheckArgType(_argnr,ARGUMENT(0),aEnvironment,KLispErrInvalidArg);}
#define CHK_ISLIST(_pred,_argnr)        {if (!InternalIsList(_pred))                     CheckArgType(_argnr,aArguments, aEnvironment,KLispErrNotList);}
#define CHK_ISLIST_CORE(_pred,_argnr)   {if (!InternalIsList(_pred))                     CheckArgType(_argnr,ARGUMENT(0),aEnvironment,KLispErrNotList);}
#define CHK_ISSTRING(_pred,_argnr)      {if (!InternalIsString((_pred)->String())) CheckArgType(_argnr,aArguments, aEnvironment,KLispErrNotString);}
#define CHK_ISSTRING_CORE(_pred,_argnr) {if (!InternalIsString((_pred)->String())) CheckArgType(_argnr,ARGUMENT(0),aEnvironment,KLispErrNotString);}

void CheckNrArgs(LispInt n, LispPtr& aArguments, LispEnvironment& aEnvironment);
#define TESTARGS(_n)  CheckNrArgs(_n,aArguments,aEnvironment)

void CheckFuncGeneric(LispInt aError,LispPtr& aArguments,LispEnvironment& aEnvironment);
void CheckFuncGeneric(LispInt aError,LispEnvironment& aEnvironment);
#define CHK(_pred,_err)      {if (!(_pred)) CheckFuncGeneric(_err,aArguments, aEnvironment);}
#define CHK_CORE(_pred,_err) {if (!(_pred)) CheckFuncGeneric(_err,ARGUMENT(0),aEnvironment);}
#define CHK2(_pred,_err)     {if (!(_pred)) CheckFuncGeneric(_err,aEnvironment);}

char *GenericErrorBuf(); // called (only) from errors.cpp and LispEnvironment::ErrorString()
void RaiseError(char* str,...);

/*
#define TESTARGS(_n)  \
    { \
      LispInt nrArguments = InternalListLength(aArguments);  \
      Check(nrArguments == _n,KLispErrWrongNumberOfArgs); \
    }
*/

#endif
