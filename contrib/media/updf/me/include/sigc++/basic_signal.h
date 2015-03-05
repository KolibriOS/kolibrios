// -*- c++ -*-
/* This is a generated file, do not edit.  Generated from ../sigc++/macros/basic_signal.h.m4 */

#ifndef SIGCXX_BASIC_SIGNAL_H
#define SIGCXX_BASIC_SIGNAL_H
#include <sigc++/marshal.h>
#include <sigc++/slot.h>

// Qt steals a method name.
#ifdef SIGC_QT
#undef emit
#endif

#ifdef emit
#define SIGC_QT
#undef emit
#endif



#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

/****************************************************************
*****  Signals (build by macros)
****************************************************************/

// common part to all signals
class LIBSIGC_API Signal_
  {
    private:
      Signal_(const Signal_&);

    protected:
      typedef ScopeList List;
   
      struct LIBSIGC_API Impl 
        { 
          typedef ScopeList List;
          List incoming_;
          List outgoing_;
          Impl();
          ~Impl();
        };

      Impl *impl;

      SlotData* in_connect();
      SlotData* out_connect(SlotData *s);

      Signal_();
      ~Signal_();

    public:
      bool empty() const;
      void clear();
  };


/****************************************************************
*****  Signal 0
****************************************************************/

template <class R,typename Marsh=class Marshal<R> >
  class Signal0:public Signal_
  {
   public:
     typedef Slot0<R>                       InSlotType;
     typedef Slot0<typename Marsh::OutType> OutSlotType;

   private:
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef typename Marsh::OutType SType;
     typedef R RType;
#else
     typedef Trait<typename Marsh::OutType>::type SType;
     typedef Trait<R>::type RType;
#endif
     typedef typename InSlotType::Callback Callback;
     typedef Signal0<R,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static SType callback(void* d)
       {
        CallData* data=(CallData*)d;
        return data->obj->emit();
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData &data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=(typename OutSlotType::Func)callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     SType emit();
     SType operator()()
       {return emit();}

     Signal0() {}
     Signal0(const InSlotType &s) {connect(s);}
     ~Signal0() {}
  };


// emit
template <class R,class Marsh>
typename  Signal0<R,Marsh>::SType Signal0<R,Marsh>::
  emit()
  {
   if (!impl||impl->outgoing_.empty()) return Marsh::default_value();
   List &out=impl->outgoing_; 
   Marsh rc;
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      if (rc.marshal(s.call())) return rc.value();
     } 
   return rc.value();
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class Marsh>
class Signal0<void,Marsh>
  :public Signal_
  {
   public:
     typedef Slot0<void> InSlotType;
     typedef Slot0<void> OutSlotType;
   private:
     typedef InSlotType::Callback Callback;
     typedef Signal0<void,Marsh> Self;
     typedef CallDataObj2<OutSlotType::Func,Self> CallData;

     static void callback(void* d)
       {
        CallData* data=(CallData*)d;
        data->obj->emit();
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData& data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     void emit();
     void operator()()
       {emit();}

     Signal0() {}
     Signal0(const InSlotType &s) {connect(s);}
     ~Signal0() {}
  };


// emit
template <class Marsh>
void Signal0<void,Marsh>::
  emit()
  {
   if (!impl||impl->outgoing_.empty()) return;
   List &out=impl->outgoing_; 
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      s.call();
     }
  }

#endif


/****************************************************************
*****  Signal 1
****************************************************************/

template <class R,class P1,typename Marsh=class Marshal<R> >
  class Signal1:public Signal_
  {
   public:
     typedef Slot1<R,P1>                       InSlotType;
     typedef Slot1<typename Marsh::OutType,P1> OutSlotType;

   private:
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef typename Marsh::OutType SType;
     typedef R RType;
#else
     typedef Trait<typename Marsh::OutType>::type SType;
     typedef Trait<R>::type RType;
#endif
     typedef typename InSlotType::Callback Callback;
     typedef Signal1<R,P1,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static SType callback(void* d,P1 p1)
       {
        CallData* data=(CallData*)d;
        return data->obj->emit(p1);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData &data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=(typename OutSlotType::Func)callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     SType emit(typename Trait<P1>::ref p1);
     SType operator()(typename Trait<P1>::ref p1)
       {return emit(p1);}

     Signal1() {}
     Signal1(const InSlotType &s) {connect(s);}
     ~Signal1() {}
  };


// emit
template <class R,class P1,class Marsh>
typename  Signal1<R,P1,Marsh>::SType Signal1<R,P1,Marsh>::
  emit(typename Trait<P1>::ref p1)
  {
   if (!impl||impl->outgoing_.empty()) return Marsh::default_value();
   List &out=impl->outgoing_; 
   Marsh rc;
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      if (rc.marshal(s.call(p1))) return rc.value();
     } 
   return rc.value();
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class Marsh>
class Signal1<void,P1,Marsh>
  :public Signal_
  {
   public:
     typedef Slot1<void,P1> InSlotType;
     typedef Slot1<void,P1> OutSlotType;
   private:
     typedef typename InSlotType::Callback Callback;
     typedef Signal1<void,P1,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static void callback(void* d,P1 p1)
       {
        CallData* data=(CallData*)d;
        data->obj->emit(p1);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData& data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     void emit(typename Trait<P1>::ref p1);
     void operator()(typename Trait<P1>::ref p1)
       {emit(p1);}

     Signal1() {}
     Signal1(const InSlotType &s) {connect(s);}
     ~Signal1() {}
  };


// emit
template <class P1,class Marsh>
void Signal1<void,P1,Marsh>::
  emit(typename Trait<P1>::ref p1)
  {
   if (!impl||impl->outgoing_.empty()) return;
   List &out=impl->outgoing_; 
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      s.call(p1);
     }
  }

#endif


/****************************************************************
*****  Signal 2
****************************************************************/

template <class R,class P1,class P2,typename Marsh=class Marshal<R> >
  class Signal2:public Signal_
  {
   public:
     typedef Slot2<R,P1,P2>                       InSlotType;
     typedef Slot2<typename Marsh::OutType,P1,P2> OutSlotType;

   private:
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef typename Marsh::OutType SType;
     typedef R RType;
#else
     typedef Trait<typename Marsh::OutType>::type SType;
     typedef Trait<R>::type RType;
#endif
     typedef typename InSlotType::Callback Callback;
     typedef Signal2<R,P1,P2,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static SType callback(void* d,P1 p1,P2 p2)
       {
        CallData* data=(CallData*)d;
        return data->obj->emit(p1,p2);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData &data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=(typename OutSlotType::Func)callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     SType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2);
     SType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
       {return emit(p1,p2);}

     Signal2() {}
     Signal2(const InSlotType &s) {connect(s);}
     ~Signal2() {}
  };


// emit
template <class R,class P1,class P2,class Marsh>
typename  Signal2<R,P1,P2,Marsh>::SType Signal2<R,P1,P2,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
  {
   if (!impl||impl->outgoing_.empty()) return Marsh::default_value();
   List &out=impl->outgoing_; 
   Marsh rc;
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      if (rc.marshal(s.call(p1,p2))) return rc.value();
     } 
   return rc.value();
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class Marsh>
class Signal2<void,P1,P2,Marsh>
  :public Signal_
  {
   public:
     typedef Slot2<void,P1,P2> InSlotType;
     typedef Slot2<void,P1,P2> OutSlotType;
   private:
     typedef typename InSlotType::Callback Callback;
     typedef Signal2<void,P1,P2,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static void callback(void* d,P1 p1,P2 p2)
       {
        CallData* data=(CallData*)d;
        data->obj->emit(p1,p2);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData& data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     void emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2);
     void operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
       {emit(p1,p2);}

     Signal2() {}
     Signal2(const InSlotType &s) {connect(s);}
     ~Signal2() {}
  };


// emit
template <class P1,class P2,class Marsh>
void Signal2<void,P1,P2,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2)
  {
   if (!impl||impl->outgoing_.empty()) return;
   List &out=impl->outgoing_; 
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      s.call(p1,p2);
     }
  }

