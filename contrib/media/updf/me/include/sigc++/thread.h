// -*- c++ -*-
/* 
 * Copyright 1999 Karl Nelson <kenelson@ece.ucdavis.edu>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef SIGCXX_THREAD_H
#define SIGCXX_THREAD_H
#include <sigc++config.h>

#ifdef SIGC_PTHREADS

#ifdef SIGC_THREAD_IMPL
#include <pthread.h>
#else
#include <time.h>
#endif

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
namespace Threads
{
#else
#define Threads 
#endif

#ifdef SIGC_THREAD_IMPL
#ifdef SIGC_PTHREAD_DCE
struct CondAttr { pthread_condattr_t impl_;};
struct MutexAttr { pthread_mutexattr_t impl_;};
struct ThreadAttr { pthread_attr_t impl_;};
#else
struct CondAttr { pthread_condattr_t* impl_;};
struct MutexAttr { pthread_mutexattr_t* impl_;};
struct ThreadAttr { pthread_attr_t* impl_;};
#endif
typedef pthread_mutex_t MutexImpl;
typedef pthread_cond_t CondImpl;
typedef pthread_key_t KeyImpl;
typedef pthread_t ThreadImpl;
#else
class CondAttr {unsigned char dummy[SIGC_PTHREAD_COND_ATTR];};
class CondImpl {unsigned char dummy[SIGC_PTHREAD_COND_IMPL];};
class MutexAttr {unsigned char dummy[SIGC_PTHREAD_MUTEX_ATTR];};
class MutexImpl {unsigned char dummy[SIGC_PTHREAD_MUTEX_IMPL];};
class ThreadAttr {unsigned char dummy[SIGC_PTHREAD_THREAD_ATTR];};
class ThreadImpl {unsigned char dummy[SIGC_PTHREAD_THREAD_IMPL];};
class KeyImpl {unsigned char dummy[SIGC_PTHREAD_KEY_IMPL];};
#endif

// Mutual Exclusion
class Mutex
  {
   typedef MutexImpl Impl;
   private:
     Impl mutex_;
     int destroy();

   public:
     static MutexAttr Default;
#ifdef SIGC_THREAD_IMPL
     operator Impl* ()  {return (Impl*)(&mutex_);}
#endif

     Mutex(const MutexAttr attr=Default); 

     // (needs work) 
     ~Mutex();

     int lock();
     int trylock();
     int unlock();
  };

// A lazy way to unlock at end of scope
struct MLock
  {
   Mutex &mutex_;
   MLock(Mutex& mutex):mutex_(mutex) {mutex_.lock();}
   ~MLock()                          {mutex_.unlock();}
  };

// Condition Variable
struct Condition
  {
   typedef CondImpl Impl;
   private:
     Impl cond_;

     int destroy();
   public:
     static CondAttr Default;
#ifdef SIGC_THREAD_IMPL
     operator Impl* ()  {return (Impl*)(&cond_);}
#endif

     Condition(const CondAttr &attr=Default);
     ~Condition();

     // restarts exactly one thread hung on condition
     int signal();

     // restarts all threads waiting on condition
     int broadcast();

     // unlocks a mutex while waiting on a condition, then reaquires lock.
     int wait(Mutex &m);

     // unlocks a mutex while waiting on a condition, then reaquires lock
     // with a fixed maximum duration.
     int wait(Mutex &m,struct timespec* spec);

  };

// Integer Semaphore
struct Semaphore
  {
   int value_;
   Condition sig_;
   Mutex access_;

   void up();
   void down();
   
   Semaphore(int value=1);
   ~Semaphore();
  };

struct Private_
  {
    KeyImpl key_;
    void* get();
    void set(void *value);
    void create(void (*dtor)(void*));
    void destroy();
  };

// Private is a thread split static.  
template <class T>
class Private : private Private_
  {
    private:
      static void dtor(void* v)
        {
          T* obj=(T*)v;
          delete obj;
        }

    public:

      T& operator =(const T& t)
        {return (((T&)*this)=t);}

      operator T& ()
        {
          T *value=(T*)get();
          if (!value)
            set((void*)(value=new T()));  
          return *(value);
        }

      Private()  { create(&dtor); }
      ~Private() { destroy(); }
  };

// int needs to initialized
template <>
class Private<int> : private Private_
  {
    private:
      static void dtor(void* v)
        {
          int* obj=(int*)v;
          delete obj;
        }

      public:
        int& operator =(const int& t)
          {return (((int&)*this)=t);}

        operator int& ()
          {
           int *value=(int*)get();
           if (!value)
             set((void*)(value=new int(0)));  
           return *(value); 
          }

        Private() { create(&dtor); }
        ~Private() { destroy(); }
  };

struct Thread
  {
   protected:
     typedef ThreadImpl Impl;
     Impl thread_;
     void*     arg_;
     ThreadAttr attr_;

     static void* call_main_(void* obj);

   public:
#ifdef SIGC_THREAD_IMPL
     operator Impl* () {return &thread_;}
#endif

     virtual void* main(void*)=0;
     int detach();

     static ThreadAttr Default;

     // arg is for passing extra data to main, but never pass a
     // local variable or address of local variable.  Arg must
     // be available throughout life of program.
     int start(void* arg=0);
     
     Thread(const ThreadAttr &attr=Default);
     virtual ~Thread();
  };


#ifdef SIGC_CXX_NAMESPACES
} /* namespace Threads */
} /* namespace SigC */
#endif 

#endif /* SIGC_PTHREADS */
#endif /* SIGCXX_THREAD_H */
