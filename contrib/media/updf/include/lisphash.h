/** \file lisphash.h
 *  hashing of strings. Each string will exist only once in the
 * hash table, and have an unique id.
 */


#ifndef __lisphash_h__
#define __lisphash_h__

#include "yacasbase.h"
#include "grower.h"
#include "lispstring.h"


const LispInt KSymTableSize = 211;
LispInt LispHash( const char *s );
LispInt LispHashCounted( const char *s, LispInt length );
LispInt LispHashStringify( const char *s );
LispInt LispHashUnStringify( const char *s );
LispInt LispHashPtr(const LispString * aString);  // hash the *address*!

/**
 * This is the symbol table, implemented as a hash table for fast
 * lookup. It is meant to store any string just once, have fast
 * searching for strings and return a reference to the string.
 * This also allows fast comparison of two strings (two strings
 * are equal iff the pointers to the strings are equal).
 */
class LispHashTable : public YacasBase
{
public:
  LispHashTable() {}
  ~LispHashTable();
  // If string not yet in table, insert. Afterwards return the string.
  LispString * LookUp(const LispChar * aString, LispBoolean aStringOwnedExternally=LispFalse);
  /// LookUp that takes ownership of the string
  LispString * LookUp(LispString * aString);
  LispString * LookUpCounted(LispChar * aString,LispInt aLength);
  LispString * LookUpStringify(LispChar * aString, LispBoolean aStringOwnedExternally=LispFalse);
  LispString * LookUpUnStringify(LispChar * aString, LispBoolean aStringOwnedExternally=LispFalse);
  void GarbageCollect();
private:
  void AppendString(LispInt bin,LispString * result);
private:
  CArrayGrower<LispStringSmartPtr, ArrOpsCustomObj<LispStringSmartPtr> > iHashTable[KSymTableSize];
};





/** VoidGrow is a helper class for LispAssociatedHash
 */
class VoidGrow : public CArrayGrower<void*, ArrOpsCustomPtr<void> >
{
};

/** LispAssociatedHash allows you to associate arbitrary
 * information with a string in the above hash table. You can
 * specify what type of information to link to the string, and
 * this class then stores that information for a string. It is
 * in a sense a way to extend the string object without modifying
 * the string class itself. This class does not own the strings it
 * points to, but instead relies on the fact that the strings
 * are maintained in a hash table (like LispHashTable above).
 */
template<class T>
class LispAssociatedHash : public YacasBase
{
public:
  /// Find the data associated to \a aString.
  /// If \a aString is not stored in the hash table, this function
  /// returns #NULL.
  inline T* LookUp(LispString * aString);

  /// Add an association to the hash table.
  /// If \a aString is already stored in the hash table, its
  /// association is changed to \a aData. Otherwise, a new
  /// association is added.
  inline void SetAssociation(const T& aData, LispString * aString);

  /// Delete an association from the hash table.
  inline void Release(LispString * aString);

protected:
  /** The destructor is made protected because we do not want the outside world to directly
   *  call this destructor. The alternative would be to make the destructor virtual.
   */
  inline ~LispAssociatedHash();

private:
  // The next array is in fact an array of arrays of type LAssoc<T>
  VoidGrow iHashTable[KSymTableSize];
};



#include "lisphash.inl"


#endif
