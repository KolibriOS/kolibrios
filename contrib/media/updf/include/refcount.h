#ifndef __refcount_h__
#define __refcount_h__

#include "lispassert.h"
#include "yacasbase.h"

//------------------------------------------------------------------------------
// RefPtr - Smart pointer for (intrusive) reference counting.
// Simply, an object's reference count is the number of RefPtrs refering to it.
// The RefPtr will delete the referenced object when the count reaches zero.

/*TODO: this might be improved a little by having RefPtr wrap the object being
  pointed to so the user of RefPtr does not need to add ReferenceCount explicitly.
  One can use RefPtr on any arbitrary object from that moment on.
 */

typedef ReferenceType ReferenceCount;

template<class T>
class RefPtr : public YacasBase  // derived, so we can 'NEW LispPtr[nnn]'
{
public:
  // Default constructor (not explicit, so it auto-initializes)
  inline RefPtr() : iPtr(NULL) {}
  // Construct from pointer to T
  explicit RefPtr(T *ptr) : iPtr(ptr) { if (ptr) { ptr->iReferenceCount++; } }
  // Copy constructor
  RefPtr(const RefPtr &refPtr) : iPtr(refPtr.ptr()) { if (iPtr) { iPtr->iReferenceCount++; } }
  // Destructor
  ~RefPtr()
  {
    if (iPtr)
    {
      iPtr->iReferenceCount--;
      if (iPtr->iReferenceCount == 0)
      {
        delete iPtr;
      }
    }
  }
  // Assignment from pointer
  RefPtr &operator=(T *ptr)
  {
    if (ptr)
    {
      ptr->iReferenceCount++;
    }
    if (iPtr)
    {
      iPtr->iReferenceCount--;
      if (iPtr->iReferenceCount == 0)
      {
        delete iPtr;
      }
    }
    iPtr = ptr;
    return *this;
  }
  // Assignment from another
  RefPtr &operator=(const RefPtr &refPtr) { return this->operator=(refPtr.ptr()); }

  operator T*()    const { return  iPtr; }  // implicit conversion to pointer to T
  T &operator*()   const { return *iPtr; }  // so (*refPtr) is a reference to T
  T *operator->()  const { return  iPtr; }  // so (refPtr->member) accesses T's member
  T *ptr()         const { return  iPtr; }  // so (refPtr.ptr()) returns the pointer to T (boost calls this method 'get')
  bool operator!() const { return !iPtr; }  // is null pointer

private:
   T *iPtr;
};


#endif

