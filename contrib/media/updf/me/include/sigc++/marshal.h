#ifndef SIGCXX_MARSHALLER_H
#define SIGCXX_MARSHALLER_H
#include <sigc++config.h>

#ifndef SIGC_CXX_PARTIAL_SPEC
#include <sigc++/slot.h>
#endif

#ifdef SIGC_PTHREADS
#include <sigc++/thread.h>
#endif

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif 

/* 

All classes used to marshal return values should have the following

  class SomeMarshal
    {
     // both typedefs must be defined.
     typedef Type1 InType;
     typedef Type2 OutType;

     public:
       // Return final return code.
       OutType value();  

       // Captures return codes and returns TRUE to stop emittion.
       bool marshal(const InType&);

       SomeMarshal();
   };

It is not necessary for the InType to match the OutType.
This is to allow for things like list capturing.

*/

/*******************************************************************
***** Marshal 
*******************************************************************/

// A struct that holds an flag for determining 
// if the return value is to be ignored.  
class LIBSIGC_API RetCode
  {
   public:
     static int check_ignore();
     static void ignore();
  };

// Basic Marshal class.  
template <typename R>
class Marshal
  {
   public:
     typedef R OutType;
#ifdef SIGC_CXX_PARTIAL_SPEC
     typedef R InType;
   protected:
     typedef OutType OutType_;
#else
     typedef Trait<R>::type InType;
   protected:
     typedef InType OutType_;
#endif
     OutType_ value_;
   public:
     OutType_& value() {return value_;}

     static OutType_ default_value() 
#ifdef SIGC_CXX_INT_CTOR
       {return OutType_();}
#else
       {OutType_ r; new (&r) OutType_(); return r;}
#endif

     // This captures return values.  Return TRUE to stop emittion process.
     bool marshal(const InType& newval)
       {
        if (!RetCode::check_ignore()) value_=newval;
        return 0;  // continue emittion process
       };
     Marshal()
#ifdef SIGC_CXX_INT_CTOR
       :value_()
       {RetCode::check_ignore();}
#else
       {
        RetCode::check_ignore();
        new (&value_) OutType_();
       }
#endif
  };

#ifdef SIGC_CXX_SPECIALIZE_REFERENCES
// Basic Marshal class.
template <typename R>
class Marshal<R&>
  {
    public:
     typedef R& OutType;
     typedef R& InType;
     R* value_;
     OutType value() {return value_;}
     static OutType default_value() {return Default;}
     static R Default;

     // This captures return values.  Return TRUE to stop emittion process.
     bool marshal(InType newval)
       {
        if (!RetCode::check_ignore()) value_=&newval;
        return 0;  // continue emittion process
       };
     Marshal()
       :value_(&Default)
       {RetCode::check_ignore();}
     ~Marshal()
       {}
  };

template <typename T> T Marshal<T&>::Default;
#endif

#ifdef SIGC_CXX_PARTIAL_SPEC
// dummy marshaller for void type.
template <>
class Marshal<void>
  {
   public:
   Marshal() 
     {}
   ~Marshal()
     {}
  };
#endif


// starts with a fixed value
template <class R,R initial>
class FixedMarshal
  {
    public:
     typedef R OutType;
     typedef R InType;
     R value_;
     OutType& value() {return value_;}
     static OutType default_value() { return initial; }

     bool marshal(const InType& newval)
       {
        if (!RetCode::check_ignore()) value_=newval;
        return 0;  // continue emittion process
       };

     FixedMarshal()
       :value_(initial) 
       {RetCode::check_ignore();}
     ~FixedMarshal()
       {}
  };

template <class R>
struct FastMarshal
  {
     typedef R OutType;
     typedef R InType;

     R value_;
     OutType& value() {return value_;}
     static OutType default_value() 
#ifdef SIGC_CXX_INT_CTOR
       {return R();}
#else
       {R r; new (&r) R(); return r;}
#endif

     bool marshal(const InType& newval)
       {
        value_=newval;
        return 0;  // continue emittion process
       };

     FastMarshal()
#ifdef SIGC_CXX_INT_CTOR
       :value_()
       {}
#else
       {new (&value_) R();}
#endif
     ~FastMarshal()
       {}
  };



#ifdef SIGC_CXX_NAMESPACES
} // namespace sigc
#endif

#endif
