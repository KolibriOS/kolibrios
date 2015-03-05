
#ifndef __arggetter_h__
#define __arggetter_h__

#include "yacasbase.h"



/// Get an argument that should be a (long) integer
LispString * GetIntegerArgument(LispEnvironment& aEnvironment, LispInt aStackTop, LispInt iArgNr);
/// Get a string (atom)
LispString * GetStringArgument(LispEnvironment& aEnvironment, LispInt aStackTop, LispInt iArgNr);
/// Get the atomic string of the argument
LispString * GetAtomArgument(LispEnvironment& aEnvironment, LispInt aStackTop, LispInt iArgNr);
/// Get an argument that should be a short integer
LispInt GetShortIntegerArgument(LispEnvironment& aEnvironment, LispInt aStackTop, LispInt iArgNr);
/// Get a list argument
void GetListArgument(LispPtr& aResult, LispEnvironment& aEnvironment, LispInt aStackTop, LispInt iArgNr);
/// Get a void* pointer to a struct encapsulated in a generic class
void* GetVoidStruct(LispEnvironment& aEnvironment, LispInt aStackTop, LispInt iArgNr, LispChar * aTypeString);


#define ListArgument(_list,_argnr) LispPtr _list; GetListArgument(_list, aEnvironment,aStackTop,_argnr)
#define IntegerArgument(_i,_argnr) LispString * _i = GetIntegerArgument(aEnvironment,aStackTop,_argnr)
#define ShortIntegerArgument(_i,_argnr) LispInt _i = GetShortIntegerArgument(aEnvironment,aStackTop,_argnr)
#define InpStringArgument(_i,_argnr) LispChar * _i = GetStringArgument(aEnvironment,aStackTop,_argnr)->c_str()

#define DoubleFloatArgument(_i,_argnr) double _i = GetDoubleFloatArgument(aEnvironment,aStackTop,_argnr)
#define VoidStructArgument(_typ,_i,_argnr,_name) _typ _i = (_typ)GetVoidStruct(aEnvironment,aStackTop,_argnr,_name)


void ReturnShortInteger(LispEnvironment& aEnvironment,
                        LispPtr& aResult, LispInt r);
void SetShortIntegerConstant(LispEnvironment& aEnvironment,
                                    LispChar * aName,
                                    LispInt aValue);
double GetDoubleFloatArgument(LispEnvironment& aEnvironment, LispInt aStackTop, LispInt iArgNr);
void ReturnDoubleFloat(LispEnvironment& aEnvironment,LispPtr& aResult, double r);
void ReturnVoidStruct(LispEnvironment& aEnvironment,
                      LispPtr& aResult,
                      LispChar * aName,
                      void* aData,
                      void (*aFree)(void*));


#endif

