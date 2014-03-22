/** \file genericstructs.h defines the class that handles structs and classes for plugins.
 */
#ifndef __genericstruct_h__
#define __genericstruct_h__

#include "yacasbase.h"
#include "genericobject.h"
#include "lispassert.h"

/** \class GenericStruct This class maintains a pointer to some arbitrary
 *  object (which can be any thing). The plugin is responsible for supplying
 *  functions for manipulating such structs/classes/arrays. The Yacas core
 *  then needs to know nothing about the internals of such a struct.
 *
 *  The struct is represented by a void pointer to the struct, a pointer
 *  to a function that can clean up the struct (used when automatically
 *  deleting the object), and a pointer to a text string representing the
 *  type of the object (useful for testing if the type passed as an argument
 *  to a function is correct).
 */
class GenericStruct : public GenericClass
{
public:
  GenericStruct(LispChar * aTypeName, void* aData, void (*aDestructor)(void*));
  virtual ~GenericStruct();
  virtual LispChar * Send(LispArgList& aArgList);
  virtual LispChar * TypeName();
  inline void* Data() {return iData;}
private:
  GenericStruct(const GenericStruct& aOther) : iData(NULL),iTypeName(NULL),iDestructor(NULL)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
  }
  GenericStruct& operator=(const GenericStruct& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
public:
  void* iData;
  LispChar * iTypeName;
  void (*iDestructor)(void* data);
};


#endif

