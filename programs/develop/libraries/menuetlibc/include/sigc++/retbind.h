// -*- c++ -*-
/* This is a generated file, do not edit.  Generated from ../sigc++/macros/retbind.h.m4 */
/* This was also shamelessly copied, hacked, munched, and carefully 
 * tweaked from KNelson's original bind.h.m4
 * CJN 3.22.00
 */

#ifndef SIGCXX_RETBIND_H
#define SIGCXX_RETBIND_H

/*
  SigC::retbind
  -------------
  retbind() alters a Slot by fixing the return value to certain values

  Return value fixing ignores any slot return value.  The slot is
  destroyed in the process and a new one is created, so references
  holding onto the slot will no longer be valid.

  Typecasting may be necessary to match arguments between the
  slot and the binding return value.  Types must be an exact match.
  To insure the proper type, the type can be explicitly specified
  on template instantation.

  Simple Sample usage:

    void f(int,int);
    Slot2<void,int,int> s1=slot(f);

    Slot1<int,int,int>  s2=retbind(s1,1);  // s1 is invalid
    cout << "s2: " << s2(2,1) << endl;

  Type specified usage:
    
    struct A {};
    struct B :public A {};
    B* b;
    Slot1<void> s1;

    Slot0<A*> s2=retbind<A*>(s1,b);  // B* must be told to match A*
     
*/

#include <sigc++/adaptor.h>
#include <sigc++/scope.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif




/****************************************************************
***** Adaptor RetBind Slot 0 arguments
****************************************************************/
template <class Ret,class R>
struct AdaptorRetBindSlot0: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot0<Ret> SlotType;
   typedef Slot0<R> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))();
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Ret>
struct AdaptorRetBindSlot0
   <Ret,void> : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot0<Ret> SlotType;
   typedef Slot0<void> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))();
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif

template <class Ret,
    class R>
Slot0<Ret>
  retbind(const Slot0<R> &s,
       Ret ret)
  {return AdaptorRetBindSlot0<Ret,R>::create(s.data(),ret);
  }


/****************************************************************
***** Adaptor RetBind Slot 1 arguments
****************************************************************/
template <class Ret,class R,
   class P1>
struct AdaptorRetBindSlot1: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot1<Ret,P1> SlotType;
   typedef Slot1<R,P1> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Ret,class P1>
struct AdaptorRetBindSlot1
   <Ret,void,
   P1> : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot1<Ret,P1> SlotType;
   typedef Slot1<void,P1> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif

template <class Ret,
    class R,
    class P1>
Slot1<Ret,P1>
  retbind(const Slot1<R,P1> &s,
       Ret ret)
  {return AdaptorRetBindSlot1<Ret,R,
           P1>::create(s.data(),ret);
  }


/****************************************************************
***** Adaptor RetBind Slot 2 arguments
****************************************************************/
template <class Ret,class R,
   class P1,class P2>
struct AdaptorRetBindSlot2: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot2<Ret,P1,P2> SlotType;
   typedef Slot2<R,P1,P2> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Ret,class P1,class P2>
struct AdaptorRetBindSlot2
   <Ret,void,
   P1,P2> : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot2<Ret,P1,P2> SlotType;
   typedef Slot2<void,P1,P2> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif

template <class Ret,
    class R,
    class P1,class P2>
Slot2<Ret,P1,P2>
  retbind(const Slot2<R,P1,P2> &s,
       Ret ret)
  {return AdaptorRetBindSlot2<Ret,R,
           P1,P2>::create(s.data(),ret);
  }


/****************************************************************
***** Adaptor RetBind Slot 3 arguments
****************************************************************/
template <class Ret,class R,
   class P1,class P2,class P3>
struct AdaptorRetBindSlot3: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot3<Ret,P1,P2,P3> SlotType;
   typedef Slot3<R,P1,P2,P3> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Ret,class P1,class P2,class P3>
struct AdaptorRetBindSlot3
   <Ret,void,
   P1,P2,P3> : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot3<Ret,P1,P2,P3> SlotType;
   typedef Slot3<void,P1,P2,P3> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif

template <class Ret,
    class R,
    class P1,class P2,class P3>
Slot3<Ret,P1,P2,P3>
  retbind(const Slot3<R,P1,P2,P3> &s,
       Ret ret)
  {return AdaptorRetBindSlot3<Ret,R,
           P1,P2,P3>::create(s.data(),ret);
  }


/****************************************************************
***** Adaptor RetBind Slot 4 arguments
****************************************************************/
template <class Ret,class R,
   class P1,class P2,class P3,class P4>
struct AdaptorRetBindSlot4: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot4<Ret,P1,P2,P3,P4> SlotType;
   typedef Slot4<R,P1,P2,P3,P4> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Ret,class P1,class P2,class P3,class P4>
struct AdaptorRetBindSlot4
   <Ret,void,
   P1,P2,P3,P4> : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot4<Ret,P1,P2,P3,P4> SlotType;
   typedef Slot4<void,P1,P2,P3,P4> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif

