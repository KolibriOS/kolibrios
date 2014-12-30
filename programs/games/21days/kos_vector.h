/******************************************************************
*   21 days: a game for programmers
*   Copyright (C) 2014 Maxim Grishin
*
*   This program is free software; you can redistribute it and/or
*   modify it under the terms of the GNU General Public License
*   as published by the Free Software Foundation; either version 2
*   of the License, or (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*   MA  02110-1301, USA.
*
*
*   This file contains a part of file "/include/vector" from menuetlibc
*   adapted to the developer's needs.
*   Changes:
*       1. "*__cdecl" replaced with "* __attribute__((cdecl))" in order
*       to compile a C++ project.
*       2. Added front() methods with the following code:
*           T& front() {return data[0];}
*       3. Code reformatted
*
********************************************************************/

#ifndef KOS_VECTOR_H
#define KOS_VECTOR_H

extern void * __attribute__((cdecl)) operator new(size_t);
inline void * __attribute__((cdecl)) operator new(size_t, void *_P) {
    return (_P);
    }

template<class T> class vector21 {
    unsigned length;
    unsigned allocated;
    T* data;
  public:
    typedef unsigned size_type;
    typedef T* iterator;
    vector21():length(0),allocated(0),data(NULL) {}
    ~vector21() {for (unsigned i=length;i--;)data[i].~T();free(data);}
    unsigned size() const {return length;}
    void clear() {length=0;}
    T& operator[](unsigned pos) {return data[pos];}
    T* begin() {return data;}
    T* end() {return data+length;}
    void push_back(const T& x) {
        if (length==allocated){
            allocated+=16;
            data=(T*)realloc(data,allocated*sizeof(T));
            }
        new (data+length++) T(x);
        }
    bool empty() const {return length==0;}
    void pop_back() {data[--length].~T();}
    T& back() {return data[length-1];}
    T& front() {return data[0];}
    iterator erase(iterator it) {
        T* a=it;
        while (++a != data+length) {
            a[-1] = *a;
            }
        length--;
        return it;
        }
    /*iterator*/T* insert(iterator where, const T& what = T()) {
        int z=where-data;
        if (length==allocated) {
            allocated+=16;
            data=(T*)realloc(data,allocated*sizeof(T));
            }
        T* a=data+length;
        T* b=data+z;
        length++;
        while (a != b) {
            *a = a[-1];
            --a;
            }
        *a = what;
        return /*iterator*/a;
        }
    };

#endif
