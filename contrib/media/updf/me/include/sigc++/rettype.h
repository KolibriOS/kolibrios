// -*- c++ -*-
/* This is a generated file, do not edit.  Generated from ../sigc++/macros/rettype.h.m4 */

#ifndef SIGCXX_RETTYPE_H
#define SIGCXX_RETTYPE_H

/*
  SigC::rettype
  -------------
  rettype() alters a Slot by changing the return type.

  Only allowed conversions or conversions to void can properly
  be implemented.  The type must always be specified as a
  template parameter. 

  Simple Sample usage:

    int f(int);

    Slot1<void,int>   s1=rettype<void>(slot(&f)); 
    Slot1<float,int>  s2=rettype<float>(slot(&f)); 

*/

#include <sigc++/adaptor.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif











/****************************************************************
***** Adaptor Rettype Slot 0
****************************************************************/
template <class R1,
   class R2>
struct AdaptorRettypeSlot0_
  : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot0<R1> SlotType;
   typedef Slot0<R2> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return RType(((typename InSlotType::Callback&)(node->data_))());
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class R2>
struct AdaptorRettypeSlot0_
<void,
   R2>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot0<void> SlotType;
   typedef Slot0<R2> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))();
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif


template <class R1,class R2>
Slot0<R1>
  rettype(const Slot0<R2> &s)
  {return AdaptorRettypeSlot0_<R1,R2>::create(s.obj());
  }

/****************************************************************
***** Adaptor Rettype Slot 1
****************************************************************/
template <class R1,
   class P1,
   class R2>
struct AdaptorRettypeSlot1_
  : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot1<R1,P1> SlotType;
   typedef Slot1<R2,P1> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return RType(((typename InSlotType::Callback&)(node->data_))(p1));
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class P1,
   class R2>
struct AdaptorRettypeSlot1_
<void,
   P1,
   R2>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot1<void,P1> SlotType;
   typedef Slot1<R2,P1> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1);
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif


template <class R1,class R2,class P1>
Slot1<R1,P1>
  rettype(const Slot1<R2,P1> &s)
  {return AdaptorRettypeSlot1_<R1,
           P1,R2>::create(s.obj());
  }

/****************************************************************
***** Adaptor Rettype Slot 2
****************************************************************/
template <class R1,
   class P1,class P2,
   class R2>
struct AdaptorRettypeSlot2_
  : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot2<R1,P1,P2> SlotType;
   typedef Slot2<R2,P1,P2> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return RType(((typename InSlotType::Callback&)(node->data_))(p1,p2));
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class P1,class P2,
   class R2>
struct AdaptorRettypeSlot2_
<void,
   P1,P2,
   R2>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot2<void,P1,P2> SlotType;
   typedef Slot2<R2,P1,P2> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2);
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif


template <class R1,class R2,class P1,class P2>
Slot2<R1,P1,P2>
  rettype(const Slot2<R2,P1,P2> &s)
  {return AdaptorRettypeSlot2_<R1,
           P1,P2,R2>::create(s.obj());
  }

/****************************************************************
***** Adaptor Rettype Slot 3
****************************************************************/
template <class R1,
   class P1,class P2,class P3,
   class R2>
struct AdaptorRettypeSlot3_
  : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot3<R1,P1,P2,P3> SlotType;
   typedef Slot3<R2,P1,P2,P3> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return RType(((typename InSlotType::Callback&)(node->data_))(p1,p2,p3));
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class P1,class P2,class P3,
   class R2>
struct AdaptorRettypeSlot3_
<void,
   P1,P2,P3,
   R2>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot3<void,P1,P2,P3> SlotType;
   typedef Slot3<R2,P1,P2,P3> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3);
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif


template <class R1,class R2,class P1,class P2,class P3>
Slot3<R1,P1,P2,P3>
  rettype(const Slot3<R2,P1,P2,P3> &s)
  {return AdaptorRettypeSlot3_<R1,
           P1,P2,P3,R2>::create(s.obj());
  }

/****************************************************************
***** Adaptor Rettype Slot 4
****************************************************************/
template <class R1,
   class P1,class P2,class P3,class P4,
   class R2>
