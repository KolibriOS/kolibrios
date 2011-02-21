// -*- c++ -*-
/* This is a generated file, do not edit.  Generated from ../sigc++/macros/convert.h.m4 */

#ifndef SIGCXX_CONVERT_H
#define SIGCXX_CONVERT_H

/*
  SigC::convert
  -------------
  convert() alters a Slot by assigning a conversion function 
  which can completely alter the parameter types of a slot. 

  Only convert functions for changing with same number of
  arguments is compiled by default.  See examples/custom_convert.h.m4 
  for details on how to build non standard ones.

  Sample usage:
    int my_string_to_char(Callback1<int,const char*> *d,const string &s)
    int f(const char*);
    string s=hello;


    Slot1<int,const string &>  s2=convert(slot(f),my_string_to_char);
    s2(s);  

*/
#include <sigc++/adaptor.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif



/****************************************************************
***** Adaptor Convert Slot 1
****************************************************************/
template <class R1,class P1,
          class R2,class Q1>
struct AdaptorConvertSlot1_1: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot1<R1,P1> SlotType;
   typedef Slot1<R2,Q1> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1);
   typedef R1    (*InFunc)(Callback*,P1);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      return (data->func)(data->obj,p1);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,
          class R2,class Q1>
struct AdaptorConvertSlot1_1
         <void,P1,
          R2,Q1>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot1<void,P1> SlotType;
   typedef Slot1<R2,Q1> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1);
   typedef void    (*InFunc)(Callback*,P1);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
       (data->func)(data->obj,p1);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };


#endif
#endif

template <class R1,class P1,
          class R2,class Q1>
Slot1<R1,P1>
  convert(const Slot1<R2,Q1> &s,
          R1 (*func)(Callback1<R2,Q1>*,P1))
  {return AdaptorConvertSlot1_1<R1,P1,
           R2,Q1>::create(s.obj(),func);
  }


/****************************************************************
***** Adaptor Convert Slot 2
****************************************************************/
template <class R1,class P1,class P2,
          class R2,class Q1,class Q2>
struct AdaptorConvertSlot2_2: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot2<R1,P1,P2> SlotType;
   typedef Slot2<R2,Q1,Q2> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2);
   typedef R1    (*InFunc)(Callback*,P1,P2);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      return (data->func)(data->obj,p1,p2);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,
          class R2,class Q1,class Q2>
struct AdaptorConvertSlot2_2
         <void,P1,P2,
          R2,Q1,Q2>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot2<void,P1,P2> SlotType;
   typedef Slot2<R2,Q1,Q2> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2);
   typedef void    (*InFunc)(Callback*,P1,P2);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
       (data->func)(data->obj,p1,p2);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };


#endif
#endif

template <class R1,class P1,class P2,
          class R2,class Q1,class Q2>
Slot2<R1,P1,P2>
  convert(const Slot2<R2,Q1,Q2> &s,
          R1 (*func)(Callback2<R2,Q1,Q2>*,P1,P2))
  {return AdaptorConvertSlot2_2<R1,P1,P2,
           R2,Q1,Q2>::create(s.obj(),func);
  }


/****************************************************************
***** Adaptor Convert Slot 3
****************************************************************/
template <class R1,class P1,class P2,class P3,
          class R2,class Q1,class Q2,class Q3>
struct AdaptorConvertSlot3_3: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot3<R1,P1,P2,P3> SlotType;
   typedef Slot3<R2,Q1,Q2,Q3> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3);
   typedef R1    (*InFunc)(Callback*,P1,P2,P3);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      return (data->func)(data->obj,p1,p2,p3);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,
          class R2,class Q1,class Q2,class Q3>
struct AdaptorConvertSlot3_3
         <void,P1,P2,P3,
          R2,Q1,Q2,Q3>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot3<void,P1,P2,P3> SlotType;
   typedef Slot3<R2,Q1,Q2,Q3> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3);
   typedef void    (*InFunc)(Callback*,P1,P2,P3);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
       (data->func)(data->obj,p1,p2,p3);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };


#endif
#endif

template <class R1,class P1,class P2,class P3,
          class R2,class Q1,class Q2,class Q3>
