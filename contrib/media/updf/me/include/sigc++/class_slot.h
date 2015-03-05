// -*- c++ -*-
/* This is a generated file, do not edit.  Generated from ../sigc++/macros/class_slot.h.m4 */

#ifndef SIGCXX_CLASS_SLOT_H
#define SIGCXX_CLASS_SLOT_H

/*
  SigC::slot_class() (class)
  -----------------------
  slot_class() can be applied to a class method to form a Slot with a
  profile equivelent to the method.  At the same time an instance
  of that class must be specified.  This is an unsafe interface.

  This does NOT require that the class be derived from SigC::Object.
  However, the object should be static with regards to the signal system.
  (allocated within the global scope.)  If it is not and a connected
  slot is call it will result in a seg fault.  If the object must
  be destroyed before the connected slots, all connections must
  be disconnected by hand.

  Sample usage:

    struct A
      {
       void foo(int,int);
      } a;

    Slot2<void,int,int> s=slot_class(a,&A::foo);

*/


#include <sigc++/object_slot.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif


/****************************************************************
***** Class Slot 0
****************************************************************/
template <class R,class Obj>
struct ClassSlot0_:public ObjectSlot0_<R,Obj>
  {
   typedef ObjectSlot0_<R,Obj> Base;
   typedef typename Base::InFunc InFunc;

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      return tmp;
     }
  };

template <class R,class Obj>
Slot0<R>
  slot_class(Obj &obj,R (Obj::*func)())
  {return ClassSlot0_<R,Obj>::create(&obj,func);
  }

template <class R,class Obj>
Slot0<R>
  slot_class(Obj *obj,R (Obj::*func)())
  {return ClassSlot0_<R,Obj>::create(obj,func);
  }


/****************************************************************
***** Class Slot 1
****************************************************************/
template <class R,class P1,class Obj>
struct ClassSlot1_:public ObjectSlot1_<R,P1,Obj>
  {
   typedef ObjectSlot1_<R,P1,Obj> Base;
   typedef typename Base::InFunc InFunc;

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      return tmp;
     }
  };

template <class R,class P1,class Obj>
Slot1<R,P1>
  slot_class(Obj &obj,R (Obj::*func)(P1))
  {return ClassSlot1_<R,P1,Obj>::create(&obj,func);
  }

template <class R,class P1,class Obj>
Slot1<R,P1>
  slot_class(Obj *obj,R (Obj::*func)(P1))
  {return ClassSlot1_<R,P1,Obj>::create(obj,func);
  }


/****************************************************************
***** Class Slot 2
****************************************************************/
template <class R,class P1,class P2,class Obj>
struct ClassSlot2_:public ObjectSlot2_<R,P1,P2,Obj>
  {
   typedef ObjectSlot2_<R,P1,P2,Obj> Base;
   typedef typename Base::InFunc InFunc;

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      return tmp;
     }
  };

template <class R,class P1,class P2,class Obj>
Slot2<R,P1,P2>
  slot_class(Obj &obj,R (Obj::*func)(P1,P2))
  {return ClassSlot2_<R,P1,P2,Obj>::create(&obj,func);
  }

template <class R,class P1,class P2,class Obj>
Slot2<R,P1,P2>
  slot_class(Obj *obj,R (Obj::*func)(P1,P2))
  {return ClassSlot2_<R,P1,P2,Obj>::create(obj,func);
  }


/****************************************************************
***** Class Slot 3
****************************************************************/
template <class R,class P1,class P2,class P3,class Obj>
struct ClassSlot3_:public ObjectSlot3_<R,P1,P2,P3,Obj>
  {
   typedef ObjectSlot3_<R,P1,P2,P3,Obj> Base;
   typedef typename Base::InFunc InFunc;

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      return tmp;
     }
  };

template <class R,class P1,class P2,class P3,class Obj>
Slot3<R,P1,P2,P3>
  slot_class(Obj &obj,R (Obj::*func)(P1,P2,P3))
  {return ClassSlot3_<R,P1,P2,P3,Obj>::create(&obj,func);
  }

template <class R,class P1,class P2,class P3,class Obj>
Slot3<R,P1,P2,P3>
  slot_class(Obj *obj,R (Obj::*func)(P1,P2,P3))
  {return ClassSlot3_<R,P1,P2,P3,Obj>::create(obj,func);
  }


