#ifndef SIGCXX_HANDLE_H
#define SIGCXX_HANDLE_H
#include <sigc++config.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

// Signiture for Handles
template <class Obj,class Scope_>
  class Handle
  {
   protected:
     Scope_ scope_;
   public:
     // access
     Obj* obj()
       {
        return static_cast<Obj*>(scope_.cache());
       }
     Obj* obj() const
       {
        return static_cast<Obj*>(scope_.cache());
       }

     bool connected() const
       {return  (scope_.object()!=0);}
     operator Obj*() 
       {return  (obj());}
     operator Obj*() const
       {return  (obj());}

     Obj& operator*() const
       {return *(obj());}
     Obj* operator->() const
       {return  (obj());}

     Scope_& scope()
       {return scope_;}
     const Scope_& scope() const
       {return scope_;}

     void disconnect()
       {scope_.disconnect(0);}

     // copy
     Handle& operator =(Obj* obj)
       {scope_.set(obj,obj,true); return *this;}
     Handle& operator =(Obj& obj)
       {scope_.set(&obj,&obj,false); return *this;}
#ifndef SIGC_CXX_TEMPLATE_CCTOR
     Handle& operator =(const Handle& handle)
       {
        Obj *o=handle.obj();
        scope_.set(o,o,false);
        return *this;
       }
#endif
     template <class Obj2,class Scope2>
       Handle& operator = (const Handle<Obj2,Scope2>& handle)
       {
        Obj *o=handle.obj();
        scope_.set(o,o,false);
        return *this;
       }

     // construct
     Handle():scope_()           {}
     Handle(Obj *obj):scope_()   {scope_.set(obj,obj,true);}
     Handle(Obj &obj):scope_()   {scope_.set(&obj,&obj,false);}
#ifndef SIGC_CXX_TEMPLATE_CCTOR
     Handle(const Handle& handle)
       :scope_() 
       {
        Obj *o=handle.obj();
        scope_.set(o,o,false);
       }
#endif
     template <class Obj2,class Scope2>
     Handle(const Handle<Obj2,Scope2>& handle)
       :scope_()
       {
        Obj *o=handle.obj();
        scope_.set(o,o,false);
       }
  };

#define HANDLE_CTORS(X,T,P)                  \
public:                                       \
  X(T *t):Handle<T,P>(t) {}                    \
  X(T &t):Handle<T,P>(t) {}                     \
  template <class T2,class P2>                   \
    X(const Handle<T2,P2> &h):Handle<T,P>(h) {}   \
  X& operator =(T *t)                              \
    {return Handle<T,P>::operator=(t);}             \
  X& operator =(T &t)                                \
    {return Handle<T,P>::operator=(t);}               \
  template <class T2,class P2>                         \
  X& operator =(const Handle<T2,P2> &t)                 \
    {return Handle<T,P>::operator=(t);}

//template <class T>
//  class Ref:public Handle<T,Scopes::RefCount>
//    {
//     HANDLE_CTORS(Ref,T,Scopes::RefCount)
//    };

#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