Slot3<R1,P1,P2,P3>
  convert(const Slot3<R2,Q1,Q2,Q3> &s,
          R1 (*func)(Callback3<R2,Q1,Q2,Q3>*,P1,P2,P3))
  {return AdaptorConvertSlot3_3<R1,P1,P2,P3,
           R2,Q1,Q2,Q3>::create(s.obj(),func);
  }


/****************************************************************
***** Adaptor Convert Slot 4
****************************************************************/
template <class R1,class P1,class P2,class P3,class P4,
          class R2,class Q1,class Q2,class Q3,class Q4>
struct AdaptorConvertSlot4_4: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot4<R1,P1,P2,P3,P4> SlotType;
   typedef Slot4<R2,Q1,Q2,Q3,Q4> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3,P4);
   typedef R1    (*InFunc)(Callback*,P1,P2,P3,P4);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      return (data->func)(data->obj,p1,p2,p3,p4);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,
          class R2,class Q1,class Q2,class Q3,class Q4>
struct AdaptorConvertSlot4_4
         <void,P1,P2,P3,P4,
          R2,Q1,Q2,Q3,Q4>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot4<void,P1,P2,P3,P4> SlotType;
   typedef Slot4<R2,Q1,Q2,Q3,Q4> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3,P4);
   typedef void    (*InFunc)(Callback*,P1,P2,P3,P4);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
       (data->func)(data->obj,p1,p2,p3,p4);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };


#endif
#endif

template <class R1,class P1,class P2,class P3,class P4,
          class R2,class Q1,class Q2,class Q3,class Q4>
Slot4<R1,P1,P2,P3,P4>
  convert(const Slot4<R2,Q1,Q2,Q3,Q4> &s,
          R1 (*func)(Callback4<R2,Q1,Q2,Q3,Q4>*,P1,P2,P3,P4))
  {return AdaptorConvertSlot4_4<R1,P1,P2,P3,P4,
           R2,Q1,Q2,Q3,Q4>::create(s.obj(),func);
  }


/****************************************************************
***** Adaptor Convert Slot 5
****************************************************************/
template <class R1,class P1,class P2,class P3,class P4,class P5,
          class R2,class Q1,class Q2,class Q3,class Q4,class Q5>
struct AdaptorConvertSlot5_5: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot5<R1,P1,P2,P3,P4,P5> SlotType;
   typedef Slot5<R2,Q1,Q2,Q3,Q4,Q5> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3,P4,P5);
   typedef R1    (*InFunc)(Callback*,P1,P2,P3,P4,P5);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      return (data->func)(data->obj,p1,p2,p3,p4,p5);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,
          class R2,class Q1,class Q2,class Q3,class Q4,class Q5>
struct AdaptorConvertSlot5_5
         <void,P1,P2,P3,P4,P5,
          R2,Q1,Q2,Q3,Q4,Q5>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot5<void,P1,P2,P3,P4,P5> SlotType;
   typedef Slot5<R2,Q1,Q2,Q3,Q4,Q5> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3,P4,P5);
   typedef void    (*InFunc)(Callback*,P1,P2,P3,P4,P5);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
       (data->func)(data->obj,p1,p2,p3,p4,p5);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };


#endif
#endif

template <class R1,class P1,class P2,class P3,class P4,class P5,
          class R2,class Q1,class Q2,class Q3,class Q4,class Q5>
Slot5<R1,P1,P2,P3,P4,P5>
  convert(const Slot5<R2,Q1,Q2,Q3,Q4,Q5> &s,
          R1 (*func)(Callback5<R2,Q1,Q2,Q3,Q4,Q5>*,P1,P2,P3,P4,P5))
  {return AdaptorConvertSlot5_5<R1,P1,P2,P3,P4,P5,
           R2,Q1,Q2,Q3,Q4,Q5>::create(s.obj(),func);
  }


/****************************************************************
***** Adaptor Convert Slot 6
****************************************************************/
template <class R1,class P1,class P2,class P3,class P4,class P5,class P6,
          class R2,class Q1,class Q2,class Q3,class Q4,class Q5,class Q6>
struct AdaptorConvertSlot6_6: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot6<R1,P1,P2,P3,P4,P5,P6> SlotType;
   typedef Slot6<R2,Q1,Q2,Q3,Q4,Q5,Q6> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3,P4,P5,P6);
   typedef R1    (*InFunc)(Callback*,P1,P2,P3,P4,P5,P6);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
      return (data->func)(data->obj,p1,p2,p3,p4,p5,p6);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6,
          class R2,class Q1,class Q2,class Q3,class Q4,class Q5,class Q6>
