#ifndef SIGCXX_GENERATOR_H
#define SIGCXX_GENERATOR_H
#include <sigc++config.h>
#include <new>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif 


// 0
template <class T> 
  T* gen()
  {return manage(new T());}

// 1
template <class T,class P1> 
  T* gen(const P1& p1)
  {return manage(new T(p1));}

template <class T,class P1> 
  T* gen(P1& p1)
  {return manage(new T(p1));}

// 2
template <class T,class P1,class P2> 
  T* gen(const P1& p1,const P2& p2)
  {return manage(new T(p1,p2));}

template <class T,class P1,class P2> 
  T* gen(P1& p1,const P2& p2)
  {return manage(new T(p1,p2));}

template <class T,class P1,class P2> 
  T* gen(const P1& p1,P2& p2)
  {return manage(new T(p1,p2));}

template <class T,class P1,class P2> 
  T* gen(P1& p1,P2& p2)
  {return manage(new T(p1,p2));}

// 3
template <class T,class P1,class P2,class P3> 
  T* gen(const P1& p1,const P2& p2,const P3& p3)
  {return manage(new T(p1,p2,p3));}

template <class T,class P1,class P2,class P3> 
  T* gen(P1& p1,const P2& p2,const P3& p3)
  {return manage(new T(p1,p2,p3));}

template <class T,class P1,class P2,class P3> 
  T* gen(const P1& p1,P2& p2,const P3& p3)
  {return manage(new T(p1,p2,p3));}

template <class T,class P1,class P2,class P3> 
  T* gen(const P1& p1,const P2& p2,P3& p3)
  {return manage(new T(p1,p2,p3));}

template <class T,class P1,class P2,class P3> 
  T* gen(const P1& p1,P2& p2,P3& p3)
  {return manage(new T(p1,p2,p3));}

template <class T,class P1,class P2,class P3> 
  T* gen(P1& p1,const P2& p2,P3& p3)
  {return manage(new T(p1,p2,p3));}

template <class T,class P1,class P2,class P3> 
  T* gen(P1& p1,P2& p2,const P3& p3)
  {return manage(new T(p1,p2,p3));}

template <class T,class P1,class P2,class P3> 
  T* gen(P1& p1,P2& p2,P3& p3)
  {return manage(new T(p1,p2,p3));}

// 4
template <class T,class P1,class P2,class P3,class P4> 
  T* gen(const P1& p1,const P2& p2,const P3& p3,const P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(P1& p1,const P2& p2,const P3& p3,const P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(const P1& p1,P2& p2,const P3& p3,const P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(const P1& p1,const P2& p2,P3& p3,const P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(const P1& p1,const P2& p2,const P3& p3,P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(const P1& p1,P2& p2,P3& p3,P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(P1& p1,const P2& p2,P3& p3,P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(P1& p1,P2& p2,const P3& p3,P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(P1& p1,P2& p2,P3& p3,const P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(const P1& p1,const P2& p2,P3& p3,P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(const P1& p1,P2& p2,const P3& p3,P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(const P1& p1,P2& p2,P3& p3,const P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(P1& p1,const P2& p2,const P3& p3,P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(P1& p1,const P2& p2,P3& p3,const P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(P1& p1,P2& p2,const P3& p3,const P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

template <class T,class P1,class P2,class P3,class P4> 
  T* gen(P1& p1,P2& p2,P3& p3,P4& p4)
  {return manage(new T(p1,p2,p3,p4));}

//From here one we will just generate warnings
template <class T,class P1,class P2,class P3,class P4,class P5> 
  T* gen(P1& p1,P2& p2,P3& p3,P4& p4,P5& p5)
  {return manage(new T(p1,p2,p3,p4,p5));}

template <class T,class P1,class P2,class P3,class P4,class P5,class P6> 
  T* gen(P1& p1,P2& p2,P3& p3,P4& p4,P5& p5,P6& p6)
  {return manage(new T(p1,p2,p3,p4,p5,p6));}

template <class T,class P1,class P2,class P3,class P4,class P5,class P6,class P7> 
  T* gen(P1& p1,P2& p2,P3& p3,P4& p4,P5& p5,P6& p6,P7& p7)
  {return manage(new T(p1,p2,p3,p4,p5,p6,p7));}

#ifdef SIGC_CXX_NAMESPACES
} // namespace sigc
#endif

#endif