template <class Ret,
    class R,
    class P1,class P2,class P3,class P4>
Slot4<Ret,P1,P2,P3,P4>
  retbind(const Slot4<R,P1,P2,P3,P4> &s,
       Ret ret)
  {return AdaptorRetBindSlot4<Ret,R,
           P1,P2,P3,P4>::create(s.data(),ret);
  }


/****************************************************************
***** Adaptor RetBind Slot 5 arguments
****************************************************************/
template <class Ret,class R,
   class P1,class P2,class P3,class P4,class P5>
struct AdaptorRetBindSlot5: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot5<Ret,P1,P2,P3,P4,P5> SlotType;
   typedef Slot5<R,P1,P2,P3,P4,P5> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Ret,class P1,class P2,class P3,class P4,class P5>
struct AdaptorRetBindSlot5
   <Ret,void,
   P1,P2,P3,P4,P5> : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot5<Ret,P1,P2,P3,P4,P5> SlotType;
   typedef Slot5<void,P1,P2,P3,P4,P5> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif

template <class Ret,
    class R,
    class P1,class P2,class P3,class P4,class P5>
Slot5<Ret,P1,P2,P3,P4,P5>
  retbind(const Slot5<R,P1,P2,P3,P4,P5> &s,
       Ret ret)
  {return AdaptorRetBindSlot5<Ret,R,
           P1,P2,P3,P4,P5>::create(s.data(),ret);
  }


/****************************************************************
***** Adaptor RetBind Slot 6 arguments
****************************************************************/
template <class Ret,class R,
   class P1,class P2,class P3,class P4,class P5,class P6>
struct AdaptorRetBindSlot6: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot6<Ret,P1,P2,P3,P4,P5,P6> SlotType;
   typedef Slot6<R,P1,P2,P3,P4,P5,P6> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Ret,class P1,class P2,class P3,class P4,class P5,class P6>
struct AdaptorRetBindSlot6
   <Ret,void,
   P1,P2,P3,P4,P5,P6> : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot6<Ret,P1,P2,P3,P4,P5,P6> SlotType;
   typedef Slot6<void,P1,P2,P3,P4,P5,P6> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif

template <class Ret,
    class R,
    class P1,class P2,class P3,class P4,class P5,class P6>
Slot6<Ret,P1,P2,P3,P4,P5,P6>
  retbind(const Slot6<R,P1,P2,P3,P4,P5,P6> &s,
       Ret ret)
  {return AdaptorRetBindSlot6<Ret,R,
           P1,P2,P3,P4,P5,P6>::create(s.data(),ret);
  }


/****************************************************************
***** Adaptor RetBind Slot 7 arguments
****************************************************************/
template <class Ret,class R,
   class P1,class P2,class P3,class P4,class P5,class P6,class P7>
struct AdaptorRetBindSlot7: public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot7<Ret,P1,P2,P3,P4,P5,P6,P7> SlotType;
   typedef Slot7<R,P1,P2,P3,P4,P5,P6,P7> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6,p7);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };


#ifndef SIGC_CXX_VOID_RETURN
#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Ret,class P1,class P2,class P3,class P4,class P5,class P6,class P7>
struct AdaptorRetBindSlot7
   <Ret,void,
   P1,P2,P3,P4,P5,P6,P7> : public AdaptorSlot_
  {
#ifdef SIGC_CXX_PARTIAL_SPEC
   typedef Ret RType;
#else
   typedef typename Trait<Ret>::type RType;
#endif
   typedef Slot7<Ret,P1,P2,P3,P4,P5,P6,P7> SlotType;
   typedef Slot7<void,P1,P2,P3,P4,P5,P6,P7> InSlotType;

   struct Node:public AdaptorNode
     {
	   Ret ret_;
     };

   typedef CallDataObj2<typename SlotType::Func,Node> CallData;

   static RType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
     {
      CallData* data=(CallData*)d;
      Node* node=data->obj;
      ((typename InSlotType::Callback&)(node->data_))(p1,p2,p3,p4,p5,p6,p7);
      return node->ret_;
     }
   static SlotData* create(SlotData *s,Ret ret)
     {
      SlotData* tmp=(SlotData*)s;
      Node *node=new Node();
      copy_callback(tmp,node);
      node->ret_ = ret;
      CallData &data=reinterpret_cast<CallData&>(tmp->data_);
      data.callback=&callback;
      data.obj=node;
      return tmp;
     }
  };

#endif
#endif

template <class Ret,
    class R,
    class P1,class P2,class P3,class P4,class P5,class P6,class P7>
Slot7<Ret,P1,P2,P3,P4,P5,P6,P7>
  retbind(const Slot7<R,P1,P2,P3,P4,P5,P6,P7> &s,
       Ret ret)
  {return AdaptorRetBindSlot7<Ret,R,
           P1,P2,P3,P4,P5,P6,P7>::create(s.data(),ret);
  }



#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
