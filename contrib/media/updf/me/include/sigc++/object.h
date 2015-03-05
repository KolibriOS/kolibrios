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
#ifndef SIGCXX_OBJECT_H
#define SIGCXX_OBJECT_H
#include <sigc++config.h>
#include <sigc++/scope.h>


#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

extern int sigc_micro_version;
extern int sigc_minor_version;
extern int sigc_major_version;

class Invalid_;
class LIBSIGC_API ObjectReferenced
  {
   friend class Reference;
   friend class Scope;
   friend class Invalid_;

#ifdef SIGC_CXX_FRIEND_TEMPLATES
   template <class T>
     friend T* manage(T*);
#endif

   protected:
     // count of current references
     unsigned int obj_count_    :24;

     // indicates object generated through an interface that marks dynamic
     unsigned int obj_dynamic_  :1;

     // indicates the pointed to scope is the owner
     unsigned int obj_owned_    :1;

     // indicates object not will delete when count reachs zero
     unsigned int obj_floating_ :1;

     // indicates the owned scope is surrendering ownership
     unsigned int obj_transfer_ :1;

     // indicates the object is doing a list clean up 
     unsigned int obj_invalid_  :1;

     // indicates the object been destroyed 
     unsigned int obj_destroyed_ :1;

     // indicates there is a weak reference
     unsigned int obj_weak_ :1;


   /*************************************************************/
#ifdef SIGC_CXX_FRIEND_TEMPLATES
   protected:
#else
   public:
#endif
     // For the controller and scope
     virtual void set_dynamic(); 
     inline void set_sink() {obj_floating_=0;}

   protected:

     inline void register_ref(Reference *)
       {  
        if (obj_transfer_) 
         {
          obj_transfer_=0;
          obj_owned_=0;
         }
       }
  
   public:
     virtual void reference();
     virtual void unreference();

     inline bool is_dynamic()  {return obj_dynamic_;}
     inline bool is_floating() {return obj_floating_;}

     ObjectReferenced();
     virtual ~ObjectReferenced();
  };


class LIBSIGC_API ObjectScoped :public ObjectReferenced
  {
   friend class Scope;
   typedef ScopeList List_;

   private:
     mutable List_ list_;

   // interface for scopes
     void register_scope(Scope *scope,const Scope *parent=0);
     void unregister_scope(Scope *scope);

   protected:
     // This sets a very weak reference which is removed at next invalid
     void set_weak();

   public:
     void register_data(ScopeNode* data);

     // inform connections that object wishs to delete
     void invalid(bool destroy=0);

     ObjectScoped();
     virtual ~ObjectScoped();
  };


// There can only be one Scope_Object per any object
class LIBSIGC_API Object: public virtual ObjectScoped 
  {
   public:
     Object() {}
     virtual ~Object();
  };

// mark this a managable object
template <class T>
inline T* manage(T* t)
  { 
   if (t) t->set_dynamic();
   return t;
  }


#ifdef SIGC_CXX_NAMESPACES
} // namespace
#endif

#endif
