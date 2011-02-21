// -*- c++ -*-
/* This is a generated file, do not edit.  Generated from ../sigc++/macros/slot.h.m4 */


#ifndef SIGCXX_SLOT_H
#define SIGCXX_SLOT_H

/*

This file just gives the basic definition of Slots. 

Callback# is the 4 byte data necessary for representing all
callback types.

CallData is a specific interpretation of the Callback data.

Slot_ is a pimple on SlotData containing an Object for
holding its referencees, a Dependency that removes the slot
when its caller or receiver die, and a Callback.

Slot is a handle to a Slot_.

*/

#include <sigc++config.h>
#include <sigc++/type.h>
#include <sigc++/object.h>
#include <sigc++/handle.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

// Base node for a polymorphic list of "extra" data needed
// by various slots. 
struct LIBSIGC_API SlotNode
  {
   void *next_;
   SlotNode();
   virtual ~SlotNode()=0;
  };

struct LIBSIGC_API SlotIterator_
  {
	typedef SlotNode NodeType;
   typedef SlotIterator_ Iterator;
   NodeType *node_;

   NodeType* node()             {return node_;}
   const NodeType* node() const {return node_;}

   NodeType& operator*()
     {return *node_;
     }
   const NodeType& operator*() const
     {return *node_;
     }

   bool operator==(const Iterator& i) const
     {return node_==i.node_;
     }
   bool operator!=(const Iterator& i) const
     {return node_!=i.node_;
     }

   Iterator& operator++()
     {
      if (node_)
        node_=(NodeType*)node_->next_;
      return *this;
     }

   Iterator operator++(int)
     {Iterator tmp=*this;
      ++*this;
      return tmp;
     }

   Iterator& operator= (const Iterator& i)
     {
      node_=i.node_;
      return *this;
     }

   SlotIterator_():node_(0) {}
   SlotIterator_(NodeType *node):node_(node) {}
  };

// This is a list for storing internal data for slots
struct LIBSIGC_API SlotList_
  {
   typedef SlotNode NodeType;
   typedef SlotIterator_ Iterator;
   NodeType* head_;

   Iterator begin()             {return ((NodeType*)head_);}
   Iterator end()               {return Iterator();}
   const Iterator begin() const {return ((NodeType*)head_);}
   const Iterator end()   const {return Iterator();}

   // this is best used at the begining of list.
   Iterator insert_direct(Iterator pos,NodeType *n);

   void clear();
   bool empty() const {return head_==0;}

   SlotList_():head_(0)
     {}
   ~SlotList_()
     {clear();}

   private:
     SlotList_(const SlotList_&);
  };


struct SlotData;

// SlotDependent is an internal of SlotData used to unreference the
// Slot when either the sender or receiver have gone away
struct LIBSIGC_API SlotDependent:public ScopeNode
  {
   struct LIBSIGC_API Dep: public ScopeNode
     {
      SlotData *parent;
      virtual void erase();
      Dep() {}
      virtual ~Dep();
     } dep;

   ScopeNode* receiver() {return &dep;}
   ScopeNode* sender()   {return this;}
   SlotData*  parent()   {return dep.parent;}

   bool connected()
     {return (next_!=this);}

   virtual void erase();

   void set_parent(SlotData *s)
     {dep.parent=s;}

   SlotDependent(SlotData &s)
     {dep.parent=&s;}

   SlotDependent()
     {}

   virtual ~SlotDependent();
  };

// common data to all callbacks.  
struct Callback_
  {
   // callback function
   void* (*func_)(void*);

   struct O;
   struct C1
     {
      void* (*f1)(void*);
     };
   struct C2
     {
      O* o;
      void (O::*v)(void);
     };

   // Object pointer or function pointer
   union {C1 a1; C2 a2;};
  };

// All slots have the same base 
struct LIBSIGC_API SlotData:public ObjectScoped
  {
   typedef SlotList_ List;

   SlotDependent dep_;

   ScopeNode* receiver() {return dep_.receiver();}
   ScopeNode* sender()   {return dep_.sender();}

   // Called from signals to tell slot object it is connected
   // invalidates list and sets weak reference
   void connect();

   List list_;
   Callback_ data_;

   Callback_& callback() {return data_;}

   SlotData()
     {dep_.set_parent(this);}
   virtual ~SlotData();
  };