#endif


/****************************************************************
*****  Signal 3
****************************************************************/

template <class R,class P1,class P2,class P3,typename Marsh=class Marshal<R> >
  class Signal3:public Signal_
  {
   public:
     typedef Slot3<R,P1,P2,P3>                       InSlotType;
     typedef Slot3<typename Marsh::OutType,P1,P2,P3> OutSlotType;

   private:
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef typename Marsh::OutType SType;
     typedef R RType;
#else
     typedef Trait<typename Marsh::OutType>::type SType;
     typedef Trait<R>::type RType;
#endif
     typedef typename InSlotType::Callback Callback;
     typedef Signal3<R,P1,P2,P3,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static SType callback(void* d,P1 p1,P2 p2,P3 p3)
       {
        CallData* data=(CallData*)d;
        return data->obj->emit(p1,p2,p3);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData &data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=(typename OutSlotType::Func)callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     SType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3);
     SType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
       {return emit(p1,p2,p3);}

     Signal3() {}
     Signal3(const InSlotType &s) {connect(s);}
     ~Signal3() {}
  };


// emit
template <class R,class P1,class P2,class P3,class Marsh>
typename  Signal3<R,P1,P2,P3,Marsh>::SType Signal3<R,P1,P2,P3,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
  {
   if (!impl||impl->outgoing_.empty()) return Marsh::default_value();
   List &out=impl->outgoing_; 
   Marsh rc;
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      if (rc.marshal(s.call(p1,p2,p3))) return rc.value();
     } 
   return rc.value();
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class Marsh>
class Signal3<void,P1,P2,P3,Marsh>
  :public Signal_
  {
   public:
     typedef Slot3<void,P1,P2,P3> InSlotType;
     typedef Slot3<void,P1,P2,P3> OutSlotType;
   private:
     typedef typename InSlotType::Callback Callback;
     typedef Signal3<void,P1,P2,P3,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static void callback(void* d,P1 p1,P2 p2,P3 p3)
       {
        CallData* data=(CallData*)d;
        data->obj->emit(p1,p2,p3);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData& data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     void emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3);
     void operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
       {emit(p1,p2,p3);}

     Signal3() {}
     Signal3(const InSlotType &s) {connect(s);}
     ~Signal3() {}
  };


