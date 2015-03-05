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
#ifndef SIGCXX_TYPE_H
#define SIGCXX_TYPE_H

#include <sigc++config.h>

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif

/* some classes for making parameter passing easier */

#ifdef SIGC_CXX_SPECIALIZE_REFERENCES
template <typename T>
struct Trait
  {
   typedef const T& ref;
   typedef T        type;
  };

template <typename T>
struct Trait<T&>
  {
   typedef T& ref;
   typedef T& type;
  };
#else
template <typename T>
struct Trait
  {
   typedef T ref;  // VC++ does not support reference typedef
   typedef T type;
  };
#endif

template <>
struct Trait<void>:public Trait<int>
  {};


#ifdef SIGC_CXX_NAMESPACES
}
#endif

#endif
