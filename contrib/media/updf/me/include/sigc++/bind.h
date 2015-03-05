// -*- c++ -*-
/* This is a generated file, do not edit.  Generated from ../sigc++/macros/bind.h.m4 */

#ifndef SIGCXX_BIND_H
#define SIGCXX_BIND_H

/*
  SigC::bind
  -------------
  bind() alters a Slot by fixing arguments to certain values.

  Argument fixing starts from the last argument.  The slot is
  destroyed in the process and a new one is created, so references
  holding onto the slot will no longer be valid.

  Up to two arguments can be bound at a time with the default
  header.

  Simple Sample usage:

    void f(int,int);
    Slot2<void,int,int> s1=slot(f);

    Slot1<void,int>  s2=bind(s1,1);  // s1 is invalid
    s2(2);  // call f with arguments 2,1 

  Multibinding usage:

    void f(int,int);
    Slot2<void,int,int> s1=slot(f);
 
    Slot0<void>  s2=bind(s1,1,2);  // s1 is invalid
    s2();  // call f with arguments 1,2 

  Type specified usage:
    
    struct A {};
    struct B :public A {};
    B* b;
    Slot0<void, A*> s1;

    Slot0<void> s2=bind(s1, b);  // B* converted to A*
     
*/

#include <sigc++/adaptor.h>
#include <sigc++/scope.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif












/****************************************************************
***** Adaptor Bind Slot 0 arguments, 1 hidden arguments
****************************************************************/
template <class R,
   class C1>
struct AdaptorBindSlot0_1: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot0<R> SlotType;
   typedef Slot1<R,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class C1>
struct AdaptorBindSlot0_1
   <void,
    C1> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot0<void> SlotType;
   typedef Slot1<void,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,
    class R,
    class C1>
inline
Slot0<R>
  bind(const Slot1<R,C1> &s,
       A1 a1)
  {return AdaptorBindSlot0_1<R,
           C1>::create(s.data(),a1);
  }


/****************************************************************
***** Adaptor Bind Slot 1 arguments, 1 hidden arguments
****************************************************************/
template <class R,
   class P1,
   class C1>
struct AdaptorBindSlot1_1: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot1<R,P1> SlotType;
   typedef Slot2<R,P1,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,
   class C1>
struct AdaptorBindSlot1_1
   <void,P1,
    C1> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot1<void,P1> SlotType;
   typedef Slot2<void,P1,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,
    class R,
    class P1,
    class C1>
inline
Slot1<R,P1>
  bind(const Slot2<R,P1,C1> &s,
       A1 a1)
  {return AdaptorBindSlot1_1<R,
           P1,
           C1>::create(s.data(),a1);
  }


/****************************************************************
***** Adaptor Bind Slot 2 arguments, 1 hidden arguments
****************************************************************/
template <class R,
   class P1,class P2,
   class C1>
struct AdaptorBindSlot2_1: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot2<R,P1,P2> SlotType;
   typedef Slot3<R,P1,P2,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,p2,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,
   class C1>
struct AdaptorBindSlot2_1
   <void,P1,P2,
    C1> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot2<void,P1,P2> SlotType;
   typedef Slot3<void,P1,P2,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,p2,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,
    class R,
    class P1,class P2,
    class C1>
inline
Slot2<R,P1,P2>
  bind(const Slot3<R,P1,P2,C1> &s,
       A1 a1)
  {return AdaptorBindSlot2_1<R,
           P1,P2,
           C1>::create(s.data(),a1);
  }


/****************************************************************
***** Adaptor Bind Slot 3 arguments, 1 hidden arguments
****************************************************************/
template <class R,
   class P1,class P2,class P3,
   class C1>
struct AdaptorBindSlot3_1: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot3<R,P1,P2,P3> SlotType;
   typedef Slot4<R,P1,P2,P3,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,
   class C1>
struct AdaptorBindSlot3_1
   <void,P1,P2,P3,
    C1> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot3<void,P1,P2,P3> SlotType;
   typedef Slot4<void,P1,P2,P3,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,
    class R,
    class P1,class P2,class P3,
    class C1>
inline
Slot3<R,P1,P2,P3>
  bind(const Slot4<R,P1,P2,P3,C1> &s,
       A1 a1)
  {return AdaptorBindSlot3_1<R,
           P1,P2,P3,
           C1>::create(s.data(),a1);
  }


/****************************************************************
***** Adaptor Bind Slot 4 arguments, 1 hidden arguments
****************************************************************/
template <class R,
   class P1,class P2,class P3,class P4,
   class C1>
struct AdaptorBindSlot4_1: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot4<R,P1,P2,P3,P4> SlotType;
   typedef Slot5<R,P1,P2,P3,P4,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,
   class C1>