typedef Scopes::Extend SlotExtend;
#ifdef LIBSIGC_MSC
#pragma warning(disable: 4231)
LIBSIGC_TMPL template class LIBSIGC_API Handle<SlotData,SlotExtend>;
#endif
class LIBSIGC_API Connection:protected Handle<SlotData,SlotExtend>
  {
   typedef Handle<SlotData,SlotExtend> Base;
   public:
     // hides virtual method
     void disconnect() {if (obj()) obj()->invalid();}
     bool connected() {return Base::connected ();}

     Connection():Base() {}
     Connection(SlotData *s):Base(s) {}
     Connection(const Connection& s):Base(s) { }
     ~Connection() {}
  };

// possible casts of Callback
template <class C,class F>
struct CallDataFunc
  {
   C callback;
   F func;
  };

template <class C,class O>
struct CallDataObj2
  {
   C callback;
   O *obj;
  };

template <class C,class O,class F>
struct CallDataObj3
  { 
   C   callback;
   O*  obj;
   F   func;
  };

// from Abstract_Slots we build abstract slots
// with various lengths of arguments
//   A slot is not concrete til it has a call



/****************************************************************
***** Abstract Slot 0
****************************************************************/

template <class R>
struct Callback0:public Callback_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef Trait<R>::type RType;
#endif
   typedef RType (*Func)(void*);
   inline RType call()
     {return ((Func)(func_))((void*)this);}
   inline RType operator()()
     {return ((Func)(func_))((void*)this);}
  };

template <class R>
class Slot0
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback0<R> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot0() {}
     Slot0(SlotData *s):Base(s)    {}
     Slot0(const Slot0& s):Base(s.obj()) {}

     inline RType call() 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call();
         return RType();
        }
     inline RType operator()() 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call();
         return RType();
        }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC

template <>
struct Callback0<void>:public Callback_
  {
   typedef void RType;
   typedef RType (*Func)(void*);
   inline RType call()
     {((Func)(func_))((void*)this);}
   inline RType operator()()
     {((Func)(func_))((void*)this);}
  };

template <>
class Slot0<void>
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback0<void> Callback;
     typedef  Callback::RType RType;
     typedef RType (*Func)(void*);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot0() {}
     Slot0(SlotData *s):Base(s)    {}
     Slot0(const Slot0& s):Base(s.obj()) {}

     inline RType call() 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call();
        }
     inline RType operator()() 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call();
        }
  };

#endif
#endif


/****************************************************************
***** Abstract Slot 1
****************************************************************/

template <class R,class P1>
struct Callback1:public Callback_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef Trait<R>::type RType;
#endif
   typedef RType (*Func)(void*,P1);
   inline RType call(typename Trait<P1>::ref p1)
     {return ((Func)(func_))((void*)this,p1);}
   inline RType operator()(typename Trait<P1>::ref p1)
     {return ((Func)(func_))((void*)this,p1);}
  };

template <class R,class P1>
class Slot1
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback1<R,P1> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot1() {}
     Slot1(SlotData *s):Base(s)    {}
     Slot1(const Slot1& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1);
         return RType();
        }
     inline RType operator()(typename Trait<P1>::ref p1) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1);
         return RType();
        }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC

template <class P1>
struct Callback1<void,P1>:public Callback_
  {
   typedef void RType;
   typedef RType (*Func)(void*,P1);
   inline RType call(typename Trait<P1>::ref p1)
     {((Func)(func_))((void*)this,p1);}
   inline RType operator()(typename Trait<P1>::ref p1)
     {((Func)(func_))((void*)this,p1);}
  };

template <class P1>
class Slot1<void,P1>
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback1<void,P1> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot1() {}
     Slot1(SlotData *s):Base(s)    {}
     Slot1(const Slot1& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1);
        }
     inline RType operator()(typename Trait<P1>::ref p1) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1);
        }
  };

#endif
#endif


/****************************************************************
***** Abstract Slot 2
****************************************************************/

template <class R,class P1,class P2>
struct Callback2:public Callback_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef Trait<R>::type RType;
#endif
   typedef RType (*Func)(void*,P1,P2);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
     {return ((Func)(func_))((void*)this,p1,p2);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
     {return ((Func)(func_))((void*)this,p1,p2);}
  };

template <class R,class P1,class P2>
class Slot2
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback2<R,P1,P2> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot2() {}
     Slot2(SlotData *s):Base(s)    {}
     Slot2(const Slot2& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2);
         return RType();
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2);
         return RType();
        }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC

