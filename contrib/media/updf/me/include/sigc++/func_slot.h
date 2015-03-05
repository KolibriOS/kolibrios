// -*- c++ -*-
/* This is a generated file, do not edit.  Generated from ../sigc++/macros/func_slot.h.m4 */

#ifndef SIGCXX_FUNC_SLOT_H
#define SIGCXX_FUNC_SLOT_H
#include <sigc++/slot.h>

/*
  SigC::slot() (function)
  -----------------------
  slot() can be applied to a function to form a Slot with a 
  profile equivelent to the function.  To avoid warns be
  sure to pass the address of the function.

  Sample usage:

    void foo(int,int);

    Slot2<void,int,int> s=slot(&foo);

*/

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

// From which we build specific Slots and a set of
// functions for creating a slot of this type



/****************************************************************
***** Function Slot 0
****************************************************************/
template <class R>
struct FuncSlot0_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (*InFunc)();
   typedef RType (*Func)();
   typedef Slot0<R> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data)
     {
      return (((CallData*)data)->func)();
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <>
struct FuncSlot0_<void>
  {
   typedef void RType;
   typedef void     (*InFunc)();
   typedef RType (*Func)();
   typedef Slot0<void> SlotType;

   typedef CallDataFunc<SlotType::Func,Func> CallData;

   static RType callback(void* data)
     {
       (((CallData*)data)->func)();
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };

#endif
#endif

template <class R>
inline Slot0<R>
  slot(R (*func)())
  {
   return FuncSlot0_<R>::create(func);
  }


/****************************************************************
***** Function Slot 1
****************************************************************/
template <class R,class P1>
struct FuncSlot1_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (*InFunc)(P1);
   typedef RType (*Func)(P1);
   typedef Slot1<R,P1> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1)
     {
      return (((CallData*)data)->func)(p1);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1>
struct FuncSlot1_<void,P1>
  {
   typedef void RType;
   typedef void     (*InFunc)(P1);
   typedef RType (*Func)(P1);
   typedef Slot1<void,P1> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1)
     {
       (((CallData*)data)->func)(p1);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };

#endif
#endif

template <class R,class P1>
inline Slot1<R,P1>
  slot(R (*func)(P1))
  {
   return FuncSlot1_<R,P1>::create(func);
  }


/****************************************************************
***** Function Slot 2
****************************************************************/
template <class R,class P1,class P2>
struct FuncSlot2_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (*InFunc)(P1,P2);
   typedef RType (*Func)(P1,P2);
   typedef Slot2<R,P1,P2> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2)
     {
      return (((CallData*)data)->func)(p1,p2);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2>
struct FuncSlot2_<void,P1,P2>
  {
   typedef void RType;
   typedef void     (*InFunc)(P1,P2);
   typedef RType (*Func)(P1,P2);
   typedef Slot2<void,P1,P2> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2)
     {
       (((CallData*)data)->func)(p1,p2);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };

#endif
#endif

template <class R,class P1,class P2>
inline Slot2<R,P1,P2>
  slot(R (*func)(P1,P2))
  {
   return FuncSlot2_<R,P1,P2>::create(func);
  }


/****************************************************************
***** Function Slot 3
****************************************************************/
template <class R,class P1,class P2,class P3>
struct FuncSlot3_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (*InFunc)(P1,P2,P3);
   typedef RType (*Func)(P1,P2,P3);
   typedef Slot3<R,P1,P2,P3> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3)
     {
      return (((CallData*)data)->func)(p1,p2,p3);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3>
struct FuncSlot3_<void,P1,P2,P3>
  {
   typedef void RType;
   typedef void     (*InFunc)(P1,P2,P3);
   typedef RType (*Func)(P1,P2,P3);
   typedef Slot3<void,P1,P2,P3> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3)
     {
       (((CallData*)data)->func)(p1,p2,p3);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };

#endif
#endif

template <class R,class P1,class P2,class P3>
inline Slot3<R,P1,P2,P3>
  slot(R (*func)(P1,P2,P3))
  {
   return FuncSlot3_<R,P1,P2,P3>::create(func);
  }


/****************************************************************
***** Function Slot 4
****************************************************************/
template <class R,class P1,class P2,class P3,class P4>
struct FuncSlot4_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (*InFunc)(P1,P2,P3,P4);
   typedef RType (*Func)(P1,P2,P3,P4);
   typedef Slot4<R,P1,P2,P3,P4> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      return (((CallData*)data)->func)(p1,p2,p3,p4);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4>
struct FuncSlot4_<void,P1,P2,P3,P4>
  {
   typedef void RType;
   typedef void     (*InFunc)(P1,P2,P3,P4);
   typedef RType (*Func)(P1,P2,P3,P4);
   typedef Slot4<void,P1,P2,P3,P4> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3,P4 p4)
     {
       (((CallData*)data)->func)(p1,p2,p3,p4);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };

#endif
#endif

template <class R,class P1,class P2,class P3,class P4>
inline Slot4<R,P1,P2,P3,P4>
  slot(R (*func)(P1,P2,P3,P4))
  {
   return FuncSlot4_<R,P1,P2,P3,P4>::create(func);
  }


/****************************************************************
***** Function Slot 5
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class P5>
struct FuncSlot5_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (*InFunc)(P1,P2,P3,P4,P5);
   typedef RType (*Func)(P1,P2,P3,P4,P5);
   typedef Slot5<R,P1,P2,P3,P4,P5> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      return (((CallData*)data)->func)(p1,p2,p3,p4,p5);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5>
struct FuncSlot5_<void,P1,P2,P3,P4,P5>
  {
   typedef void RType;
   typedef void     (*InFunc)(P1,P2,P3,P4,P5);
   typedef RType (*Func)(P1,P2,P3,P4,P5);
   typedef Slot5<void,P1,P2,P3,P4,P5> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
       (((CallData*)data)->func)(p1,p2,p3,p4,p5);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };

#endif
#endif

template <class R,class P1,class P2,class P3,class P4,class P5>
inline Slot5<R,P1,P2,P3,P4,P5>
  slot(R (*func)(P1,P2,P3,P4,P5))
  {
   return FuncSlot5_<R,P1,P2,P3,P4,P5>::create(func);
  }


/****************************************************************
***** Function Slot 6
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
struct FuncSlot6_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (*InFunc)(P1,P2,P3,P4,P5,P6);
   typedef RType (*Func)(P1,P2,P3,P4,P5,P6);
   typedef Slot6<R,P1,P2,P3,P4,P5,P6> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      return (((CallData*)data)->func)(p1,p2,p3,p4,p5,p6);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6>
struct FuncSlot6_<void,P1,P2,P3,P4,P5,P6>
  {
   typedef void RType;
   typedef void     (*InFunc)(P1,P2,P3,P4,P5,P6);
   typedef RType (*Func)(P1,P2,P3,P4,P5,P6);
   typedef Slot6<void,P1,P2,P3,P4,P5,P6> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
       (((CallData*)data)->func)(p1,p2,p3,p4,p5,p6);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };

#endif
#endif

template <class R,class P1,class P2,class P3,class P4,class P5,class P6>
inline Slot6<R,P1,P2,P3,P4,P5,P6>
  slot(R (*func)(P1,P2,P3,P4,P5,P6))
  {
   return FuncSlot6_<R,P1,P2,P3,P4,P5,P6>::create(func);
  }


/****************************************************************
***** Function Slot 7
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
struct FuncSlot7_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef R     (*InFunc)(P1,P2,P3,P4,P5,P6,P7);
   typedef RType (*Func)(P1,P2,P3,P4,P5,P6,P7);
   typedef Slot7<R,P1,P2,P3,P4,P5,P6,P7> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
      return (((CallData*)data)->func)(p1,p2,p3,p4,p5,p6,p7);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7>
struct FuncSlot7_<void,P1,P2,P3,P4,P5,P6,P7>
  {
   typedef void RType;
   typedef void     (*InFunc)(P1,P2,P3,P4,P5,P6,P7);
   typedef RType (*Func)(P1,P2,P3,P4,P5,P6,P7);
   typedef Slot7<void,P1,P2,P3,P4,P5,P6,P7> SlotType;

   typedef CallDataFunc<typename SlotType::Func,Func> CallData;

   static RType callback(void* data,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
       (((CallData*)data)->func)(p1,p2,p3,p4,p5,p6,p7);
     }

   static SlotData* create(InFunc func)
     {
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      return tmp;
     }
  };

#endif
#endif

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
inline Slot7<R,P1,P2,P3,P4,P5,P6,P7>
  slot(R (*func)(P1,P2,P3,P4,P5,P6,P7))
  {
   return FuncSlot7_<R,P1,P2,P3,P4,P5,P6,P7>::create(func);
  }



#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif // SIGCXX_FUNC_SLOT_H
