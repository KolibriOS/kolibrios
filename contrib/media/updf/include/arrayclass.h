
#ifndef __arrayclass_h__
#define __arrayclass_h__

#include "yacasbase.h"
#include "lispobject.h"
#include "genericobject.h"

class ArrayClass : public GenericClass
{
public: //required
    ArrayClass(LispInt aSize,LispObject* aInitialItem);
    virtual ~ArrayClass();
    virtual LispChar * Send(LispArgList& aArgList);
    virtual LispChar * TypeName();
public: //array-specific
    inline LispInt Size();
    inline LispObject* GetElement(LispInt aItem); // TODO: 1-based, ...
    inline void SetElement(LispInt aItem,LispObject* aObject);

private:
    LispPtrArray iArray;
};

#include "arrayclass.inl"
#endif

