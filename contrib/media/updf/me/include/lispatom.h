/** \file lispatom.h
 *  implementation of the standard lisp elements: atom and sublist.
 *
 * class LispAtom. This class implements one atom, which is a
 * reference to a string it represents, and a pointer to the next
 * lisp atom if it is in a list.
 * It also has a pointer to the annotation object.
 * The local class LispPtr implements automatic garbage collection
 * through reference counting.
 *
 * The class LispNumber inherits from LispAtom and holds a numeric atom
 * in the string representation and in the numeric representation (BigNumber).
 * The string representation is converted to BigNumber (using the current precision for floats) when a numeric
 * operation is first requested on the atom. The BigNumber representation is
 * converted to the string representation whenever the number needs to be printed i.e. LispAtom::String() method is requested.
 * The string is held in the number (to avoid repeated conversions) and also cached in the string cache (this caching will eventually be abandoned).
 * When LispNumber is constructed from BigNumber, no string representation is available.
 * Conversion from string to BigNumber is done only if no BigNumber object is present.
 */

#ifndef __lispatom_h__
#define __lispatom_h__

#include "yacasbase.h"
#include "lispobject.h"
#include "lispstring.h"
#include "numbers.h"  // RefPtr<BigNumber> needs definition of BigNumber

/// This should be used whenever constants 2, 10 mean binary and decimal.
// maybe move somewhere else?
#define BASE10 10
#define BASE2 2

// Flags used for atom types. These are not strictly necessary, but
// do speed up certain evaluations by avoiding a lot of overhead.
#define KFlagIsNumeric 0x01  // Quoted (after executing the args for
                          // a LispLambda, or for a LispSetQuoted)

class LispEnvironment;

class LispAtom : public ObjectHelper<LispAtom>
{
public:
  static LispObject* New(LispEnvironment& aEnvironment, const LispChar * aString);
  virtual ~LispAtom();
  virtual LispString * String();
  virtual LispObject* Copy() { return NEW LispAtom(*this); }
private:
  LispAtom(LispString * aString);
  LispAtom& operator=(const LispAtom& aOther)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
public:
  LispAtom(const LispAtom& other);
private:
  LispString * iString;
};

//------------------------------------------------------------------------------
// LispSublist

class LispSubList : public ObjectHelper<LispSubList>
{
public:
  static LispSubList* New(LispObject* aSubList);
  virtual ~LispSubList();
  virtual LispPtr* SubList() { return &iSubList; }
  virtual LispObject* Copy() { return NEW LispSubList(*this); }
private:
  // Constructor is private -- use New() instead
  LispSubList(LispObject* aSubList) : iSubList(aSubList) {}  // iSubList's constructor is messed up (it's a LispPtr, duh)
public:
  LispSubList(const LispSubList& other) : ASuper(other), iSubList(other.iSubList) {}
private:
  LispPtr iSubList;
};


//------------------------------------------------------------------------------
// LispGenericClass

class LispGenericClass : public ObjectHelper<LispGenericClass>
{
public:
  static LispGenericClass* New(GenericClass* aClass);
  virtual ~LispGenericClass();
  virtual GenericClass* Generic();
  virtual LispObject* Copy() { return NEW LispGenericClass(*this); }
private:
  // Constructor is private -- use New() instead
  LispGenericClass(GenericClass* aClass);
public:
  LispGenericClass(const LispGenericClass& other) : ASuper(other), iClass(other.iClass) { iClass->iReferenceCount++; }
private:
  LispGenericClass& operator=(const LispGenericClass& other)
  {
    // copy constructor not written yet, hence the assert
    LISPASSERT(0);
    return *this;
  }
private:
    GenericClass* iClass;
};

class LispHashTable;
class LispNumber : public ObjectHelper<LispNumber>
{
public:
    /// constructors:
    /// construct from another LispNumber
  LispNumber(BigNumber* aNumber, LispString * aString = NULL) : iNumber(aNumber), iString(aString) {}
  LispNumber(const LispNumber& other) : ASuper(other), iNumber(other.iNumber), iString(other.iString) {}
  /// construct from a decimal string representation (also create a number object) and use aBasePrecision decimal digits
  LispNumber(LispString * aString, LispInt aBasePrecision) : iNumber(NULL), iString(aString) { Number(aBasePrecision); }

  virtual ~LispNumber() {}
  virtual LispObject* Copy() { return NEW LispNumber(*this); }
  /// return a string representation in decimal with maximum decimal precision allowed by the inherent accuracy of the number
  virtual LispString * String();
  /// give access to the BigNumber object; if necessary, will create a BigNumber object out of the stored string, at given precision (in decimal?)
  virtual BigNumber* Number(LispInt aPrecision);
private:
  /// number object; NULL if not yet converted from string
  RefPtr<BigNumber> iNumber;
  /// string representation in decimal; NULL if not yet converted from BigNumber
  RefPtr<LispString> iString;
};


#endif