struct AdaptorRettypeSlot4_
  : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot4<R1,P1,P2,P3,P4> SlotType;
   typedef Slot4<R2,P1,P2,P3,P4> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return RType(((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4));
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class P1,class P2,class P3,class P4,
   class R2>
struct AdaptorRettypeSlot4_
<void,
   P1,P2,P3,P4,
   R2>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot4<void,P1,P2,P3,P4> SlotType;
   typedef Slot4<R2,P1,P2,P3,P4> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4);
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif


template <class R1,class R2,class P1,class P2,class P3,class P4>
Slot4<R1,P1,P2,P3,P4>
  rettype(const Slot4<R2,P1,P2,P3,P4> &s)
  {return AdaptorRettypeSlot4_<R1,
           P1,P2,P3,P4,R2>::create(s.obj());
  }

/****************************************************************
***** Adaptor Rettype Slot 5
****************************************************************/
template <class R1,
   class P1,class P2,class P3,class P4,class P5,
   class R2>
struct AdaptorRettypeSlot5_
  : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot5<R1,P1,P2,P3,P4,P5> SlotType;
   typedef Slot5<R2,P1,P2,P3,P4,P5> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return RType(((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5));
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class P1,class P2,class P3,class P4,class P5,
   class R2>
struct AdaptorRettypeSlot5_
<void,
   P1,P2,P3,P4,P5,
   R2>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot5<void,P1,P2,P3,P4,P5> SlotType;
   typedef Slot5<R2,P1,P2,P3,P4,P5> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5);
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif


template <class R1,class R2,class P1,class P2,class P3,class P4,class P5>
Slot5<R1,P1,P2,P3,P4,P5>
  rettype(const Slot5<R2,P1,P2,P3,P4,P5> &s)
  {return AdaptorRettypeSlot5_<R1,
           P1,P2,P3,P4,P5,R2>::create(s.obj());
  }

/****************************************************************
***** Adaptor Rettype Slot 6
****************************************************************/
template <class R1,
   class P1,class P2,class P3,class P4,class P5,class P6,
   class R2>
struct AdaptorRettypeSlot6_
  : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot6<R1,P1,P2,P3,P4,P5,P6> SlotType;
   typedef Slot6<R2,P1,P2,P3,P4,P5,P6> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return RType(((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6));
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class P1,class P2,class P3,class P4,class P5,class P6,
   class R2>
struct AdaptorRettypeSlot6_
<void,
   P1,P2,P3,P4,P5,P6,
   R2>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot6<void,P1,P2,P3,P4,P5,P6> SlotType;
   typedef Slot6<R2,P1,P2,P3,P4,P5,P6> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6);
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif


template <class R1,class R2,class P1,class P2,class P3,class P4,class P5,class P6>
Slot6<R1,P1,P2,P3,P4,P5,P6>
  rettype(const Slot6<R2,P1,P2,P3,P4,P5,P6> &s)
  {return AdaptorRettypeSlot6_<R1,
           P1,P2,P3,P4,P5,P6,R2>::create(s.obj());
  }

/****************************************************************
***** Adaptor Rettype Slot 7
****************************************************************/
template <class R1,
   class P1,class P2,class P3,class P4,class P5,class P6,class P7,
   class R2>
struct AdaptorRettypeSlot7_
  : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot7<R1,P1,P2,P3,P4,P5,P6,P7> SlotType;
   typedef Slot7<R2,P1,P2,P3,P4,P5,P6,P7> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return RType(((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6,p7));
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class P1,class P2,class P3,class P4,class P5,class P6,class P7,
   class R2>
struct AdaptorRettypeSlot7_
<void,
   P1,P2,P3,P4,P5,P6,P7,
   R2>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot7<void,P1,P2,P3,P4,P5,P6,P7> SlotType;
   typedef Slot7<R2,P1,P2,P3,P4,P5,P6,P7> InSlotType;
   typedef AdaptorNode Node;
   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6,p7);
     }
   static SlotData* create(SlotData *s)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif


template <class R1,class R2,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
Slot7<R1,P1,P2,P3,P4,P5,P6,P7>
  rettype(const Slot7<R2,P1,P2,P3,P4,P5,P6,P7> &s)
  {return AdaptorRettypeSlot7_<R1,
           P1,P2,P3,P4,P5,P6,P7,R2>::create(s.obj());
  }


#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