// emit
template <class P1,class P2,class P3,class Marsh>
void Signal3<void,P1,P2,P3,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3)
  {
   if (!impl||impl->outgoing_.empty()) return;
   List &out=impl->outgoing_; 
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      s.call(p1,p2,p3);
     }
  }

#endif


/****************************************************************
*****  Signal 4
****************************************************************/

template <class R,class P1,class P2,class P3,class P4,typename Marsh=class Marshal<R> >
  class Signal4:public Signal_
  {
   public:
     typedef Slot4<R,P1,P2,P3,P4>                       InSlotType;
     typedef Slot4<typename Marsh::OutType,P1,P2,P3,P4> OutSlotType;

   private:
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef typename Marsh::OutType SType;
     typedef R RType;
#else
     typedef Trait<typename Marsh::OutType>::type SType;
     typedef Trait<R>::type RType;
#endif
     typedef typename InSlotType::Callback Callback;
     typedef Signal4<R,P1,P2,P3,P4,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static SType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
       {
        CallData* data=(CallData*)d;
        return data->obj->emit(p1,p2,p3,p4);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData &data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=(typename OutSlotType::Func)callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     SType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4);
     SType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
       {return emit(p1,p2,p3,p4);}

     Signal4() {}
     Signal4(const InSlotType &s) {connect(s);}
     ~Signal4() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class Marsh>
typename  Signal4<R,P1,P2,P3,P4,Marsh>::SType Signal4<R,P1,P2,P3,P4,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
  {
   if (!impl||impl->outgoing_.empty()) return Marsh::default_value();
   List &out=impl->outgoing_; 
   Marsh rc;
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      if (rc.marshal(s.call(p1,p2,p3,p4))) return rc.value();
     } 
   return rc.value();
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class Marsh>
class Signal4<void,P1,P2,P3,P4,Marsh>
  :public Signal_
  {
   public:
     typedef Slot4<void,P1,P2,P3,P4> InSlotType;
     typedef Slot4<void,P1,P2,P3,P4> OutSlotType;
   private:
     typedef typename InSlotType::Callback Callback;
     typedef Signal4<void,P1,P2,P3,P4,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static void callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4)
       {
        CallData* data=(CallData*)d;
        data->obj->emit(p1,p2,p3,p4);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData& data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     void emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4);
     void operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
       {emit(p1,p2,p3,p4);}

     Signal4() {}
     Signal4(const InSlotType &s) {connect(s);}
     ~Signal4() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class Marsh>
void Signal4<void,P1,P2,P3,P4,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4)
  {
   if (!impl||impl->outgoing_.empty()) return;
   List &out=impl->outgoing_; 
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      s.call(p1,p2,p3,p4);
     }
  }

#endif


/****************************************************************
*****  Signal 5
****************************************************************/