struct AdaptorConvertSlot6_6
         <void,P1,P2,P3,P4,P5,P6,
          R2,Q1,Q2,Q3,Q4,Q5,Q6>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot6<void,P1,P2,P3,P4,P5,P6> SlotType;
   typedef Slot6<R2,Q1,Q2,Q3,Q4,Q5,Q6> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3,P4,P5,P6);
   typedef void    (*InFunc)(Callback*,P1,P2,P3,P4,P5,P6);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
       (data->func)(data->obj,p1,p2,p3,p4,p5,p6);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };


#endif
#endif

template <class R1,class P1,class P2,class P3,class P4,class P5,class P6,
          class R2,class Q1,class Q2,class Q3,class Q4,class Q5,class Q6>
Slot6<R1,P1,P2,P3,P4,P5,P6>
  convert(const Slot6<R2,Q1,Q2,Q3,Q4,Q5,Q6> &s,
          R1 (*func)(Callback6<R2,Q1,Q2,Q3,Q4,Q5,Q6>*,P1,P2,P3,P4,P5,P6))
  {return AdaptorConvertSlot6_6<R1,P1,P2,P3,P4,P5,P6,
           R2,Q1,Q2,Q3,Q4,Q5,Q6>::create(s.obj(),func);
  }


/****************************************************************
***** Adaptor Convert Slot 7
****************************************************************/
template <class R1,class P1,class P2,class P3,class P4,class P5,class P6,class P7,
          class R2,class Q1,class Q2,class Q3,class Q4,class Q5,class Q6,class Q7>
struct AdaptorConvertSlot7_7: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R1 RType;
#else
   typedef typename Trait<R1>::type RType;
#endif
   typedef Slot7<R1,P1,P2,P3,P4,P5,P6,P7> SlotType;
   typedef Slot7<R2,Q1,Q2,Q3,Q4,Q5,Q6,Q7> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3,P4,P5,P6,P7);
   typedef R1    (*InFunc)(Callback*,P1,P2,P3,P4,P5,P6,P7);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
      CallData* data=(CallData*)d;
      return (data->func)(data->obj,p1,p2,p3,p4,p5,p6,p7);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };



#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,
          class R2,class Q1,class Q2,class Q3,class Q4,class Q5,class Q6,class Q7>
struct AdaptorConvertSlot7_7
         <void,P1,P2,P3,P4,P5,P6,P7,
          R2,Q1,Q2,Q3,Q4,Q5,Q6,Q7>
  : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot7<void,P1,P2,P3,P4,P5,P6,P7> SlotType;
   typedef Slot7<R2,Q1,Q2,Q3,Q4,Q5,Q6,Q7> InSlotType;
   typedef typename InSlotType::Callback Callback;
   typedef RType (*Func)  (Callback*,P1,P2,P3,P4,P5,P6,P7);
   typedef void    (*InFunc)(Callback*,P1,P2,P3,P4,P5,P6,P7);
   typedef CallDataObj3<typename SlotType::Func,Callback,Func> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
      CallData* data=(CallData*)d;
       (data->func)(data->obj,p1,p2,p3,p4,p5,p6,p7);
     }

   static SlotData* create(SlotData *s,InFunc func)
     {
      SlotData* tmp=(SlotData*)s;
      AdaptorNode *node=new AdaptorNode();
      copy_callback(tmp,node);
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.func=(Func)func;
      data.obj=(Callback*)&(node->data_);
      return tmp;
     }
  };


#endif
#endif

template <class R1,class P1,class P2,class P3,class P4,class P5,class P6,class P7,
          class R2,class Q1,class Q2,class Q3,class Q4,class Q5,class Q6,class Q7>
Slot7<R1,P1,P2,P3,P4,P5,P6,P7>
  convert(const Slot7<R2,Q1,Q2,Q3,Q4,Q5,Q6,Q7> &s,
          R1 (*func)(Callback7<R2,Q1,Q2,Q3,Q4,Q5,Q6,Q7>*,P1,P2,P3,P4,P5,P6,P7))
  {return AdaptorConvertSlot7_7<R1,P1,P2,P3,P4,P5,P6,P7,
           R2,Q1,Q2,Q3,Q4,Q5,Q6,Q7>::create(s.obj(),func);
  }



#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
