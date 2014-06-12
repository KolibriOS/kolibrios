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
#ifndef SIGCXX_SCOPE_H
#define SIGCXX_SCOPE_H
#include <sigc++config.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif 


struct LIBSIGC_API ScopeNode 
  {
   mutable ScopeNode *prev_;
   mutable ScopeNode *next_;

   // removes self from list
   void remove_self();

   // Called to inform the item that it is erased
   virtual void erase();

   // inform scopes that invalid requested.
   virtual void disconnect(bool destroy=0);

   ScopeNode()
#ifdef LIBSIGC_WIN32
   {prev_=next_=this;}
#else
   :prev_(this),next_(this) {}
#endif

   virtual ~ScopeNode();

   private:
     ScopeNode& operator=(const ScopeNode&);
     ScopeNode(const ScopeNode&);
  };

struct LIBSIGC_API DataNode: public ScopeNode
  {
   virtual void erase();
   virtual ~DataNode();
  };

/*******************************************************************
***** Basis Scope
*******************************************************************/
class ObjectScoped;
class ObjectReferenced;
class Object;
class Scope;

class LIBSIGC_API Reference
  {
   protected:
     mutable ObjectReferenced* obj_;
     mutable void* cache_;

   public:
     void set_sink();

     void init(ObjectReferenced* obj);
     void set(ObjectReferenced* obj,void* cache=0,bool ptr=false);

     Reference& operator=(ObjectReferenced *obj) { set(obj); return *this; }
     Reference& operator=(ObjectReferenced &obj) { set(&obj); return *this; }
     Reference& operator=(const Reference& ref)  { set(ref.obj_); return *this; };

     ObjectReferenced* object() const {return obj_;}
     void* cache() const {return cache_;}

     Reference():obj_(0) {}
     Reference(ObjectReferenced &obj)
       {init(&obj);}
     Reference(const Reference& ref)
       {init(ref.obj_);}
     ~Reference();
  };
     
class LIBSIGC_API Scope:public ScopeNode
  {
   friend class ObjectScoped;

     Scope& operator=(const Scope& scope);
     Scope(const Scope& scope);

   protected:
     void set(ObjectScoped* obj,void* cache,bool ptr);
     mutable ObjectScoped* obj_;
     mutable void* cache_;

     virtual void on_connect()=0;
     virtual void erase();

     void register_scope(ObjectScoped *);
     void register_scope(const Scope *parent=0);
     void unregister_scope();

   public:

     void reference();
     void unreference();
     void set_sink();

     ObjectScoped* object() const {return (ObjectScoped*)(obj_);}
     void* cache() const {return cache_;}

     // Inform object it should invalidate its list.
     void invalid();

     Scope():obj_(0),cache_(0) {}
     virtual ~Scope();
  };


/****************************************************** 
**** Common Scopes
*******************************************************
  Available Scopes:
    Uncounted  - non-reference
    Limit      - Limits the lifetime of object to this scope
                 Sinks object.
    Extend     - Extends the lifetime of the object to this scope
                 Sinks object.
    LimitOwned - Conditionally limits the lifetime of object
                 Sinks object.
    FuncRef    - Extends the lifetime, without sink
                 (intended for functions)
    Reference  - Extends the lifetime, with sink
    
    AutoPtr    - Shorthand for auto_ptr like scope.
    RefCount   - Shorthand for ref_ptr like scope.
        
******************************************************/
struct Scopes
{

class LIBSIGC_API Uncounted:public Scope
  {
     Uncounted& operator=(const Uncounted&);
     Uncounted(const Uncounted&);
   public:
     virtual void disconnect(bool level=0);
     Uncounted():Scope() {}
     virtual ~Uncounted();
  };

class LIBSIGC_API Extend:public Scope
  {
  public:
     Extend& operator=(const Extend&);
     Extend(const Extend&);
   protected:
     virtual void on_connect();
     virtual void erase();
   public:
     virtual void disconnect(bool level=0);
     void set(ObjectScoped* obj,void* cache,bool ptr);
     Extend():Scope() {}
     virtual ~Extend();
  };

class LIBSIGC_API Limit:public Scope
  {
     Limit& operator=(const Limit&);
     Limit(const Limit&);
   protected:
     virtual void on_connect();
     virtual void erase();
   public:
     virtual void disconnect(bool level=0);
     void set(ObjectScoped* obj,void* cache,bool ptr);
     Limit():Scope() {}
     virtual ~Limit();
  };

typedef Extend RefCount;
typedef Reference Lock;
};

/*************************************************************
***** Lists 
*************************************************************/
// Stub for building polylists


// Iterator skeleton
struct LIBSIGC_API ScopeIterator_
  {
   typedef ScopeNode NodeType;
   private:
     NodeType *node_;
   public:

   inline NodeType* node()             {return node_;}
   inline const NodeType* node() const {return node_;}

   inline NodeType& operator*()
     {return *node_;
     }
   inline const NodeType& operator*() const
     {return *node_;
     }
   
   inline bool operator==(const ScopeIterator_& i) const 
     {return node_==i.node_;
     }
   inline bool operator!=(const ScopeIterator_& i) const
     {return node_!=i.node_;
     }

   inline ScopeIterator_& operator++()
     {
      if (node_) 
        node_=(NodeType*)node_->next_;
      return *this;
     }

   ScopeIterator_ operator++(int)
     {
      ScopeIterator_ tmp=*this;
      ++*this;
      return tmp;
     }

   ScopeIterator_& operator= (const ScopeIterator_& i)
     {
      node_=i.node_;
      return *this;
     }

   ScopeIterator_(const ScopeIterator_ &n):node_(n.node_) {}
   ScopeIterator_(NodeType *n):node_(n) {}
   ScopeIterator_():node_(0) {}
  };

class LIBSIGC_API ScopeList
  {
   public:
   typedef ScopeNode NodeType; 
   typedef ScopeIterator_ Iterator;

   ScopeNode node_;   

   inline Iterator begin()             {return Iterator(node_.next_);}
   inline Iterator end()               {return Iterator(&node_);}

   // insert item directly on list
   Iterator insert_direct(Iterator pos,NodeType *n);

   Iterator erase(Iterator pos);
   void erase(Iterator start,Iterator stop)
     { while (start!=stop) start=erase(start); }
   void swap_elements(Iterator p1,Iterator p2);

   void clear()
     { erase(begin(),end()); }

   bool empty() const {return node_.next_==&node_;} 
   
   ScopeList():node_() {}
   ~ScopeList() { clear(); }

   private:
     ScopeList(const ScopeList&);
  };



#ifdef SIGC_CXX_NAMESPACES
} // namespace sigc
#endif

#endif