template <class R,class P1,class P2,class P3,class P4,class P5,typename Marsh=class Marshal<R> >
  class Signal5:public Signal_
  {
   public:
     typedef Slot5<R,P1,P2,P3,P4,P5>                       InSlotType;
     typedef Slot5<typename Marsh::OutType,P1,P2,P3,P4,P5> OutSlotType;

   private:
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef typename Marsh::OutType SType;
     typedef R RType;
#else
     typedef Trait<typename Marsh::OutType>::type SType;
     typedef Trait<R>::type RType;
#endif
     typedef typename InSlotType::Callback Callback;
     typedef Signal5<R,P1,P2,P3,P4,P5,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static SType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
       {
        CallData* data=(CallData*)d;
        return data->obj->emit(p1,p2,p3,p4,p5);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData &data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=(typename OutSlotType::Func)callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     SType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5);
     SType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
       {return emit(p1,p2,p3,p4,p5);}

     Signal5() {}
     Signal5(const InSlotType &s) {connect(s);}
     ~Signal5() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class Marsh>
typename  Signal5<R,P1,P2,P3,P4,P5,Marsh>::SType Signal5<R,P1,P2,P3,P4,P5,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
  {
   if (!impl||impl->outgoing_.empty()) return Marsh::default_value();
   List &out=impl->outgoing_; 
   Marsh rc;
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      if (rc.marshal(s.call(p1,p2,p3,p4,p5))) return rc.value();
     } 
   return rc.value();
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class Marsh>
class Signal5<void,P1,P2,P3,P4,P5,Marsh>
  :public Signal_
  {
   public:
     typedef Slot5<void,P1,P2,P3,P4,P5> InSlotType;
     typedef Slot5<void,P1,P2,P3,P4,P5> OutSlotType;
   private:
     typedef typename InSlotType::Callback Callback;
     typedef Signal5<void,P1,P2,P3,P4,P5,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static void callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5)
       {
        CallData* data=(CallData*)d;
        data->obj->emit(p1,p2,p3,p4,p5);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData& data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     void emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5);
     void operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
       {emit(p1,p2,p3,p4,p5);}

     Signal5() {}
     Signal5(const InSlotType &s) {connect(s);}
     ~Signal5() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class Marsh>
void Signal5<void,P1,P2,P3,P4,P5,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5)
  {
   if (!impl||impl->outgoing_.empty()) return;
   List &out=impl->outgoing_; 
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      s.call(p1,p2,p3,p4,p5);
     }
  }

#endif


/****************************************************************
*****  Signal 6
****************************************************************/

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,typename Marsh=class Marshal<R> >
  class Signal6:public Signal_
  {
   public:
     typedef Slot6<R,P1,P2,P3,P4,P5,P6>                       InSlotType;
     typedef Slot6<typename Marsh::OutType,P1,P2,P3,P4,P5,P6> OutSlotType;

   private:
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef typename Marsh::OutType SType;
     typedef R RType;
#else
     typedef Trait<typename Marsh::OutType>::type SType;
     typedef Trait<R>::type RType;
#endif
     typedef typename InSlotType::Callback Callback;
     typedef Signal6<R,P1,P2,P3,P4,P5,P6,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static SType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
       {
        CallData* data=(CallData*)d;
        return data->obj->emit(p1,p2,p3,p4,p5,p6);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData &data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=(typename OutSlotType::Func)callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     SType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6);
     SType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
       {return emit(p1,p2,p3,p4,p5,p6);}

     Signal6() {}
     Signal6(const InSlotType &s) {connect(s);}
     ~Signal6() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class Marsh>
typename  Signal6<R,P1,P2,P3,P4,P5,P6,Marsh>::SType Signal6<R,P1,P2,P3,P4,P5,P6,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
  {
   if (!impl||impl->outgoing_.empty()) return Marsh::default_value();
   List &out=impl->outgoing_; 
   Marsh rc;
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      if (rc.marshal(s.call(p1,p2,p3,p4,p5,p6))) return rc.value();
     } 
   return rc.value();
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6,class Marsh>
class Signal6<void,P1,P2,P3,P4,P5,P6,Marsh>
  :public Signal_
  {
   public:
     typedef Slot6<void,P1,P2,P3,P4,P5,P6> InSlotType;
     typedef Slot6<void,P1,P2,P3,P4,P5,P6> OutSlotType;
   private:
     typedef typename InSlotType::Callback Callback;
     typedef Signal6<void,P1,P2,P3,P4,P5,P6,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static void callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6)
       {
        CallData* data=(CallData*)d;
        data->obj->emit(p1,p2,p3,p4,p5,p6);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData& data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     void emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6);
     void operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
       {emit(p1,p2,p3,p4,p5,p6);}

     Signal6() {}
     Signal6(const InSlotType &s) {connect(s);}
     ~Signal6() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class P6,class Marsh>
void Signal6<void,P1,P2,P3,P4,P5,P6,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6)
  {
   if (!impl||impl->outgoing_.empty()) return;
   List &out=impl->outgoing_; 
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      s.call(p1,p2,p3,p4,p5,p6);
     }
  }

