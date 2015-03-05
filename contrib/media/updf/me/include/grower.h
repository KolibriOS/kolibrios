/** \file grower.h Template class that implements dynamic growing arrays.
 *  Yacas implements its own growing array class to ensure correct behaviour
 *  on all platforms (stl was still in development and not supported on all
 *  platforms when development started).
 *
 * grower started 1998 ayal Pinkus
 *
 */
 
#ifndef _GROWER_H_
#define _GROWER_H_

#include "yacasbase.h"
#include "lispassert.h"



// Default placement versions of operator new and delete, placed here because I do not want to include <new>
inline void* operator new(size_t, void* __p) throw() { return __p; }
inline void* operator new[](size_t, void* __p) throw() { return __p; }
inline void  operator delete  (void*, void*) throw() { }
inline void  operator delete[](void*, void*) throw() { }


template <class T>
inline void constructAt(void * to_obj, const T * from_obj)
{
  new (to_obj) T(*from_obj);
}

class ArrOps
{
public:
  virtual bool isPOD() const { return false; }
  virtual void construct(void * buffer) const = 0;
  virtual void construct(void * to_obj, const void * from_obj) const = 0;
  virtual void destruct(void * obj) const = 0;
  virtual int granularity() const { return 8; }
  static void resize(char** pArray, const ArrOps& opers, int& iSize, int& iCapacity, int aSize, int aItemSize, int ext);
    static void remove(char** pArray, const ArrOps& opers, int& iSize, int aIndex, int aCount, int aItemSize);
  static void reserve(char** pArray, int& iCapacity, int aSize, int aItemSize);
  //static void* insert(char** pArray, const ArrOps& opers, int& iSize, void* aData, int aIndex, int aCount, int aSize, int aExtSize);
};

template <class T>
class ArrOpsCustomObj : public ArrOps
{
public:
  ArrOpsCustomObj() {}
  inline void construct(void * buffer) const
  { new (buffer) T; }
  inline void construct(void * to_obj, const void * from_obj) const
  { constructAt(to_obj, static_cast<const T*>(from_obj)); }
  inline void destruct(void * obj) const
  { /*obj;*/ static_cast<T*>(obj)->~T(); }
  //inline int size() const { return sizeof(T); }
};

template <class T>
class ArrOpsPOD : public ArrOps
{
public:
  ArrOpsPOD() {}
  inline bool isPOD() const { return true; }
  inline void construct(void * buffer) const {}
  inline void construct(void * to_obj, const void * from_obj) const
  { *static_cast<T*>(to_obj) = *static_cast<const T*>(from_obj); }
  inline void destruct(void * obj) const {}
  //inline int size() const { return sizeof(T); }
};

template <class T>
class ArrOpsCustomPtr : public ArrOps
{
  typedef T* TY;
public:
  ArrOpsCustomPtr() {}
  inline void construct(void * buffer) const
  { *static_cast<TY*>(buffer) = 0; }
  inline void construct(void * to_obj, const void * from_obj) const
  { *static_cast<TY*>(to_obj) = *static_cast<const TY*>(from_obj); }
  inline void destruct(void * obj) const {}
  //inline int size() const { return sizeof(TY); }
  inline int granularity() const { return 8; }
};

template <class T>
class ArrOpsDeletingObj {};

template <class T>
class ArrOpsDeletingPtr : public ArrOpsCustomPtr<T>
{
  typedef T* TY;
public:
  inline void destruct(void * obj) const
  { delete *static_cast<const TY*>(obj); }
};

/** template class useful for implementing a dynamic growing array
 *  for any arbitrary type of object. If using the array to maintain
 *  objects, please use pointers to the objects, and use the
 * CDeletingArrayGrower to automatically delete the objects at destruction.
 */
template <class T, class TOps >
class CArrayGrower : public YacasBase
{
public:
  /** ElementType can be used outside this class as the type of the
   * object in the array. This is useful in templated functions that
   * work on a CArrayGrower without being part of CArrayGrower
   */
  typedef LispInt SizeType;
  typedef T ElementType;

  CArrayGrower()
    : iArray(0)
    , iSize(0)
    , iCapacity(0)
    , iArrayOwnedExternally(LispFalse)
    {
    }
  virtual ~CArrayGrower()
  {
    Clear();
  }
  inline void Clear()  // oddly, only *one* use outside destructor!?
  {
    if (iSize)
    {
      const TOps opers;
      if(!opers.isPOD())
      {
        while (iSize)
        opers.destruct(iArray + --iSize);
      }
    }
    if (!iArrayOwnedExternally)
    {
      PlatFree(iArray);
    }
    iArray = 0;
    iCapacity = iSize = 0;
    iArrayOwnedExternally = LispFalse;
  }
  inline SizeType Size() const { return iSize; }

