// -*- c++ -*-
/* This is a generated file, do not edit.  Generated from ../sigc++/macros/object_slot.h.m4 */

#ifndef SIGCXX_OBJECT_SLOT_H
#define SIGCXX_OBJECT_SLOT_H

/*
  SigC::slot() (obj)
  -----------------------
  slot() can be applied to a object method to form a Slot with a 
  profile equivelent to the method.  At the same time an instance
  of that object must be specified.  The object must be derived
  from SigC::Object.

  Sample usage:

    struct A: public SigC::Object
      {
       void foo(int,int);
      } a;

    Slot2<void,int,int> s=slot(a,&A::foo);

*/


#include <sigc++/slot.h>
#include <sigc++/scope.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif



/****************************************************************
***** Object Slot 0
****************************************************************/
template <class R,class Obj>
struct ObjectSlot0_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (Obj::*InFunc)();
   typedef RType (Obj::*Func)();
   typedef Slot0<R> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
      return ((data->obj)->*(data->func))();
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Obj>
struct ObjectSlot0_<void,Obj>
  {
   typedef void RType;
   typedef void     (Obj::*InFunc)();
   typedef RType (Obj::*Func)();
   typedef Slot0<void> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
       ((data->obj)->*(data->func))();
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };


#endif
#endif

template <class R,class O,class O2>
inline Slot0<R>
  slot(O* &obj,R (O2::*func)())
  {
    return ObjectSlot0_<R,O2>
             ::create(obj,func);
  }

template <class R,class O,class O2>
inline Slot0<R>
  slot(O* const &obj,R (O2::*func)())
  {
    return ObjectSlot0_<R,O2>
             ::create(obj,func);
  }

template <class R,class O,class O2>
inline Slot0<R>
  slot(O &obj,R (O2::*func)())
  {
    return ObjectSlot0_<R,O2>
             ::create(&obj,func);
  }



/****************************************************************
***** Object Slot 1
****************************************************************/
template <class R,class P1,class Obj>
struct ObjectSlot1_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (Obj::*InFunc)(P1);
   typedef RType (Obj::*Func)(P1);
   typedef Slot1<R,P1> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      return ((data->obj)->*(data->func))(p1);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class Obj>
struct ObjectSlot1_<void,P1,Obj>
  {
   typedef void RType;
   typedef void     (Obj::*InFunc)(P1);
   typedef RType (Obj::*Func)(P1);
   typedef Slot1<void,P1> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
       ((data->obj)->*(data->func))(p1);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };


#endif
#endif

template <class R,class P1,class O,class O2>
inline Slot1<R,P1>
  slot(O* &obj,R (O2::*func)(P1))
  {
    return ObjectSlot1_<R,P1,O2>
             ::create(obj,func);
  }

template <class R,class P1,class O,class O2>
inline Slot1<R,P1>
  slot(O* const &obj,R (O2::*func)(P1))
  {
    return ObjectSlot1_<R,P1,O2>
             ::create(obj,func);
  }

template <class R,class P1,class O,class O2>
inline Slot1<R,P1>
  slot(O &obj,R (O2::*func)(P1))
  {
    return ObjectSlot1_<R,P1,O2>
             ::create(&obj,func);
  }



/****************************************************************
***** Object Slot 2
****************************************************************/
template <class R,class P1,class P2,class Obj>
struct ObjectSlot2_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (Obj::*InFunc)(P1,P2);
   typedef RType (Obj::*Func)(P1,P2);
   typedef Slot2<R,P1,P2> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      return ((data->obj)->*(data->func))(p1,p2);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class Obj>
struct ObjectSlot2_<void,P1,P2,Obj>
  {
   typedef void RType;
   typedef void     (Obj::*InFunc)(P1,P2);
   typedef RType (Obj::*Func)(P1,P2);
   typedef Slot2<void,P1,P2> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
       ((data->obj)->*(data->func))(p1,p2);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };


#endif
#endif