#endif


/****************************************************************
*****  Signal 7
****************************************************************/

template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,typename Marsh=class Marshal<R> >
  class Signal7:public Signal_
  {
   public:
     typedef Slot7<R,P1,P2,P3,P4,P5,P6,P7>                       InSlotType;
     typedef Slot7<typename Marsh::OutType,P1,P2,P3,P4,P5,P6,P7> OutSlotType;

   private:
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef typename Marsh::OutType SType;
     typedef R RType;
#else
     typedef Trait<typename Marsh::OutType>::type SType;
     typedef Trait<R>::type RType;
#endif
     typedef typename InSlotType::Callback Callback;
     typedef Signal7<R,P1,P2,P3,P4,P5,P6,P7,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static SType callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
       {
        CallData* data=(CallData*)d;
        return data->obj->emit(p1,p2,p3,p4,p5,p6,p7);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData &data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=(typename OutSlotType::Func)callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     SType emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7);
     SType operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
       {return emit(p1,p2,p3,p4,p5,p6,p7);}

     Signal7() {}
     Signal7(const InSlotType &s) {connect(s);}
     ~Signal7() {}
  };


// emit
template <class R,class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Marsh>
typename  Signal7<R,P1,P2,P3,P4,P5,P6,P7,Marsh>::SType Signal7<R,P1,P2,P3,P4,P5,P6,P7,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
  {
   if (!impl||impl->outgoing_.empty()) return Marsh::default_value();
   List &out=impl->outgoing_; 
   Marsh rc;
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      if (rc.marshal(s.call(p1,p2,p3,p4,p5,p6,p7))) return rc.value();
     } 
   return rc.value();
  }

#ifdef SIGC_CXX_PARTIAL_SPEC
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Marsh>
class Signal7<void,P1,P2,P3,P4,P5,P6,P7,Marsh>
  :public Signal_
  {
   public:
     typedef Slot7<void,P1,P2,P3,P4,P5,P6,P7> InSlotType;
     typedef Slot7<void,P1,P2,P3,P4,P5,P6,P7> OutSlotType;
   private:
     typedef typename InSlotType::Callback Callback;
     typedef Signal7<void,P1,P2,P3,P4,P5,P6,P7,Marsh> Self;
     typedef CallDataObj2<typename OutSlotType::Func,Self> CallData;

     static void callback(void* d,P1 p1,P2 p2,P3 p3,P4 p4,P5 p5,P6 p6,P7 p7)
       {
        CallData* data=(CallData*)d;
        data->obj->emit(p1,p2,p3,p4,p5,p6,p7);
       }

   public:
     OutSlotType slot()
       {
        SlotData* tmp=in_connect();
        CallData& data=reinterpret_cast<CallData&>(tmp->data_);
        data.callback=callback;
        data.obj=this;
        return tmp;
       }

     Connection connect(const InSlotType &s)
       {
        return out_connect(s.data());
       }

     void emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7);
     void operator()(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
       {emit(p1,p2,p3,p4,p5,p6,p7);}

     Signal7() {}
     Signal7(const InSlotType &s) {connect(s);}
     ~Signal7() {}
  };


// emit
template <class P1,class P2,class P3,class P4,class P5,class P6,class P7,class Marsh>
void Signal7<void,P1,P2,P3,P4,P5,P6,P7,Marsh>::
  emit(typename Trait<P1>::ref p1,typename Trait<P2>::ref p2,typename Trait<P3>::ref p3,typename Trait<P4>::ref p4,typename Trait<P5>::ref p5,typename Trait<P6>::ref p6,typename Trait<P7>::ref p7)
  {
   if (!impl||impl->outgoing_.empty()) return;
   List &out=impl->outgoing_; 
   SlotData *data;
   List::Iterator i=out.begin();
   while (i!=out.end())
     {
      data=((SlotDependent*)(i.node()))->parent();
      ++i;
      Callback& s=(Callback&)(data->callback());
      s.call(p1,p2,p3,p4,p5,p6,p7);
     }
  }

#endif



#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#ifdef SIGC_QT
#define emit
#endif


#endif // SIGCXX_BASIC_SIGNAL_H