  inline CArrayGrower(const CArrayGrower<T,TOps>& aOther)
    : iArray(0)
    , iSize(0)
    , iCapacity(0)
    , iArrayOwnedExternally(LispFalse)
  {
    // Make sure we're not accidentally copying a huge array. We want this system to stay efficient...
    LISPASSERT(aOther.iSize == 0);
    LISPASSERT(aOther.iArrayOwnedExternally == LispFalse);
  }

  inline const CArrayGrower<T,TOps>& operator=(const CArrayGrower<T,TOps>& aOther)
  {
    // Make sure we're not accidentally copying a huge array. We want this system to stay efficient...
    LISPASSERT(iArray == 0);
    LISPASSERT(iSize == 0);
    LISPASSERT(iCapacity == 0);
    LISPASSERT(iArrayOwnedExternally == LispFalse);

    LISPASSERT(aOther.iSize == 0);
    LISPASSERT(aOther.iArrayOwnedExternally == LispFalse);
    return *this;
  }

private:

  void moreCapacity(SizeType aSize, int g)  // almost independent of T
  {
    LISPASSERT(!iArrayOwnedExternally);
    LISPASSERT(iCapacity >= 0);
    // Compute a new iCapacity >= aSize, with iCapacity % g == 0.
    // We assume g is a power of 2.  (fwiw, in two's-complement, ~(g-1) == -g.
    iCapacity = (aSize + g) & ~(g-1);
    if (!iArray)
    {
      iArray = (ElementType*)PlatAlloc(iCapacity*sizeof(ElementType));
    }
    else
    {
      // we assume 'memcpy' suffices for moving the existing items.
      iArray = (ElementType*)PlatReAlloc(iArray,iCapacity*sizeof(ElementType));
    }
  }
public:
  inline void ResizeTo(SizeType aSize)
  {
    LISPASSERT(!iArrayOwnedExternally);
    TOps opers;
    if (aSize > iCapacity)
    {
      moreCapacity(aSize, opers.granularity());
    }
    if (!opers.isPOD())
    {
      if (iSize < aSize)
      {
        for (int ii = iSize; ii < aSize; ii++)
          opers.construct(iArray + ii);
      }
      else
      {
        for (int ii = aSize; ii < iSize; ii++)
          opers.destruct(iArray + ii);
      }
    }
    iSize = aSize;
  }
  void Delete(SizeType aIndex, SizeType aCount=1)
  {
    LISPASSERT(aIndex>=0 && aIndex<iSize);
    ArrOps::remove((char**)&iArray, TOps(), iSize, aIndex, aCount, sizeof(ElementType));
  }
  inline LispBoolean ArrayOwnedExternally()
  {
    return iArrayOwnedExternally;
  }
public:
  /// Access to an element in the array
  inline ElementType& operator[](const SizeType aIndex) const
  {
    return iArray[aIndex];
  }

    /// Append an element to an array
  template <class Type>
    inline void Append(const Type& aVal)
    {
      if (iSize >= iCapacity) moreCapacity(iSize+1, TOps().granularity());
      new ((void *)(iArray+iSize)) ElementType(aVal);
      ++iSize;
    }

  /// Insert object aObj at aIndex, aCount times.
  inline void Insert(SizeType aIndex, const ElementType& aObj, SizeType aCount=1)
  {
    const SizeType oldItems = iSize;
    LISPASSERT(aIndex <= oldItems && aCount >= 0);
    ResizeTo(iSize+aCount);
    ElementType * pOld = iArray+oldItems;
    ElementType * pEnd = iArray+iSize;
    int i = iSize - aIndex;  // = (oldItems - aIndex) + aCount
    for ( ; i > aCount; i--)
      *--pEnd = *--pOld;
    while (i-- > 0)
      *--pEnd = aObj;
  }

  /** Set the array to an external array. This means the array will
    * not be freed at destruction time
    */
  inline void SetExternalArray(ElementType* aArray, SizeType aSize)
  {
    LISPASSERT(!iArray || iArrayOwnedExternally == LispTrue);
    iArray = aArray;
    iSize = aSize;
    iArrayOwnedExternally = LispTrue;
    iCapacity = -10000;  // Setting iCapacity should not strictly be necessary, setting it to hugely negative number will hopefully force a fail.
  }

  /// Copy the array to another array
  inline void CopyToExternalArray(ElementType * aArray)
  {
    PlatMemCopy(aArray,iArray,iSize*sizeof(ElementType));
  }

protected:
  inline ElementType * elements() const { return iArray; }
private:
  ElementType * iArray;
  SizeType iSize;
  SizeType iCapacity;
  LispBoolean iArrayOwnedExternally;
};

/** \class CDeletingArrayGrower calls delete on each element in the
 * array at destruction time. This is useful if the array is a list
 * of pointers to objects.
 */

template <class T, class TOps >
class CDeletingArrayGrower : public CArrayGrower<T, TOps >
{
public:
  CDeletingArrayGrower() {}
};

#endif