struct AdaptorBindSlot4_1
   <void,P1,P2,P3,P4,
    C1> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot4<void,P1,P2,P3,P4> SlotType;
   typedef Slot5<void,P1,P2,P3,P4,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,
    class R,
    class P1,class P2,class P3,class P4,
    class C1>
inline
Slot4<R,P1,P2,P3,P4>
  bind(const Slot5<R,P1,P2,P3,P4,C1> &s,
       A1 a1)
  {return AdaptorBindSlot4_1<R,
           P1,P2,P3,P4,
           C1>::create(s.data(),a1);
  }


/****************************************************************
***** Adaptor Bind Slot 5 arguments, 1 hidden arguments
****************************************************************/
template <class R,
   class P1,class P2,class P3,class P4,class P5,
   class C1>
struct AdaptorBindSlot5_1: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot5<R,P1,P2,P3,P4,P5> SlotType;
   typedef Slot6<R,P1,P2,P3,P4,P5,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,
   class C1>
struct AdaptorBindSlot5_1
   <void,P1,P2,P3,P4,P5,
    C1> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot5<void,P1,P2,P3,P4,P5> SlotType;
   typedef Slot6<void,P1,P2,P3,P4,P5,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,
    class R,
    class P1,class P2,class P3,class P4,class P5,
    class C1>
inline
Slot5<R,P1,P2,P3,P4,P5>
  bind(const Slot6<R,P1,P2,P3,P4,P5,C1> &s,
       A1 a1)
  {return AdaptorBindSlot5_1<R,
           P1,P2,P3,P4,P5,
           C1>::create(s.data(),a1);
  }


/****************************************************************
***** Adaptor Bind Slot 6 arguments, 1 hidden arguments
****************************************************************/
template <class R,
   class P1,class P2,class P3,class P4,class P5,class P6,
   class C1>
struct AdaptorBindSlot6_1: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot6<R,P1,P2,P3,P4,P5,P6> SlotType;
   typedef Slot7<R,P1,P2,P3,P4,P5,P6,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6,
   class C1>