template <class R,class P1,class P2,class O,class O2>
inline Slot2<R,P1,P2>
  slot(O* &obj,R (O2::*func)(P1,P2))
  {
    return ObjectSlot2_<R,P1,P2,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class O,class O2>
inline Slot2<R,P1,P2>
  slot(O* const &obj,R (O2::*func)(P1,P2))
  {
    return ObjectSlot2_<R,P1,P2,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class O,class O2>
inline Slot2<R,P1,P2>
  slot(O &obj,R (O2::*func)(P1,P2))
  {
    return ObjectSlot2_<R,P1,P2,O2>
             ::create(&obj,func);
  }



/****************************************************************
***** Object Slot 3
****************************************************************/
template <class R,class P1,class P2,class P3,class Obj>
struct ObjectSlot3_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (Obj::*InFunc)(P1,P2,P3);
   typedef RType (Obj::*Func)(P1,P2,P3);
   typedef Slot3<R,P1,P2,P3> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      return ((data->obj)->*(data->func))(p1,p2,p3);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class Obj>
struct ObjectSlot3_<void,P1,P2,P3,Obj>
  {
   typedef void RType;
   typedef void     (Obj::*InFunc)(P1,P2,P3);
   typedef RType (Obj::*Func)(P1,P2,P3);
   typedef Slot3<void,P1,P2,P3> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
       ((data->obj)->*(data->func))(p1,p2,p3);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };


#endif
#endif

template <class R,class P1,class P2,class P3,class O,class O2>
inline Slot3<R,P1,P2,P3>
  slot(O* &obj,R (O2::*func)(P1,P2,P3))
  {
    return ObjectSlot3_<R,P1,P2,P3,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class O,class O2>
inline Slot3<R,P1,P2,P3>
  slot(O* const &obj,R (O2::*func)(P1,P2,P3))
  {
    return ObjectSlot3_<R,P1,P2,P3,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class O,class O2>
inline Slot3<R,P1,P2,P3>
  slot(O &obj,R (O2::*func)(P1,P2,P3))
  {
    return ObjectSlot3_<R,P1,P2,P3,O2>
             ::create(&obj,func);
  }



/****************************************************************
***** Object Slot 4
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class Obj>
struct ObjectSlot4_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (Obj::*InFunc)(P1,P2,P3,P4);
   typedef RType (Obj::*Func)(P1,P2,P3,P4);
   typedef Slot4<R,P1,P2,P3,P4> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      return ((data->obj)->*(data->func))(p1,p2,p3,p4);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class Obj>
struct ObjectSlot4_<void,P1,P2,P3,P4,Obj>
  {
   typedef void RType;
   typedef void     (Obj::*InFunc)(P1,P2,P3,P4);
   typedef RType (Obj::*Func)(P1,P2,P3,P4);
   typedef Slot4<void,P1,P2,P3,P4> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
       ((data->obj)->*(data->func))(p1,p2,p3,p4);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };


#endif
#endif

template <class R,class P1,class P2,class P3,class P4,class O,class O2>
inline Slot4<R,P1,P2,P3,P4>
  slot(O* &obj,R (O2::*func)(P1,P2,P3,P4))
  {
    return ObjectSlot4_<R,P1,P2,P3,P4,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class O,class O2>
inline Slot4<R,P1,P2,P3,P4>
  slot(O* const &obj,R (O2::*func)(P1,P2,P3,P4))
  {
    return ObjectSlot4_<R,P1,P2,P3,P4,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class O,class O2>
inline Slot4<R,P1,P2,P3,P4>
  slot(O &obj,R (O2::*func)(P1,P2,P3,P4))
  {
    return ObjectSlot4_<R,P1,P2,P3,P4,O2>
             ::create(&obj,func);
  }



/****************************************************************
***** Object Slot 5
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class P5,class Obj>
struct ObjectSlot5_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (Obj::*InFunc)(P1,P2,P3,P4,P5);
   typedef RType (Obj::*Func)(P1,P2,P3,P4,P5);
   typedef Slot5<R,P1,P2,P3,P4,P5> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      return ((data->obj)->*(data->func))(p1,p2,p3,p4,p5);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class Obj>
struct ObjectSlot5_<void,P1,P2,P3,P4,P5,Obj>
  {
   typedef void RType;
   typedef void     (Obj::*InFunc)(P1,P2,P3,P4,P5);
   typedef RType (Obj::*Func)(P1,P2,P3,P4,P5);
   typedef Slot5<void,P1,P2,P3,P4,P5> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
       ((data->obj)->*(data->func))(p1,p2,p3,p4,p5);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };


#endif
#endif

template <class R,class P1,class P2,class P3,class P4,class P5,class O,class O2>
inline Slot5<R,P1,P2,P3,P4,P5>
  slot(O* &obj,R (O2::*func)(P1,P2,P3,P4,P5))
  {
    return ObjectSlot5_<R,P1,P2,P3,P4,P5,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class P5,class O,class O2>
inline Slot5<R,P1,P2,P3,P4,P5>
  slot(O* const &obj,R (O2::*func)(P1,P2,P3,P4,P5))
  {
    return ObjectSlot5_<R,P1,P2,P3,P4,P5,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class P5,class O,class O2>
inline Slot5<R,P1,P2,P3,P4,P5>
  slot(O &obj,R (O2::*func)(P1,P2,P3,P4,P5))
  {
    return ObjectSlot5_<R,P1,P2,P3,P4,P5,O2>
             ::create(&obj,func);
  }



/****************************************************************
***** Object Slot 6
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class Obj>
struct ObjectSlot6_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (Obj::*InFunc)(P1,P2,P3,P4,P5,P6);
   typedef RType (Obj::*Func)(P1,P2,P3,P4,P5,P6);
   typedef Slot6<R,P1,P2,P3,P4,P5,P6> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
      return ((data->obj)->*(data->func))(p1,p2,p3,p4,p5,p6);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6,class Obj>
struct ObjectSlot6_<void,P1,P2,P3,P4,P5,P6,Obj>
  {
   typedef void RType;
   typedef void     (Obj::*InFunc)(P1,P2,P3,P4,P5,P6);
   typedef RType (Obj::*Func)(P1,P2,P3,P4,P5,P6);
   typedef Slot6<void,P1,P2,P3,P4,P5,P6> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
       ((data->obj)->*(data->func))(p1,p2,p3,p4,p5,p6);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };


#endif
#endif

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class O,class O2>
inline Slot6<R,P1,P2,P3,P4,P5,P6>
  slot(O* &obj,R (O2::*func)(P1,P2,P3,P4,P5,P6))
  {
    return ObjectSlot6_<R,P1,P2,P3,P4,P5,P6,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class O,class O2>
inline Slot6<R,P1,P2,P3,P4,P5,P6>
  slot(O* const &obj,R (O2::*func)(P1,P2,P3,P4,P5,P6))
  {
    return ObjectSlot6_<R,P1,P2,P3,P4,P5,P6,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class O,class O2>
inline Slot6<R,P1,P2,P3,P4,P5,P6>
  slot(O &obj,R (O2::*func)(P1,P2,P3,P4,P5,P6))
  {
    return ObjectSlot6_<R,P1,P2,P3,P4,P5,P6,O2>
             ::create(&obj,func);
  }



/****************************************************************
***** Object Slot 7
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Obj>
struct ObjectSlot7_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (Obj::*InFunc)(P1,P2,P3,P4,P5,P6,P7);
   typedef RType (Obj::*Func)(P1,P2,P3,P4,P5,P6,P7);
   typedef Slot7<R,P1,P2,P3,P4,P5,P6,P7> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
      CallData* data=(CallData*)d;
      return ((data->obj)->*(data->func))(p1,p2,p3,p4,p5,p6,p7);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Obj>
struct ObjectSlot7_<void,P1,P2,P3,P4,P5,P6,P7,Obj>
  {
   typedef void RType;
   typedef void     (Obj::*InFunc)(P1,P2,P3,P4,P5,P6,P7);
   typedef RType (Obj::*Func)(P1,P2,P3,P4,P5,P6,P7);
   typedef Slot7<void,P1,P2,P3,P4,P5,P6,P7> SlotType;
   typedef CallDataObj3<typename SlotType::Func,Obj,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
      CallData* data=(CallData*)d;
       ((data->obj)->*(data->func))(p1,p2,p3,p4,p5,p6,p7);
     }

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      obj->register_data(tmp->receiver());
      return tmp;
     }
  };


#endif
#endif

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class O,class O2>
inline Slot7<R,P1,P2,P3,P4,P5,P6,P7>
  slot(O* &obj,R (O2::*func)(P1,P2,P3,P4,P5,P6,P7))
  {
    return ObjectSlot7_<R,P1,P2,P3,P4,P5,P6,P7,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class O,class O2>
inline Slot7<R,P1,P2,P3,P4,P5,P6,P7>
  slot(O* const &obj,R (O2::*func)(P1,P2,P3,P4,P5,P6,P7))
  {
    return ObjectSlot7_<R,P1,P2,P3,P4,P5,P6,P7,O2>
             ::create(obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class O,class O2>
inline Slot7<R,P1,P2,P3,P4,P5,P6,P7>
  slot(O &obj,R (O2::*func)(P1,P2,P3,P4,P5,P6,P7))
  {
    return ObjectSlot7_<R,P1,P2,P3,P4,P5,P6,P7,O2>
             ::create(&obj,func);
  }




#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
