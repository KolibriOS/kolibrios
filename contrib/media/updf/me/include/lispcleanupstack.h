
/** \file lispcleanupstack.h
 * Implementation of a cleanup stack for exception handling on platforms
 *  that don't clean up the stack automatically after an exception
 *  occurs. The macro's SAFEPUSH and SAFEPOP as defined in
 *  plat/<plat>/lisptype.h define which cleanup handler
 *  to use, so it can be configured differently for different platforms.
 *
 */

#ifndef __lispcleanupstack_h__
#define __lispcleanupstack_h__

#include "yacasbase.h"
#include "grower.h"

/** Base class that specifies one pure abstract method Delete.
 *  Only classes derived from this one can be pushed onto
 *  a cleanup stack. This in order to assure the cleanup code
 *  knows where to find the destructor.
 *
 *  Typical candidates for cleanup include places in the code that
 *  have temporal global side effects that need to be finalized on,
 *  Like opening a file for reading, reading and then closing. If
 *  reading prematurely finishes through an exception, the file
 *  should be closed.
 */
class LispBase : public YacasBase
{
public:
    virtual void Delete()=0;
    virtual ~LispBase(){};
};

/** Clean up stack that doesn't actually delete objects itself.
 *  Use this clean up stack if the compiler generates
 *  cleanup code itself (particularly for newer c++ compilers).
 *  Alternatively SAFEPUSH and SAFEPOP can then be defined to do nothing.
 */
class LispCleanup : public YacasBase
{
public:
   inline LispCleanup() : iObjects() {}
    virtual ~LispCleanup();
    /// Push an object onto the cleanup stack for guarding
    virtual void Push(LispBase& aObject);
    /// Pop an object from the cleanup stack (the system is finished using it)
    virtual void Pop();
    /// Exception occurred: delete all objects on the stack, back to front.
    virtual void Delete();
    /** For testing purposes, verify that all places that pushed an object
     *  popped it too.
     */
    void CheckStackEmpty();

protected:
    CArrayGrower<LispBase*, ArrOpsCustomPtr<LispBase> > iObjects;
};

/** Clean up stack that deletes objects itself when needed.
 *  Use this clean up stack if the compiler doesn't generate
 *  cleanup code itself (particularly for older c++ compilers).
 */
class DeletingLispCleanup : public LispCleanup
{
public:
    virtual ~DeletingLispCleanup();
    virtual void Push(LispBase& aObject);
    virtual void Pop();
    virtual void Delete();
};


#endif