struct AdaptorBindSlot6_1
   <void,P1,P2,P3,P4,P5,P6,
    C1> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot6<void,P1,P2,P3,P4,P5,P6> SlotType;
   typedef Slot7<void,P1,P2,P3,P4,P5,P6,C1> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;

      Node(C1 c1)
	: c1_(c1)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6,
                                     node->c1_);
     }
   static SlotData* create(SlotData *s,C1 c1)
     {
      Node *node=new Node(c1);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,
    class R,
    class P1,class P2,class P3,class P4,class P5,class P6,
    class C1>
inline
Slot6<R,P1,P2,P3,P4,P5,P6>
  bind(const Slot7<R,P1,P2,P3,P4,P5,P6,C1> &s,
       A1 a1)
  {return AdaptorBindSlot6_1<R,
           P1,P2,P3,P4,P5,P6,
           C1>::create(s.data(),a1);
  }



/****************************************************************
***** Adaptor Bind Slot 0 arguments, 2 hidden arguments
****************************************************************/
template <class R,
   class C1,class C2>
struct AdaptorBindSlot0_2: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot0<R> SlotType;
   typedef Slot2<R,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <
   class C1,class C2>
struct AdaptorBindSlot0_2
   <void,
    C1,C2> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot0<void> SlotType;
   typedef Slot2<void,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,class A2,
    class R,
    class C1,class C2>
inline
Slot0<R>
  bind(const Slot2<R,C1,C2> &s,
       A1 a1,A2 a2)
  {return AdaptorBindSlot0_2<R,
           C1,C2>::create(s.data(),a1,a2);
  }


/****************************************************************
***** Adaptor Bind Slot 1 arguments, 2 hidden arguments
****************************************************************/
template <class R,
   class P1,
   class C1,class C2>
struct AdaptorBindSlot1_2: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot1<R,P1> SlotType;
   typedef Slot3<R,P1,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,
   class C1,class C2>
struct AdaptorBindSlot1_2
   <void,P1,
    C1,C2> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot1<void,P1> SlotType;
   typedef Slot3<void,P1,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,class A2,
    class R,
    class P1,
    class C1,class C2>
inline
Slot1<R,P1>
  bind(const Slot3<R,P1,C1,C2> &s,
       A1 a1,A2 a2)
  {return AdaptorBindSlot1_2<R,
           P1,
           C1,C2>::create(s.data(),a1,a2);
  }


/****************************************************************
***** Adaptor Bind Slot 2 arguments, 2 hidden arguments
****************************************************************/
template <class R,
   class P1,class P2,
   class C1,class C2>
struct AdaptorBindSlot2_2: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot2<R,P1,P2> SlotType;
   typedef Slot4<R,P1,P2,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,p2,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,
   class C1,class C2>
struct AdaptorBindSlot2_2
   <void,P1,P2,
    C1,C2> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot2<void,P1,P2> SlotType;
   typedef Slot4<void,P1,P2,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,p2,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,class A2,
    class R,
    class P1,class P2,
    class C1,class C2>
inline
Slot2<R,P1,P2>
  bind(const Slot4<R,P1,P2,C1,C2> &s,
       A1 a1,A2 a2)
  {return AdaptorBindSlot2_2<R,
           P1,P2,
           C1,C2>::create(s.data(),a1,a2);
  }


/****************************************************************
***** Adaptor Bind Slot 3 arguments, 2 hidden arguments
****************************************************************/
template <class R,
   class P1,class P2,class P3,
   class C1,class C2>
struct AdaptorBindSlot3_2: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot3<R,P1,P2,P3> SlotType;
   typedef Slot5<R,P1,P2,P3,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,
   class C1,class C2>
struct AdaptorBindSlot3_2
   <void,P1,P2,P3,
    C1,C2> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot3<void,P1,P2,P3> SlotType;
   typedef Slot5<void,P1,P2,P3,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,class A2,
    class R,
    class P1,class P2,class P3,
    class C1,class C2>
inline
Slot3<R,P1,P2,P3>
  bind(const Slot5<R,P1,P2,P3,C1,C2> &s,
       A1 a1,A2 a2)
  {return AdaptorBindSlot3_2<R,
           P1,P2,P3,
           C1,C2>::create(s.data(),a1,a2);
  }


/****************************************************************
***** Adaptor Bind Slot 4 arguments, 2 hidden arguments
****************************************************************/
template <class R,
   class P1,class P2,class P3,class P4,
   class C1,class C2>
struct AdaptorBindSlot4_2: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot4<R,P1,P2,P3,P4> SlotType;
   typedef Slot6<R,P1,P2,P3,P4,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,
   class C1,class C2>
struct AdaptorBindSlot4_2
   <void,P1,P2,P3,P4,
    C1,C2> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot4<void,P1,P2,P3,P4> SlotType;
   typedef Slot6<void,P1,P2,P3,P4,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,class A2,
    class R,
    class P1,class P2,class P3,class P4,
    class C1,class C2>
inline
Slot4<R,P1,P2,P3,P4>
  bind(const Slot6<R,P1,P2,P3,P4,C1,C2> &s,
       A1 a1,A2 a2)
  {return AdaptorBindSlot4_2<R,
           P1,P2,P3,P4,
           C1,C2>::create(s.data(),a1,a2);
  }


/****************************************************************
***** Adaptor Bind Slot 5 arguments, 2 hidden arguments
****************************************************************/
template <class R,
   class P1,class P2,class P3,class P4,class P5,
   class C1,class C2>
struct AdaptorBindSlot5_2: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef R RType;
#else
   typedef typename Trait<R>::type RType;
#endif
   typedef Slot5<R,P1,P2,P3,P4,P5> SlotType;
   typedef Slot7<R,P1,P2,P3,P4,P5,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      return ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,
   class C1,class C2>
struct AdaptorBindSlot5_2
   <void,P1,P2,P3,P4,P5,
    C1,C2> : public AdaptorSlot_
  {
   typedef void RType;
   typedef Slot5<void,P1,P2,P3,P4,P5> SlotType;
   typedef Slot7<void,P1,P2,P3,P4,P5,C1,C2> InSlotType;

   struct Node:public AdaptorNode
     {
      C1 c1_;
      C2 c2_;

      Node(C1 c1,C2 c2)
	: c1_(c1),c2_(c2)
 	{ }
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
       ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,
                                     node->c1_,node->c2_);
     }
   static SlotData* create(SlotData *s,C1 c1,C2 c2)
     {
      Node *node=new Node(c1,c2);
      copy_callback(s,node);
      CallData &data=reinterpret_cast<CallData&>(s->data_);
      data.callback=&callback;
      data.obj=node;
      return s;
     }
  };

#endif
#endif

template <class A1,class A2,
    class R,
    class P1,class P2,class P3,class P4,class P5,
    class C1,class C2>
inline
Slot5<R,P1,P2,P3,P4,P5>
  bind(const Slot7<R,P1,P2,P3,P4,P5,C1,C2> &s,
       A1 a1,A2 a2)
  {return AdaptorBindSlot5_2<R,
           P1,P2,P3,P4,P5,
           C1,C2>::create(s.data(),a1,a2);
  }



#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