/****************************************************************
***** Class Slot 4
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class Obj>
struct ClassSlot4_:public ObjectSlot4_<R,P1,P2,P3,P4,Obj>
  {
   typedef ObjectSlot4_<R,P1,P2,P3,P4,Obj> Base;
   typedef typename Base::InFunc InFunc;

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      return tmp;
     }
  };

template <class R,class P1,class P2,class P3,class P4,class Obj>
Slot4<R,P1,P2,P3,P4>
  slot_class(Obj &obj,R (Obj::*func)(P1,P2,P3,P4))
  {return ClassSlot4_<R,P1,P2,P3,P4,Obj>::create(&obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class Obj>
Slot4<R,P1,P2,P3,P4>
  slot_class(Obj *obj,R (Obj::*func)(P1,P2,P3,P4))
  {return ClassSlot4_<R,P1,P2,P3,P4,Obj>::create(obj,func);
  }


/****************************************************************
***** Class Slot 5
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class P5,class Obj>
struct ClassSlot5_:public ObjectSlot5_<R,P1,P2,P3,P4,P5,Obj>
  {
   typedef ObjectSlot5_<R,P1,P2,P3,P4,P5,Obj> Base;
   typedef typename Base::InFunc InFunc;

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      return tmp;
     }
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class Obj>
Slot5<R,P1,P2,P3,P4,P5>
  slot_class(Obj &obj,R (Obj::*func)(P1,P2,P3,P4,P5))
  {return ClassSlot5_<R,P1,P2,P3,P4,P5,Obj>::create(&obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class P5,class Obj>
Slot5<R,P1,P2,P3,P4,P5>
  slot_class(Obj *obj,R (Obj::*func)(P1,P2,P3,P4,P5))
  {return ClassSlot5_<R,P1,P2,P3,P4,P5,Obj>::create(obj,func);
  }


/****************************************************************
***** Class Slot 6
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class Obj>
struct ClassSlot6_:public ObjectSlot6_<R,P1,P2,P3,P4,P5,P6,Obj>
  {
   typedef ObjectSlot6_<R,P1,P2,P3,P4,P5,P6,Obj> Base;
   typedef typename Base::InFunc InFunc;

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      return tmp;
     }
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class Obj>
Slot6<R,P1,P2,P3,P4,P5,P6>
  slot_class(Obj &obj,R (Obj::*func)(P1,P2,P3,P4,P5,P6))
  {return ClassSlot6_<R,P1,P2,P3,P4,P5,P6,Obj>::create(&obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class Obj>
Slot6<R,P1,P2,P3,P4,P5,P6>
  slot_class(Obj *obj,R (Obj::*func)(P1,P2,P3,P4,P5,P6))
  {return ClassSlot6_<R,P1,P2,P3,P4,P5,P6,Obj>::create(obj,func);
  }


/****************************************************************
***** Class Slot 7
****************************************************************/
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Obj>
struct ClassSlot7_:public ObjectSlot7_<R,P1,P2,P3,P4,P5,P6,P7,Obj>
  {
   typedef ObjectSlot7_<R,P1,P2,P3,P4,P5,P6,P7,Obj> Base;
   typedef typename Base::InFunc InFunc;

   static SlotData* create(Obj* obj,InFunc func)
     {
      if (!obj) return 0;
      SlotData* tmp=manage(new SlotData());
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=obj;
      data.func=(Func)func;
      return tmp;
     }
  };

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Obj>
Slot7<R,P1,P2,P3,P4,P5,P6,P7>
  slot_class(Obj &obj,R (Obj::*func)(P1,P2,P3,P4,P5,P6,P7))
  {return ClassSlot7_<R,P1,P2,P3,P4,P5,P6,P7,Obj>::create(&obj,func);
  }

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Obj>
Slot7<R,P1,P2,P3,P4,P5,P6,P7>
  slot_class(Obj *obj,R (Obj::*func)(P1,P2,P3,P4,P5,P6,P7))
  {return ClassSlot7_<R,P1,P2,P3,P4,P5,P6,P7,Obj>::create(obj,func);
  }



#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