template <class P1,class P2>
struct Callback2<void,P1,P2>:public Callback_
  {
   typedef void RType;
   typedef RType (*Func)(void*,P1,P2);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
     {((Func)(func_))((void*)this,p1,p2);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
     {((Func)(func_))((void*)this,p1,p2);}
  };

template <class P1,class P2>
class Slot2<void,P1,P2>
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback2<void,P1,P2> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot2() {}
     Slot2(SlotData *s):Base(s)    {}
     Slot2(const Slot2& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2);
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2);
        }
  };

#endif
#endif


/****************************************************************
***** Abstract Slot 3
****************************************************************/

template <class R,class P1,class P2,class P3>
struct Callback3:public Callback_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef Trait<R>::type RType;
#endif
   typedef RType (*Func)(void*,P1,P2,P3);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
     {return ((Func)(func_))((void*)this,p1,p2,p3);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
     {return ((Func)(func_))((void*)this,p1,p2,p3);}
  };

template <class R,class P1,class P2,class P3>
class Slot3
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback3<R,P1,P2,P3> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot3() {}
     Slot3(SlotData *s):Base(s)    {}
     Slot3(const Slot3& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3);
         return RType();
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3);
         return RType();
        }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC

template <class P1,class P2,class P3>
struct Callback3<void,P1,P2,P3>:public Callback_
  {
   typedef void RType;
   typedef RType (*Func)(void*,P1,P2,P3);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
     {((Func)(func_))((void*)this,p1,p2,p3);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
     {((Func)(func_))((void*)this,p1,p2,p3);}
  };

template <class P1,class P2,class P3>
class Slot3<void,P1,P2,P3>
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback3<void,P1,P2,P3> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot3() {}
     Slot3(SlotData *s):Base(s)    {}
     Slot3(const Slot3& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3);
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3);
        }
  };

#endif
#endif


/****************************************************************
***** Abstract Slot 4
****************************************************************/

template <class R,class P1,class P2,class P3,class P4>
struct Callback4:public Callback_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef Trait<R>::type RType;
#endif
   typedef RType (*Func)(void*,P1,P2,P3,P4);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
     {return ((Func)(func_))((void*)this,p1,p2,p3,p4);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
     {return ((Func)(func_))((void*)this,p1,p2,p3,p4);}
  };

template <class R,class P1,class P2,class P3,class P4>
class Slot4
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback4<R,P1,P2,P3,P4> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3,P4);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot4() {}
     Slot4(SlotData *s):Base(s)    {}
     Slot4(const Slot4& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3,p4);
         return RType();
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3,p4);
         return RType();
        }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC

template <class P1,class P2,class P3,class P4>
struct Callback4<void,P1,P2,P3,P4>:public Callback_
  {
   typedef void RType;
   typedef RType (*Func)(void*,P1,P2,P3,P4);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
     {((Func)(func_))((void*)this,p1,p2,p3,p4);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
     {((Func)(func_))((void*)this,p1,p2,p3,p4);}
  };

template <class P1,class P2,class P3,class P4>
class Slot4<void,P1,P2,P3,P4>
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback4<void,P1,P2,P3,P4> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3,P4);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot4() {}
     Slot4(SlotData *s):Base(s)    {}
     Slot4(const Slot4& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3,p4);
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3,p4);
        }
  };

#endif
#endif


/****************************************************************
***** Abstract Slot 5
****************************************************************/

template <class R,class P1,class P2,class P3,class P4,class P5>
struct Callback5:public Callback_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef Trait<R>::type RType;
#endif
   typedef RType (*Func)(void*,P1,P2,P3,P4,P5);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
     {return ((Func)(func_))((void*)this,p1,p2,p3,p4,p5);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
     {return ((Func)(func_))((void*)this,p1,p2,p3,p4,p5);}
  };

template <class R,class P1,class P2,class P3,class P4,class P5>
class Slot5
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback5<R,P1,P2,P3,P4,P5> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3,P4,P5);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot5() {}
     Slot5(SlotData *s):Base(s)    {}
     Slot5(const Slot5& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5);
         return RType();
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5);
         return RType();
        }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC

template <class P1,class P2,class P3,class P4,class P5>
struct Callback5<void,P1,P2,P3,P4,P5>:public Callback_
  {
   typedef void RType;
   typedef RType (*Func)(void*,P1,P2,P3,P4,P5);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
     {((Func)(func_))((void*)this,p1,p2,p3,p4,p5);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
     {((Func)(func_))((void*)this,p1,p2,p3,p4,p5);}
  };

template <class P1,class P2,class P3,class P4,class P5>
class Slot5<void,P1,P2,P3,P4,P5>
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback5<void,P1,P2,P3,P4,P5> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3,P4,P5);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot5() {}
     Slot5(SlotData *s):Base(s)    {}
     Slot5(const Slot5& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5);
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5);
        }
  };

#endif
#endif


/****************************************************************
***** Abstract Slot 6
****************************************************************/

template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
struct Callback6:public Callback_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef Trait<R>::type RType;
#endif
   typedef RType (*Func)(void*,P1,P2,P3,P4,P5,P6);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
     {return ((Func)(func_))((void*)this,p1,p2,p3,p4,p5,p6);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
     {return ((Func)(func_))((void*)this,p1,p2,p3,p4,p5,p6);}
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
class Slot6
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback6<R,P1,P2,P3,P4,P5,P6> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3,P4,P5,P6);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot6() {}
     Slot6(SlotData *s):Base(s)    {}
     Slot6(const Slot6& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5,p6);
         return RType();
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5,p6);
         return RType();
        }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC

template <class P1,class P2,class P3,class P4,class P5,class P6>
struct Callback6<void,P1,P2,P3,P4,P5,P6>:public Callback_
  {
   typedef void RType;
   typedef RType (*Func)(void*,P1,P2,P3,P4,P5,P6);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
     {((Func)(func_))((void*)this,p1,p2,p3,p4,p5,p6);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
     {((Func)(func_))((void*)this,p1,p2,p3,p4,p5,p6);}
  };

template <class P1,class P2,class P3,class P4,class P5,class P6>
class Slot6<void,P1,P2,P3,P4,P5,P6>
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback6<void,P1,P2,P3,P4,P5,P6> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3,P4,P5,P6);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot6() {}
     Slot6(SlotData *s):Base(s)    {}
     Slot6(const Slot6& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5,p6);
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5,p6);
        }
  };

#endif
#endif


/****************************************************************
***** Abstract Slot 7
****************************************************************/

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
struct Callback7:public Callback_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef Trait<R>::type RType;
#endif
   typedef RType (*Func)(void*,P1,P2,P3,P4,P5,P6,P7);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
     {return ((Func)(func_))((void*)this,p1,p2,p3,p4,p5,p6,p7);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
     {return ((Func)(func_))((void*)this,p1,p2,p3,p4,p5,p6,p7);}
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
class Slot7
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback7<R,P1,P2,P3,P4,P5,P6,P7> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3,P4,P5,P6,P7);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot7() {}
     Slot7(SlotData *s):Base(s)    {}
     Slot7(const Slot7& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5,p6,p7);
         return RType();
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7) 
        {
         if (connected()) 
           return ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5,p6,p7);
         return RType();
        }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC

template <class P1,class P2,class P3,class P4,class P5,class P6,class P7>
struct Callback7<void,P1,P2,P3,P4,P5,P6,P7>:public Callback_
  {
   typedef void RType;
   typedef RType (*Func)(void*,P1,P2,P3,P4,P5,P6,P7);
   inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
     {((Func)(func_))((void*)this,p1,p2,p3,p4,p5,p6,p7);}
   inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
     {((Func)(func_))((void*)this,p1,p2,p3,p4,p5,p6,p7);}
  };

template <class P1,class P2,class P3,class P4,class P5,class P6,class P7>
class Slot7<void,P1,P2,P3,P4,P5,P6,P7>
   :public Handle<SlotData,SlotExtend>
  {
   public:
     typedef Handle<SlotData,SlotExtend> Base;
     typedef Callback7<void,P1,P2,P3,P4,P5,P6,P7> Callback;
     typedef typename Callback::RType RType;
     typedef RType (*Func)(void*,P1,P2,P3,P4,P5,P6,P7);

     SlotData* data()     const {return (SlotData*)(scope_.object());}

     Slot7() {}
     Slot7(SlotData *s):Base(s)    {}
     Slot7(const Slot7& s):Base(s.obj()) {}

     inline RType call(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5,p6,p7);
        }
     inline RType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7) 
        {
         if (connected()) 

           ((Callback&)(data()->callback())).call(p1,p2,p3,p4,p5,p6,p7);
        }
  };

#endif
#endif



#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif // SIGCXX_SLOT_H
