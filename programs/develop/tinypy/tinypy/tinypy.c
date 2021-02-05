/*
================================================================================

tinypy contains tinypy code licensed in a MIT format license.  It also
contains some goodies grabbed from Python, so that license is included
as well.

================================================================================

The tinypy License

Copyright (c) 2008 Phil Hassey

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

================================================================================

PYTHON SOFTWARE FOUNDATION LICENSE VERSION 2
--------------------------------------------

1. This LICENSE AGREEMENT is between the Python Software Foundation
("PSF"), and the Individual or Organization ("Licensee") accessing and
otherwise using this software ("Python") in source or binary form and
its associated documentation.

2. Subject to the terms and conditions of this License Agreement, PSF
hereby grants Licensee a nonexclusive, royalty-free, world-wide
license to reproduce, analyze, test, perform and/or display publicly,
prepare derivative works, distribute, and otherwise use Python
alone or in any derivative version, provided, however, that PSF's
License Agreement and PSF's notice of copyright, i.e., "Copyright (c)
2001, 2002, 2003, 2004, 2005, 2006, 2007 Python Software Foundation;
All Rights Reserved" are retained in Python alone or in any derivative
version prepared by Licensee.

3. In the event Licensee prepares a derivative work that is based on
or incorporates Python or any part thereof, and wants to make
the derivative work available to others as provided herein, then
Licensee hereby agrees to include in any such work a brief summary of
the changes made to Python.

4. PSF is making Python available to Licensee on an "AS IS"
basis.  PSF MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR
IMPLIED.  BY WAY OF EXAMPLE, BUT NOT LIMITATION, PSF MAKES NO AND
DISCLAIMS ANY REPRESENTATION OR WARRANTY OF MERCHANTABILITY OR FITNESS
FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF PYTHON WILL NOT
INFRINGE ANY THIRD PARTY RIGHTS.

5. PSF SHALL NOT BE LIABLE TO LICENSEE OR ANY OTHER USERS OF PYTHON
FOR ANY INCIDENTAL, SPECIAL, OR CONSEQUENTIAL DAMAGES OR LOSS AS
A RESULT OF MODIFYING, DISTRIBUTING, OR OTHERWISE USING PYTHON,
OR ANY DERIVATIVE THEREOF, EVEN IF ADVISED OF THE POSSIBILITY THEREOF.

6. This License Agreement will automatically terminate upon a material
breach of its terms and conditions.

7. Nothing in this License Agreement shall be deemed to create any
relationship of agency, partnership, or joint venture between PSF and
Licensee.  This License Agreement does not grant permission to use PSF
trademarks or trade name in a trademark sense to endorse or promote
products or services of Licensee, or any third party.

8. By copying, installing or otherwise using Python, Licensee
agrees to be bound by the terms and conditions of this License
Agreement.

================================================================================
*/

#ifndef TINYPY_H
#define TINYPY_H
/* File: General
 * Things defined in tp.h.
 */
#ifndef TP_H
#define TP_H

#include <setjmp.h>
#include <sys/stat.h>

#ifndef __USE_ISOC99
#define __USE_ISOC99
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>

#include "bc.c"
#include "../std_modules/ksys/start_app.c"

#ifdef __GNUC__
#define tp_inline __inline__
#endif

#ifdef _MSC_VER
#ifdef NDEBUG
#define tp_inline __inline
#else
/* don't inline in debug builds (for easier debugging) */
#define tp_inline
#endif
#endif

#ifndef tp_inline
#error "Unsuported compiler"
#endif

#define TP_CSTR(v) ((tp_str(tp,(v))).string.val)

/*  #define tp_malloc(x) calloc((x),1)
    #define tp_realloc(x,y) realloc(x,y)
    #define tp_free(x) free(x) */

/* #include <gc/gc.h>
   #define tp_malloc(x) GC_MALLOC(x)
   #define tp_realloc(x,y) GC_REALLOC(x,y)
   #define tp_free(x)*/

enum {
    TP_NONE,TP_NUMBER,TP_STRING,TP_DICT,
    TP_LIST,TP_FNC,TP_DATA,
};

typedef double tp_num;

typedef struct tp_number_ {
    int type;
    tp_num val;
} tp_number_;
typedef struct tp_string_ {
    int type;
    struct _tp_string *info;
    char const *val;
    int len;
} tp_string_;
typedef struct tp_list_ {
    int type;
    struct _tp_list *val;
} tp_list_;
typedef struct tp_dict_ {
    int type;
    struct _tp_dict *val;
    int dtype;
} tp_dict_;
typedef struct tp_fnc_ {
    int type;
    struct _tp_fnc *info;
    int ftype;
    void *cfnc;
} tp_fnc_;
typedef struct tp_data_ {
    int type;
    struct _tp_data *info;
    void *val;
    int magic;
} tp_data_;

/* Type: tp_obj
 * Tinypy's object representation.
 *
 * Every object in tinypy is of this type in the C API.
 *
 * Fields:
 * type - This determines what kind of objects it is. It is either TP_NONE, in
 *        which case this is the none type and no other fields can be accessed.
 *        Or it has one of the values listed below, and the corresponding
 *        fields can be accessed.
 * number - TP_NUMBER
 * number.val - A double value with the numeric value.
 * string - TP_STRING
 * string.val - A pointer to the string data.
 * string.len - Length in bytes of the string data.
 * dict - TP_DICT
 * list - TP_LIST
 * fnc - TP_FNC
 * data - TP_DATA
 * data.val - The user-provided data pointer.
 * data.magic - The user-provided magic number for identifying the data type.
 */
typedef union tp_obj {
    int type;
    tp_number_ number;
    struct { int type; int *data; } gci;
    tp_string_ string;
    tp_dict_ dict;
    tp_list_ list;
    tp_fnc_ fnc;
    tp_data_ data;
} tp_obj;

typedef struct _tp_string {
    int gci;
    int len;
    char s[1];
} _tp_string;
typedef struct _tp_list {
    int gci;
    tp_obj *items;
    int len;
    int alloc;
} _tp_list;
typedef struct tp_item {
    int used;
    int hash;
    tp_obj key;
    tp_obj val;
} tp_item;
typedef struct _tp_dict {
    int gci;
    tp_item *items;
    int len;
    int alloc;
    int cur;
    int mask;
    int used;
    tp_obj meta;
} _tp_dict;
typedef struct _tp_fnc {
    int gci;
    tp_obj self;
    tp_obj globals;
    tp_obj code;
} _tp_fnc;


typedef union tp_code {
    unsigned char i;
    struct { unsigned char i,a,b,c; } regs;
    struct { char val[4]; } string;
    struct { float val; } number;
} tp_code;

typedef struct tp_frame_ {
/*    tp_code *codes; */
    tp_obj code;
    tp_code *cur;
    tp_code *jmp;
    tp_obj *regs;
    tp_obj *ret_dest;
    tp_obj fname;
    tp_obj name;
    tp_obj line;
    tp_obj globals;
    int lineno;
    int cregs;
} tp_frame_;

#define TP_GCMAX 4096
#define TP_FRAMES 256
#define TP_REGS_EXTRA 2
/* #define TP_REGS_PER_FRAME 256*/
#define TP_REGS 16384

/* Type: tp_vm
 * Representation of a tinypy virtual machine instance.
 *
 * A new tp_vm struct is created with <tp_init>, and will be passed to most
 * tinypy functions as first parameter. It contains all the data associated
 * with an instance of a tinypy virtual machine - so it is easy to have
 * multiple instances running at the same time. When you want to free up all
 * memory used by an instance, call <tp_deinit>.
 *
 * Fields:
 * These fields are currently documented:
 *
 * builtins - A dictionary containing all builtin objects.
 * modules - A dictionary with all loaded modules.
 * params - A list of parameters for the current function call.
 * frames - A list of all call frames.
 * cur - The index of the currently executing call frame.
 * frames[n].globals - A dictionary of global sybmols in callframe n.
 */
typedef struct tp_vm {
    tp_obj builtins;
    tp_obj modules;
    tp_frame_ frames[TP_FRAMES];
    tp_obj _params;
    tp_obj params;
    tp_obj _regs;
    tp_obj *regs;
    tp_obj root;
    jmp_buf buf;
#ifdef CPYTHON_MOD
    jmp_buf nextexpr;
#endif
    int jmp;
    tp_obj ex;
    char chars[256][2];
    int cur;
    /* gc */
    _tp_list *white;
    _tp_list *grey;
    _tp_list *black;
    int steps;
    /* sandbox */
    clock_t clocks;
    double time_elapsed;
    double time_limit;
    unsigned long mem_limit;
    unsigned long mem_used;
    int mem_exceeded;
} tp_vm;

#define TP tp_vm *tp
typedef struct _tp_data {
    int gci;
    void (*free)(TP,tp_obj);
} _tp_data;

#define tp_True tp_number(1)
#define tp_False tp_number(0)

extern tp_obj tp_None;

#ifdef TP_SANDBOX
void *tp_malloc(TP, unsigned long);
void *tp_realloc(TP, void *, unsigned long);
void tp_free(TP, void *);
#else
#define tp_malloc(TP,x) calloc((x),1)
#define tp_realloc(TP,x,y) realloc(x,y)
#define tp_free(TP,x) free(x)
#endif

void tp_sandbox(TP, double, unsigned long);
void tp_time_update(TP);
void tp_mem_update(TP);

void tp_run(TP,int cur);
void tp_set(TP,tp_obj,tp_obj,tp_obj);
tp_obj tp_get(TP,tp_obj,tp_obj);
tp_obj tp_has(TP,tp_obj self, tp_obj k);
tp_obj tp_len(TP,tp_obj);
void tp_del(TP,tp_obj,tp_obj);
tp_obj tp_str(TP,tp_obj);
int tp_bool(TP,tp_obj);
int tp_cmp(TP,tp_obj,tp_obj);
void _tp_raise(TP,tp_obj);
tp_obj tp_printf(TP,char const *fmt,...);
tp_obj tp_track(TP,tp_obj);
void tp_grey(TP,tp_obj);
tp_obj tp_call(TP, tp_obj fnc, tp_obj params);
tp_obj tp_add(TP,tp_obj a, tp_obj b) ;

/* __func__ __VA_ARGS__ __FILE__ __LINE__ */

/* Function: tp_raise
 * Macro to raise an exception.
 *
 * This macro will return from the current function returning "r". The
 * remaining parameters are used to format the exception message.
 */
/*
#define tp_raise(r,fmt,...) { \
    _tp_raise(tp,tp_printf(tp,fmt,__VA_ARGS__)); \
    return r; \
}
*/
#define tp_raise(r,v) { \
    _tp_raise(tp,v); \
    return r; \
}

/* Function: tp_string
 * Creates a new string object from a C string.
 *
 * Given a pointer to a C string, creates a tinypy object representing the
 * same string.
 *
 * *Note* Only a reference to the string will be kept by tinypy, so make sure
 * it does not go out of scope, and don't de-allocate it. Also be aware that
 * tinypy will not delete the string for you. In many cases, it is best to
 * use <tp_string_t> or <tp_string_slice> to create a string where tinypy
 * manages storage for you.
 */
tp_inline static tp_obj tp_string(char const *v) {
    tp_obj val;
    tp_string_ s = {TP_STRING, 0, v, 0};
    s.len = strlen(v);
    val.string = s;
    return val;
}

#define TP_CSTR_LEN 256

tp_inline static void tp_cstr(TP,tp_obj v, char *s, int l) {
    if (v.type != TP_STRING) {
        tp_raise(,tp_string("(tp_cstr) TypeError: value not a string"));
    }
    if (v.string.len >= l) {
        tp_raise(,tp_string("(tp_cstr) TypeError: value too long"));
    }
    memset(s,0,l);
    memcpy(s,v.string.val,v.string.len);
}


#define TP_OBJ() (tp_get(tp,tp->params,tp_None))
tp_inline static tp_obj tp_type(TP,int t,tp_obj v) {
    if (v.type != t) { tp_raise(tp_None,tp_string("(tp_type) TypeError: unexpected type")); }
    return v;
}



#define TP_NO_LIMIT 0
#define TP_TYPE(t) tp_type(tp,t,TP_OBJ())
#define TP_NUM() (TP_TYPE(TP_NUMBER).number.val)
/* #define TP_STR() (TP_CSTR(TP_TYPE(TP_STRING))) */
#define TP_STR() (TP_TYPE(TP_STRING))
#define TP_DEFAULT(d) (tp->params.list.val->len?tp_get(tp,tp->params,tp_None):(d))

/* Macro: TP_LOOP
 * Macro to iterate over all remaining arguments.
 *
 * If you have a function which takes a variable number of arguments, you can
 * iterate through all remaining arguments for example like this:
 *
 * > tp_obj *my_func(tp_vm *tp)
 * > {
 * >     // We retrieve the first argument like normal.
 * >     tp_obj first = TP_OBJ();
 * >     // Then we iterate over the remaining arguments.
 * >     tp_obj arg;
 * >     TP_LOOP(arg)
 * >         // do something with arg
 * >     TP_END
 * > }
 */
#define TP_LOOP(e) \
    int __l = tp->params.list.val->len; \
    int __i; for (__i=0; __i<__l; __i++) { \
    (e) = _tp_list_get(tp,tp->params.list.val,__i,"TP_LOOP");
#define TP_END \
    }

tp_inline static int _tp_min(int a, int b) { return (a<b?a:b); }
tp_inline static int _tp_max(int a, int b) { return (a>b?a:b); }
tp_inline static int _tp_sign(tp_num v) { return (v<0?-1:(v>0?1:0)); }

/* Function: tp_number
 * Creates a new numeric object.
 */
tp_inline static tp_obj tp_number(tp_num v) {
    tp_obj val = {TP_NUMBER};
    val.number.val = v;
    return val;
}

tp_inline static void tp_echo(TP,tp_obj e) {
    e = tp_str(tp,e);
    fwrite(e.string.val,1,e.string.len,stdout);
}

/* Function: tp_string_n
 * Creates a new string object from a partial C string.
 *
 * Like <tp_string>, but you specify how many bytes of the given C string to
 * use for the string object. The *note* also applies for this function, as the
 * string reference and length are kept, but no actual substring is stored.
 */
tp_inline static tp_obj tp_string_n(char const *v,int n) {
    tp_obj val;
    tp_string_ s = {TP_STRING, 0,v,n};
    val.string = s;
    return val;
}

#endif
void _tp_list_realloc(TP, _tp_list *self,int len) ;
void _tp_list_set(TP,_tp_list *self,int k, tp_obj v, const char *error) ;
void _tp_list_free(TP, _tp_list *self) ;
tp_obj _tp_list_get(TP,_tp_list *self,int k,const char *error) ;
void _tp_list_insertx(TP,_tp_list *self, int n, tp_obj v) ;
void _tp_list_appendx(TP,_tp_list *self, tp_obj v) ;
void _tp_list_insert(TP,_tp_list *self, int n, tp_obj v) ;
void _tp_list_append(TP,_tp_list *self, tp_obj v) ;
tp_obj _tp_list_pop(TP,_tp_list *self, int n, const char *error) ;
int _tp_list_find(TP,_tp_list *self, tp_obj v) ;
tp_obj tp_index(TP) ;
_tp_list *_tp_list_new(TP) ;
tp_obj _tp_list_copy(TP, tp_obj rr) ;
tp_obj tp_append(TP) ;
tp_obj tp_pop(TP) ;
tp_obj tp_insert(TP) ;
tp_obj tp_extend(TP) ;
tp_obj tp_list_nt(TP) ;
tp_obj tp_list(TP) ;
tp_obj tp_list_n(TP,int n,tp_obj *argv) ;
int _tp_sort_cmp(tp_obj *a,tp_obj *b) ;
tp_obj tp_sort(TP) ;
int tp_lua_hash(void const *v,int l) ;
void _tp_dict_free(TP, _tp_dict *self) ;
int tp_hash(TP,tp_obj v) ;
void _tp_dict_hash_set(TP,_tp_dict *self, int hash, tp_obj k, tp_obj v) ;
void _tp_dict_tp_realloc(TP,_tp_dict *self,int len) ;
int _tp_dict_hash_find(TP,_tp_dict *self, int hash, tp_obj k) ;
int _tp_dict_find(TP,_tp_dict *self,tp_obj k) ;
void _tp_dict_setx(TP,_tp_dict *self,tp_obj k, tp_obj v) ;
void _tp_dict_set(TP,_tp_dict *self,tp_obj k, tp_obj v) ;
tp_obj _tp_dict_get(TP,_tp_dict *self,tp_obj k, const char *error) ;
void _tp_dict_del(TP,_tp_dict *self,tp_obj k, const char *error) ;
_tp_dict *_tp_dict_new(TP) ;
tp_obj _tp_dict_copy(TP,tp_obj rr) ;
int _tp_dict_next(TP,_tp_dict *self) ;
tp_obj tp_merge(TP) ;
tp_obj tp_dict(TP) ;
tp_obj tp_dict_n(TP,int n, tp_obj* argv) ;
tp_obj _tp_dcall(TP,tp_obj fnc(TP)) ;
tp_obj _tp_tcall(TP,tp_obj fnc) ;
tp_obj tp_fnc_new(TP,int t, void *v, tp_obj c,tp_obj s, tp_obj g) ;
tp_obj tp_def(TP,tp_obj code, tp_obj g) ;
tp_obj tp_fnc(TP,tp_obj v(TP)) ;
tp_obj tp_method(TP,tp_obj self,tp_obj v(TP)) ;
tp_obj tp_data(TP,int magic,void *v) ;
tp_obj tp_params(TP) ;
tp_obj tp_params_n(TP,int n, tp_obj argv[]) ;
tp_obj tp_params_v(TP,int n,...) ;
tp_obj tp_string_t(TP, int n) ;
tp_obj tp_string_copy(TP, const char *s, int n) ;
tp_obj tp_string_sub(TP, tp_obj s, int a, int b) ;
int _tp_str_index(tp_obj s, tp_obj k) ;
tp_obj tp_join(TP) ;
tp_obj tp_split(TP) ;
tp_obj tp_find(TP) ;
tp_obj tp_str_index(TP) ;
tp_obj tp_str2(TP) ;
tp_obj tp_chr(TP) ;
tp_obj tp_ord(TP) ;
tp_obj tp_strip(TP) ;
tp_obj tp_replace(TP) ;
tp_obj tp_print(TP) ;
tp_obj tp_bind(TP) ;
tp_obj tp_min(TP) ;
tp_obj tp_max(TP) ;
tp_obj tp_copy(TP) ;
tp_obj tp_len_(TP) ;
tp_obj tp_assert(TP) ;
tp_obj tp_range(TP) ;
tp_obj tp_system(TP) ;
tp_obj tp_istype(TP) ;
tp_obj tp_float(TP) ;
tp_obj tp_save(TP) ;
tp_obj tp_load(TP) ;
tp_obj tp_fpack(TP) ;
tp_obj tp_abs(TP) ;
tp_obj tp_int(TP) ;
tp_num _roundf(tp_num v) ;
tp_obj tp_round(TP) ;
tp_obj tp_exists(TP) ;
tp_obj tp_mtime(TP) ;
int _tp_lookup_(TP,tp_obj self, tp_obj k, tp_obj *meta, int depth) ;
int _tp_lookup(TP,tp_obj self, tp_obj k, tp_obj *meta) ;
tp_obj tp_setmeta(TP) ;
tp_obj tp_getmeta(TP) ;
tp_obj tp_object(TP) ;
tp_obj tp_object_new(TP) ;
tp_obj tp_object_call(TP) ;
tp_obj tp_getraw(TP) ;
tp_obj tp_class(TP) ;
tp_obj tp_builtins_bool(TP) ;
void tp_follow(TP,tp_obj v) ;
void tp_reset(TP) ;
void tp_gc_init(TP) ;
void tp_gc_deinit(TP) ;
void tp_delete(TP,tp_obj v) ;
void tp_collect(TP) ;
void _tp_gcinc(TP) ;
void tp_full(TP) ;
void tp_gcinc(TP) ;
tp_obj tp_iter(TP,tp_obj self, tp_obj k) ;
int tp_iget(TP,tp_obj *r, tp_obj self, tp_obj k) ;
tp_obj tp_mul(TP,tp_obj a, tp_obj b) ;
tp_obj tp_bitwise_not(TP, tp_obj a) ;
tp_vm *_tp_init(void) ;
void tp_deinit(TP) ;
void tp_frame(TP,tp_obj globals,tp_obj code,tp_obj *ret_dest) ;
void tp_print_stack(TP) ;
void tp_handle(TP) ;
void tp_return(TP, tp_obj v) ;
int tp_step(TP) ;
void _tp_run(TP,int cur) ;
tp_obj tp_ez_call(TP, const char *mod, const char *fnc, tp_obj params) ;
tp_obj _tp_import(TP, tp_obj fname, tp_obj name, tp_obj code) ;
tp_obj tp_import(TP, const char * fname, const char * name, void *codes, int len) ;
tp_obj tp_exec_(TP) ;
tp_obj tp_import_(TP) ;
void tp_builtins(TP) ;
void tp_args(TP,int argc, char *argv[]) ;
tp_obj tp_main(TP,char *fname, void *code, int len) ;
tp_obj tp_compile(TP, tp_obj text, tp_obj fname) ;
tp_obj tp_exec(TP, tp_obj code, tp_obj globals) ;
tp_obj tp_eval(TP, const char *text, tp_obj globals) ;
tp_vm *tp_init(int argc, char *argv[]) ;
void tp_compiler(TP) ;
tp_obj tp_sandbox_(TP) ;
void tp_bounds(TP, tp_code *cur, int n) ;
#endif

void _tp_list_realloc(TP, _tp_list *self,int len) {
    if (!len) { len=1; }
    self->items = (tp_obj*)tp_realloc(tp, self->items,len*sizeof(tp_obj));
    self->alloc = len;
}

void _tp_list_set(TP,_tp_list *self,int k, tp_obj v, const char *error) {
    if (k >= self->len) {
        tp_raise(,tp_string("(_tp_list_set) KeyError"));
    }
    self->items[k] = v;
    tp_grey(tp,v);
}
void _tp_list_free(TP, _tp_list *self) {
    tp_free(tp, self->items);
    tp_free(tp, self);
}

tp_obj _tp_list_get(TP,_tp_list *self,int k,const char *error) {
    if (k >= self->len) {
        tp_raise(tp_None,tp_string("(_tp_list_set) KeyError"));
    }
    return self->items[k];
}
void _tp_list_insertx(TP,_tp_list *self, int n, tp_obj v) {
    if (self->len >= self->alloc) {
        _tp_list_realloc(tp, self,self->alloc*2);
    }
    if (n < self->len) { memmove(&self->items[n+1],&self->items[n],sizeof(tp_obj)*(self->len-n)); }
    self->items[n] = v;
    self->len += 1;
}
void _tp_list_appendx(TP,_tp_list *self, tp_obj v) {
    _tp_list_insertx(tp,self,self->len,v);
}
void _tp_list_insert(TP,_tp_list *self, int n, tp_obj v) {
    _tp_list_insertx(tp,self,n,v);
    tp_grey(tp,v);
}
void _tp_list_append(TP,_tp_list *self, tp_obj v) {
    _tp_list_insert(tp,self,self->len,v);
}
tp_obj _tp_list_pop(TP,_tp_list *self, int n, const char *error) {
    tp_obj r = _tp_list_get(tp,self,n,error);
    if (n != self->len-1) { memmove(&self->items[n],&self->items[n+1],sizeof(tp_obj)*(self->len-(n+1))); }
    self->len -= 1;
    return r;
}

int _tp_list_find(TP,_tp_list *self, tp_obj v) {
    int n;
    for (n=0; n<self->len; n++) {
        if (tp_cmp(tp,v,self->items[n]) == 0) {
            return n;
        }
    }
    return -1;
}

tp_obj tp_index(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i = _tp_list_find(tp,self.list.val,v);
    if (i < 0) {
        tp_raise(tp_None,tp_string("(tp_index) ValueError: list.index(x): x not in list"));
    }
    return tp_number(i);
}

_tp_list *_tp_list_new(TP) {
    return (_tp_list*)tp_malloc(tp, sizeof(_tp_list));
}

tp_obj _tp_list_copy(TP, tp_obj rr) {
    tp_obj val = {TP_LIST};
    _tp_list *o = rr.list.val;
    _tp_list *r = _tp_list_new(tp);
    *r = *o; r->gci = 0;
    r->items = (tp_obj*)tp_malloc(tp, sizeof(tp_obj)*o->len);
    memcpy(r->items,o->items,sizeof(tp_obj)*o->len);
    val.list.val = r;
    return tp_track(tp,val);
}

tp_obj tp_append(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    _tp_list_append(tp,self.list.val,v);
    return tp_None;
}

tp_obj tp_pop(TP) {
    tp_obj self = TP_OBJ();
    return _tp_list_pop(tp,self.list.val,self.list.val->len-1,"pop");
}

tp_obj tp_insert(TP) {
    tp_obj self = TP_OBJ();
    int n = TP_NUM();
    tp_obj v = TP_OBJ();
    _tp_list_insert(tp,self.list.val,n,v);
    return tp_None;
}

tp_obj tp_extend(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i;
    for (i=0; i<v.list.val->len; i++) {
        _tp_list_append(tp,self.list.val,v.list.val->items[i]);
    }
    return tp_None;
}

tp_obj tp_list_nt(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = _tp_list_new(tp);
    return r;
}

tp_obj tp_list(TP) {
    tp_obj r = {TP_LIST};
    r.list.val = _tp_list_new(tp);
    return tp_track(tp,r);
}

tp_obj tp_list_n(TP,int n,tp_obj *argv) {
    int i;
    tp_obj r = tp_list(tp); _tp_list_realloc(tp, r.list.val,n);
    for (i=0; i<n; i++) {
        _tp_list_append(tp,r.list.val,argv[i]);
    }
    return r;
}

int _tp_sort_cmp(tp_obj *a,tp_obj *b) {
    return tp_cmp(0,*a,*b);
}

tp_obj tp_sort(TP) {
    tp_obj self = TP_OBJ();
    qsort(self.list.val->items, self.list.val->len, sizeof(tp_obj), (int(*)(const void*,const void*))_tp_sort_cmp);
    return tp_None;
}

/* File: Dict
 * Functions for dealing with dictionaries.
 */
int tp_lua_hash(void const *v,int l) {
    int i,step = (l>>5)+1;
    int h = l + (l >= 4?*(int*)v:0);
    for (i=l; i>=step; i-=step) {
        h = h^((h<<5)+(h>>2)+((unsigned char *)v)[i-1]);
    }
    return h;
}
void _tp_dict_free(TP, _tp_dict *self) {
    tp_free(tp, self->items);
    tp_free(tp, self);
}

/* void _tp_dict_reset(_tp_dict *self) {
       memset(self->items,0,self->alloc*sizeof(tp_item));
       self->len = 0;
       self->used = 0;
       self->cur = 0;
   }*/

int tp_hash(TP,tp_obj v) {
    switch (v.type) {
        case TP_NONE: return 0;
        case TP_NUMBER: return tp_lua_hash(&v.number.val,sizeof(tp_num));
        case TP_STRING: return tp_lua_hash(v.string.val,v.string.len);
        case TP_DICT: return tp_lua_hash(&v.dict.val,sizeof(void*));
        case TP_LIST: {
            int r = v.list.val->len; int n; for(n=0; n<v.list.val->len; n++) {
            tp_obj vv = v.list.val->items[n]; r += vv.type != TP_LIST?tp_hash(tp,v.list.val->items[n]):tp_lua_hash(&vv.list.val,sizeof(void*)); } return r;
        }
        case TP_FNC: return tp_lua_hash(&v.fnc.info,sizeof(void*));
        case TP_DATA: return tp_lua_hash(&v.data.val,sizeof(void*));
    }
    tp_raise(0,tp_string("(tp_hash) TypeError: value unhashable"));
}

void _tp_dict_hash_set(TP,_tp_dict *self, int hash, tp_obj k, tp_obj v) {
    tp_item item;
    int i,idx = hash&self->mask;
    for (i=idx; i<idx+self->alloc; i++) {
        int n = i&self->mask;
        if (self->items[n].used > 0) { continue; }
        if (self->items[n].used == 0) { self->used += 1; }
        item.used = 1;
        item.hash = hash;
        item.key = k;
        item.val = v;
        self->items[n] = item;
        self->len += 1;
        return;
    }
    tp_raise(,tp_string("(_tp_dict_hash_set) RuntimeError: ?"));
}

void _tp_dict_tp_realloc(TP,_tp_dict *self,int len) {
    tp_item *items = self->items;
    int i,alloc = self->alloc;
    len = _tp_max(8,len);

    self->items = (tp_item*)tp_malloc(tp, len*sizeof(tp_item));
    self->alloc = len; self->mask = len-1;
    self->len = 0; self->used = 0;

    for (i=0; i<alloc; i++) {
        if (items[i].used != 1) { continue; }
        _tp_dict_hash_set(tp,self,items[i].hash,items[i].key,items[i].val);
    }
    tp_free(tp, items);
}

int _tp_dict_hash_find(TP,_tp_dict *self, int hash, tp_obj k) {
    int i,idx = hash&self->mask;
    for (i=idx; i<idx+self->alloc; i++) {
        int n = i&self->mask;
        if (self->items[n].used == 0) { break; }
        if (self->items[n].used < 0) { continue; }
        if (self->items[n].hash != hash) { continue; }
        if (tp_cmp(tp,self->items[n].key,k) != 0) { continue; }
        return n;
    }
    return -1;
}
int _tp_dict_find(TP,_tp_dict *self,tp_obj k) {
    return _tp_dict_hash_find(tp,self,tp_hash(tp,k),k);
}

void _tp_dict_setx(TP,_tp_dict *self,tp_obj k, tp_obj v) {
    int hash = tp_hash(tp,k); int n = _tp_dict_hash_find(tp,self,hash,k);
    if (n == -1) {
        if (self->len >= (self->alloc/2)) {
            _tp_dict_tp_realloc(tp,self,self->alloc*2);
        } else if (self->used >= (self->alloc*3/4)) {
            _tp_dict_tp_realloc(tp,self,self->alloc);
        }
        _tp_dict_hash_set(tp,self,hash,k,v);
    } else {
        self->items[n].val = v;
    }
}

void _tp_dict_set(TP,_tp_dict *self,tp_obj k, tp_obj v) {
    _tp_dict_setx(tp,self,k,v);
    tp_grey(tp,k); tp_grey(tp,v);
}

tp_obj _tp_dict_get(TP,_tp_dict *self,tp_obj k, const char *error) {
    int n = _tp_dict_find(tp,self,k);
    if (n < 0) {
        tp_raise(tp_None,tp_add(tp,tp_string("(_tp_dict_get) KeyError: "),tp_str(tp,k)));
    }
    return self->items[n].val;
}

void _tp_dict_del(TP,_tp_dict *self,tp_obj k, const char *error) {
    int n = _tp_dict_find(tp,self,k);
    if (n < 0) {
        tp_raise(,tp_add(tp,tp_string("(_tp_dict_del) KeyError: "),tp_str(tp,k)));
    }
    self->items[n].used = -1;
    self->len -= 1;
}

_tp_dict *_tp_dict_new(TP) {
    _tp_dict *self = (_tp_dict*)tp_malloc(tp, sizeof(_tp_dict));
    return self;
}
tp_obj _tp_dict_copy(TP,tp_obj rr) {
    tp_obj obj = {TP_DICT};
    _tp_dict *o = rr.dict.val;
    _tp_dict *r = _tp_dict_new(tp);
    *r = *o; r->gci = 0;
    r->items = (tp_item*)tp_malloc(tp, sizeof(tp_item)*o->alloc);
    memcpy(r->items,o->items,sizeof(tp_item)*o->alloc);
    obj.dict.val = r;
    obj.dict.dtype = 1;
    return tp_track(tp,obj);
}

int _tp_dict_next(TP,_tp_dict *self) {
    if (!self->len) {
        tp_raise(0,tp_string("(_tp_dict_next) RuntimeError"));
    }
    while (1) {
        self->cur = ((self->cur + 1) & self->mask);
        if (self->items[self->cur].used > 0) {
            return self->cur;
        }
    }
}

tp_obj tp_merge(TP) {
    tp_obj self = TP_OBJ();
    tp_obj v = TP_OBJ();
    int i; for (i=0; i<v.dict.val->len; i++) {
        int n = _tp_dict_next(tp,v.dict.val);
        _tp_dict_set(tp,self.dict.val,
            v.dict.val->items[n].key,v.dict.val->items[n].val);
    }
    return tp_None;
}

/* Function: tp_dict
 *
 * Creates a new dictionary object.
 *
 * *Note* If you use <tp_setmeta> on the dictionary, you have to use <tp_getraw> to
 * access the "raw" dictionary again.
 *
 * Returns:
 * The newly created dictionary.
 */
tp_obj tp_dict(TP) {
    tp_obj r = {TP_DICT};
    r.dict.val = _tp_dict_new(tp);
    r.dict.dtype = 1;
    return tp ? tp_track(tp,r) : r;
}

tp_obj tp_dict_n(TP,int n, tp_obj* argv) {
    tp_obj r = tp_dict(tp);
    int i; for (i=0; i<n; i++) { tp_set(tp,r,argv[i*2],argv[i*2+1]); }
    return r;
}


/* File: Miscellaneous
 * Various functions to help interface tinypy.
 */

tp_obj _tp_dcall(TP,tp_obj fnc(TP)) {
    return fnc(tp);
}
tp_obj _tp_tcall(TP,tp_obj fnc) {
    if (fnc.fnc.ftype&2) {
        _tp_list_insert(tp,tp->params.list.val,0,fnc.fnc.info->self);
    }
    return _tp_dcall(tp,(tp_obj (*)(tp_vm *))fnc.fnc.cfnc);
}

tp_obj tp_fnc_new(TP,int t, void *v, tp_obj c,tp_obj s, tp_obj g) {
    tp_obj r = {TP_FNC};
    _tp_fnc *info = (_tp_fnc*)tp_malloc(tp, sizeof(_tp_fnc));
    info->code = c;
    info->self = s;
    info->globals = g;
    r.fnc.ftype = t;
    r.fnc.info = info;
    r.fnc.cfnc = v;
    return tp_track(tp,r);
}

tp_obj tp_def(TP,tp_obj code, tp_obj g) {
    tp_obj r = tp_fnc_new(tp,1,0,code,tp_None,g);
    return r;
}

/* Function: tp_fnc
 * Creates a new tinypy function object.
 *
 * This is how you can create a tinypy function object which, when called in
 * the script, calls the provided C function.
 */
tp_obj tp_fnc(TP,tp_obj v(TP)) {
    return tp_fnc_new(tp,0,v,tp_None,tp_None,tp_None);
}

tp_obj tp_method(TP,tp_obj self,tp_obj v(TP)) {
    return tp_fnc_new(tp,2,v,tp_None,self,tp_None);
}

/* Function: tp_data
 * Creates a new data object.
 *
 * Parameters:
 * magic - An integer number associated with the data type. This can be used
 *         to check the type of data objects.
 * v     - A pointer to user data. Only the pointer is stored in the object,
 *         you keep all responsibility for the data it points to.
 *
 *
 * Returns:
 * The new data object.
 *
 * Public fields:
 * The following fields can be access in a data object:
 *
 * magic      - An integer number stored in the object.
 * val        - The data pointer of the object.
 * info->free - If not NULL, a callback function called when the object gets
 *              destroyed.
 *
 * Example:
 * > void *__free__(TP, tp_obj self)
 * > {
 * >     free(self.data.val);
 * > }
 * >
 * > tp_obj my_obj = tp_data(TP, 0, my_ptr);
 * > my_obj.data.info->free = __free__;
 */
tp_obj tp_data(TP,int magic,void *v) {
    tp_obj r = {TP_DATA};
    r.data.info = (_tp_data*)tp_malloc(tp, sizeof(_tp_data));
    r.data.val = v;
    r.data.magic = magic;
    return tp_track(tp,r);
}

/* Function: tp_params
 * Initialize the tinypy parameters.
 *
 * When you are calling a tinypy function, you can use this to initialize the
 * list of parameters getting passed to it. Usually, you may want to use
 * <tp_params_n> or <tp_params_v>.
 */
tp_obj tp_params(TP) {
    tp_obj r;
    tp->params = tp->_params.list.val->items[tp->cur];
    r = tp->_params.list.val->items[tp->cur];
    r.list.val->len = 0;
    return r;
}

/* Function: tp_params_n
 * Specify a list of objects as function call parameters.
 *
 * See also: <tp_params>, <tp_params_v>
 *
 * Parameters:
 * n - The number of parameters.
 * argv - A list of n tinypy objects, which will be passed as parameters.
 *
 * Returns:
 * The parameters list. You may modify it before performing the function call.
 */
tp_obj tp_params_n(TP,int n, tp_obj argv[]) {
    tp_obj r = tp_params(tp);
    int i; for (i=0; i<n; i++) { _tp_list_append(tp,r.list.val,argv[i]); }
    return r;
}

/* Function: tp_params_v
 * Pass parameters for a tinypy function call.
 *
 * When you want to call a tinypy method, then you use this to pass parameters
 * to it.
 *
 * Parameters:
 * n   - The number of variable arguments following.
 * ... - Pass n tinypy objects, which a subsequently called tinypy method will
 *       receive as parameters.
 *
 * Returns:
 * A tinypy list object representing the current call parameters. You can modify
 * the list before doing the function call.
 */
tp_obj tp_params_v(TP,int n,...) {
    int i;
    tp_obj r = tp_params(tp);
    va_list a; va_start(a,n);
    for (i=0; i<n; i++) {
        _tp_list_append(tp,r.list.val,va_arg(a,tp_obj));
    }
    va_end(a);
    return r;
}
/* File: String
 * String handling functions.
 */

/*
 * Create a new empty string of a certain size.
 * Does not put it in for GC tracking, since contents should be
 * filled after returning.
 */
tp_obj tp_string_t(TP, int n) {
    tp_obj r = tp_string_n(0,n);
    r.string.info = (_tp_string*)tp_malloc(tp, sizeof(_tp_string)+n);
    r.string.info->len = n;
    r.string.val = r.string.info->s;
    return r;
}

/*
 * Create a new string which is a copy of some memory.
 * This is put into GC tracking for you.
 */
tp_obj tp_string_copy(TP, const char *s, int n) {
    tp_obj r = tp_string_t(tp,n);
    memcpy(r.string.info->s,s,n);
    return tp_track(tp,r);
}

/*
 * Create a new string which is a substring slice of another STRING.
 * Does not need to be put into GC tracking, as its parent is
 * already being tracked (supposedly).
 */
tp_obj tp_string_sub(TP, tp_obj s, int a, int b) {
    int l = s.string.len;
    a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
    tp_obj r = s;
    r.string.val += a;
    r.string.len = b-a;
    return r;
}

tp_obj tp_printf(TP, char const *fmt,...) {
    int l;
    tp_obj r;
    char *s;
    va_list arg;
    va_start(arg, fmt);
    l = vsnprintf(NULL, 0, fmt,arg);
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    va_end(arg);
    va_start(arg, fmt);
    vsprintf(s,fmt,arg);
    va_end(arg);
    return tp_track(tp,r);
}

int _tp_str_index(tp_obj s, tp_obj k) {
    int i=0;
    while ((s.string.len - i) >= k.string.len) {
        if (memcmp(s.string.val+i,k.string.val,k.string.len) == 0) {
            return i;
        }
        i += 1;
    }
    return -1;
}

tp_obj tp_join(TP) {
    tp_obj delim = TP_OBJ();
    tp_obj val = TP_OBJ();
    int l=0,i;
    tp_obj r;
    char *s;
    for (i=0; i<val.list.val->len; i++) {
        if (i!=0) { l += delim.string.len; }
        l += tp_str(tp,val.list.val->items[i]).string.len;
    }
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    l = 0;
    for (i=0; i<val.list.val->len; i++) {
        tp_obj e;
        if (i!=0) {
            memcpy(s+l,delim.string.val,delim.string.len); l += delim.string.len;
        }
        e = tp_str(tp,val.list.val->items[i]);
        memcpy(s+l,e.string.val,e.string.len); l += e.string.len;
    }
    return tp_track(tp,r);
}

tp_obj tp_split(TP) {
    tp_obj v = TP_OBJ();
    tp_obj d = TP_OBJ();
    tp_obj r = tp_list(tp);

    int i;
    while ((i=_tp_str_index(v,d))!=-1) {
        _tp_list_append(tp,r.list.val,tp_string_sub(tp,v,0,i));
        v.string.val += i + d.string.len; v.string.len -= i + d.string.len;
    }
    _tp_list_append(tp,r.list.val,tp_string_sub(tp,v,0,v.string.len));
    return r;
}


tp_obj tp_find(TP) {
    tp_obj s = TP_OBJ();
    tp_obj v = TP_OBJ();
    return tp_number(_tp_str_index(s,v));
}

tp_obj tp_str_index(TP) {
    tp_obj s = TP_OBJ();
    tp_obj v = TP_OBJ();
    int n = _tp_str_index(s,v);
    if (n >= 0) { return tp_number(n); }
    tp_raise(tp_None,tp_string("(tp_str_index) ValueError: substring not found"));
}

tp_obj tp_str2(TP) {
    tp_obj v = TP_OBJ();
    return tp_str(tp,v);
}

tp_obj tp_chr(TP) {
    int v = TP_NUM();
    return tp_string_n(tp->chars[(unsigned char)v],1);
}
tp_obj tp_ord(TP) {
    tp_obj s = TP_STR();
    if (s.string.len != 1) {
        tp_raise(tp_None,tp_string("(tp_ord) TypeError: ord() expected a character"));
    }
    return tp_number((unsigned char)s.string.val[0]);
}

tp_obj tp_strip(TP) {
    tp_obj o = TP_TYPE(TP_STRING);
    char const *v = o.string.val; int l = o.string.len;
    int i; int a = l, b = 0;
    tp_obj r;
    char *s;
    for (i=0; i<l; i++) {
        if (v[i] != ' ' && v[i] != '\n' && v[i] != '\t' && v[i] != '\r') {
            a = _tp_min(a,i); b = _tp_max(b,i+1);
        }
    }
    if ((b-a) < 0) { return tp_string(""); }
    r = tp_string_t(tp,b-a);
    s = r.string.info->s;
    memcpy(s,v+a,b-a);
    return tp_track(tp,r);
}

tp_obj tp_replace(TP) {
    tp_obj s = TP_OBJ();
    tp_obj k = TP_OBJ();
    tp_obj v = TP_OBJ();
    tp_obj p = s;
    int i,n = 0;
    int c;
    int l;
    tp_obj rr;
    char *r;
    char *d;
    tp_obj z;
    while ((i = _tp_str_index(p,k)) != -1) {
        n += 1;
        p.string.val += i + k.string.len; p.string.len -= i + k.string.len;
    }
/*     fprintf(stderr,"ns: %d\n",n); */
    l = s.string.len + n * (v.string.len-k.string.len);
    rr = tp_string_t(tp,l);
    r = rr.string.info->s;
    d = r;
    z = p = s;
    while ((i = _tp_str_index(p,k)) != -1) {
        p.string.val += i; p.string.len -= i;
        memcpy(d,z.string.val,c=(p.string.val-z.string.val)); d += c;
        p.string.val += k.string.len; p.string.len -= k.string.len;
        memcpy(d,v.string.val,v.string.len); d += v.string.len;
        z = p;
    }
    memcpy(d,z.string.val,(s.string.val + s.string.len) - z.string.val);

    return tp_track(tp,rr);
}

/* File: Builtins
 * Builtin tinypy functions.
 */
#ifdef CONIO
    #include "conio.c"
    tp_obj tp_print(TP) {
        if(!con_enabled){
            console_init();
        }
        int n = 0;
        tp_obj e;
        TP_LOOP(e)
        if (n) { con_printf(" "); }
        con_printf("%s",TP_CSTR(e));
        n += 1;
        TP_END;
        con_printf("\n");
        return tp_None;
    }
 
    #define BUF_SIZE 2048
    tp_obj tp_raw_input(TP) {
        console_init();
        tp_obj prompt;
        char *buf = malloc(BUF_SIZE);
        if (tp->params.list.val->len){
            prompt = TP_OBJ();
            con_printf("%s", TP_CSTR(prompt));
        }
        con_gets(buf, BUF_SIZE);
        return tp_string(buf);
}
#else
tp_obj tp_print(TP) {
    int n = 0;
    tp_obj e;
    TP_LOOP(e)
        if (n) { printf(" "); }
        tp_echo(tp,e);
        n += 1;
    TP_END;
    printf("\n");
    return tp_None;
}
#endif

tp_obj tp_bind(TP) {
    tp_obj r = TP_TYPE(TP_FNC);
    tp_obj self = TP_OBJ();
    return tp_fnc_new(tp,
        r.fnc.ftype|2,r.fnc.cfnc,r.fnc.info->code,
        self,r.fnc.info->globals);
}

tp_obj tp_min(TP) {
    tp_obj r = TP_OBJ();
    tp_obj e;
    TP_LOOP(e)
        if (tp_cmp(tp,r,e) > 0) { r = e; }
    TP_END;
    return r;
}

tp_obj tp_max(TP) {
    tp_obj r = TP_OBJ();
    tp_obj e;
    TP_LOOP(e)
        if (tp_cmp(tp,r,e) < 0) { r = e; }
    TP_END;
    return r;
}

tp_obj tp_copy(TP) {
    tp_obj r = TP_OBJ();
    int type = r.type;
    if (type == TP_LIST) {
        return _tp_list_copy(tp,r);
    } else if (type == TP_DICT) {
        return _tp_dict_copy(tp,r);
    }
    tp_raise(tp_None,tp_string("(tp_copy) TypeError: ?"));
}


tp_obj tp_len_(TP) {
    tp_obj e = TP_OBJ();
    return tp_len(tp,e);
}

tp_obj tp_assert(TP) {
    int a = TP_NUM();
    if (a) { return tp_None; }
    tp_raise(tp_None,tp_string("(tp_assert) AssertionError"));
}

tp_obj tp_range(TP) {
    int a,b,c,i;
    tp_obj r = tp_list(tp);
    switch (tp->params.list.val->len) {
        case 1: a = 0; b = TP_NUM(); c = 1; break;
        case 2:
        case 3: a = TP_NUM(); b = TP_NUM(); c = TP_DEFAULT(tp_number(1)).number.val; break;
        default: return r;
    }
    if (c != 0) {
        for (i=a; (c>0) ? i<b : i>b; i+=c) {
            _tp_list_append(tp,r.list.val,tp_number(i));
        }
    }
    return r;
}

/* Function: tp_system
 *
 * The system builtin. A grave security flaw. If your version of tinypy
 * enables this, you better remove it before deploying your app :P
 */
tp_obj tp_system(TP) {
    const char * s = TP_TYPE(TP_STRING).string.val;
    char *command=strtok((char*)s," ");
    char *argm=strtok(NULL, " ");
    int r =start_app(command, argm);
    return tp_number(r);
}

tp_obj tp_istype(TP) {
    tp_obj v = TP_OBJ();
    tp_obj t = TP_STR();
    if (tp_cmp(tp,t,tp_string("string")) == 0) { return tp_number(v.type == TP_STRING); }
    if (tp_cmp(tp,t,tp_string("list")) == 0) { return tp_number(v.type == TP_LIST); }
    if (tp_cmp(tp,t,tp_string("dict")) == 0) { return tp_number(v.type == TP_DICT); }
    if (tp_cmp(tp,t,tp_string("number")) == 0) { return tp_number(v.type == TP_NUMBER); }
    if (tp_cmp(tp,t,tp_string("fnc")) == 0) { return tp_number(v.type == TP_FNC && (v.fnc.ftype&2) == 0); }
    if (tp_cmp(tp,t,tp_string("method")) == 0) { return tp_number(v.type == TP_FNC && (v.fnc.ftype&2) != 0); }
    tp_raise(tp_None,tp_string("(is_type) TypeError: ?"));
}


tp_obj tp_float(TP) {
    tp_obj v = TP_OBJ();
    int ord = TP_DEFAULT(tp_number(0)).number.val;
    int type = v.type;
    if (type == TP_NUMBER) { return v; }
    if (type == TP_STRING && v.string.len < 32) {
        char s[32]; memset(s,0,v.string.len+1);
        memcpy(s,v.string.val,v.string.len);
        if (strchr(s,'.')) { return tp_number(atof(s)); }
        return(tp_number(strtoul(s,0,ord)));
    }
    tp_raise(tp_None,tp_string("(tp_float) TypeError: ?"));
}


tp_obj tp_save(TP) {
    char fname[256]; tp_cstr(tp,TP_STR(),fname,256);
    tp_obj v = TP_OBJ();
    FILE *f;
    f = fopen(fname,"wb");
    if (!f) { tp_raise(tp_None,tp_string("(tp_save) IOError: ?")); }
    fwrite(v.string.val,v.string.len,1,f);
    fclose(f);
    return tp_None;
}

tp_obj tp_load(TP) {
    FILE *f;
    long l;
    tp_obj r;
    char *s;
    char fname[256]; tp_cstr(tp,TP_STR(),fname,256);
    struct stat stbuf;
    stat(fname, &stbuf);
    l = stbuf.st_size;
    f = fopen(fname,"rb");
    if (!f) {
        tp_raise(tp_None,tp_string("(tp_load) IOError: ?"));
    }
    r = tp_string_t(tp,l);
    s = r.string.info->s;
    fread(s,1,l,f);
/*    if (rr !=l) { printf("hmmn: %d %d\n",rr,(int)l); }*/
    fclose(f);
    return tp_track(tp,r);
}


tp_obj tp_fpack(TP) {
    tp_num v = TP_NUM();
    tp_obj r = tp_string_t(tp,sizeof(tp_num));
    *(tp_num*)r.string.val = v;
    return tp_track(tp,r);
}

tp_obj tp_abs(TP) {
    return tp_number(fabs(tp_float(tp).number.val));
}
tp_obj tp_int(TP) {
    return tp_number((long)tp_float(tp).number.val);
}
tp_num _roundf(tp_num v) {
    tp_num av = fabs(v); tp_num iv = (long)av;
    av = (av-iv < 0.5?iv:iv+1);
    return (v<0?-av:av);
}
tp_obj tp_round(TP) {
    return tp_number(_roundf(tp_float(tp).number.val));
}

tp_obj tp_exists(TP) {
    char fname[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),fname,TP_CSTR_LEN);
    struct stat stbuf;
    return tp_number(!stat(fname,&stbuf));
}
tp_obj tp_mtime(TP) {
    char fname[TP_CSTR_LEN]; tp_cstr(tp,TP_STR(),fname,TP_CSTR_LEN);
    struct stat stbuf;
    if (!stat(fname,&stbuf)) { return tp_number(stbuf.st_mtime); }
    tp_raise(tp_None,tp_string("(tp_mtime) IOError: ?"));
}

int _tp_lookup_(TP,tp_obj self, tp_obj k, tp_obj *meta, int depth) {
    int n = _tp_dict_find(tp,self.dict.val,k);
    if (n != -1) {
        *meta = self.dict.val->items[n].val;
        return 1;
    }
    depth--; if (!depth) { tp_raise(0,tp_string("(tp_lookup) RuntimeError: maximum lookup depth exceeded")); }
    if (self.dict.dtype && self.dict.val->meta.type == TP_DICT && _tp_lookup_(tp,self.dict.val->meta,k,meta,depth)) {
        if (self.dict.dtype == 2 && meta->type == TP_FNC) {
            *meta = tp_fnc_new(tp,meta->fnc.ftype|2,
                meta->fnc.cfnc,meta->fnc.info->code,
                self,meta->fnc.info->globals);
        }
        return 1;
    }
    return 0;
}

int _tp_lookup(TP,tp_obj self, tp_obj k, tp_obj *meta) {
    return _tp_lookup_(tp,self,k,meta,8);
}

#define TP_META_BEGIN(self,name) \
    if (self.dict.dtype == 2) { \
        tp_obj meta; if (_tp_lookup(tp,self,tp_string(name),&meta)) {

#define TP_META_END \
        } \
    }

/* Function: tp_setmeta
 * Set a "dict's meta".
 *
 * This is a builtin function, so you need to use <tp_params> to provide the
 * parameters.
 *
 * In tinypy, each dictionary can have a so-called "meta" dictionary attached
 * to it. When dictionary attributes are accessed, but not present in the
 * dictionary, they instead are looked up in the meta dictionary. To get the
 * raw dictionary, you can use <tp_getraw>.
 *
 * This function is particulary useful for objects and classes, which are just
 * special dictionaries created with <tp_object> and <tp_class>. There you can
 * use tp_setmeta to change the class of the object or parent class of a class.
 *
 * Parameters:
 * self - The dictionary for which to set a meta.
 * meta - The meta dictionary.
 *
 * Returns:
 * None
 */
tp_obj tp_setmeta(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    tp_obj meta = TP_TYPE(TP_DICT);
    self.dict.val->meta = meta;
    return tp_None;
}

tp_obj tp_getmeta(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    return self.dict.val->meta;
}

/* Function: tp_object
 * Creates a new object.
 *
 * Returns:
 * The newly created object. The object initially has no parent class, use
 * <tp_setmeta> to set a class. Also see <tp_object_new>.
 */
tp_obj tp_object(TP) {
    tp_obj self = tp_dict(tp);
    self.dict.dtype = 2;
    return self;
}

tp_obj tp_object_new(TP) {
    tp_obj klass = TP_TYPE(TP_DICT);
    tp_obj self = tp_object(tp);
    self.dict.val->meta = klass;
    TP_META_BEGIN(self,"__init__");
        tp_call(tp,meta,tp->params);
    TP_META_END;
    return self;
}

tp_obj tp_object_call(TP) {
    tp_obj self;
    if (tp->params.list.val->len) {
        self = TP_TYPE(TP_DICT);
        self.dict.dtype = 2;
    } else {
        self = tp_object(tp);
    }
    return self;
}

/* Function: tp_getraw
 * Retrieve the raw dict of a dict.
 *
 * This builtin retrieves one dict parameter from tinypy, and returns its raw
 * dict. This is very useful when implementing your own __get__ and __set__
 * functions, as it allows you to directly access the attributes stored in the
 * dict.
 */
tp_obj tp_getraw(TP) {
    tp_obj self = TP_TYPE(TP_DICT);
    self.dict.dtype = 0;
    return self;
}

/* Function: tp_class
 * Creates a new base class.
 *
 * Parameters:
 * none
 *
 * Returns:
 * A new, empty class (derived from tinypy's builtin "object" class).
 */
tp_obj tp_class(TP) {
    tp_obj klass = tp_dict(tp);
    klass.dict.val->meta = tp_get(tp,tp->builtins,tp_string("object"));
    return klass;
}

/* Function: tp_builtins_bool
 * Coerces any value to a boolean.
 */
tp_obj tp_builtins_bool(TP) {
    tp_obj v = TP_OBJ();
    return (tp_number(tp_bool(tp, v)));
}
/* tp_obj tp_track(TP,tp_obj v) { return v; }
   void tp_grey(TP,tp_obj v) { }
   void tp_full(TP) { }
   void tp_gc_init(TP) { }
   void tp_gc_deinit(TP) { }
   void tp_delete(TP,tp_obj v) { }*/

void tp_grey(TP,tp_obj v) {
    if (v.type < TP_STRING || (!v.gci.data) || *v.gci.data) { return; }
    *v.gci.data = 1;
    if (v.type == TP_STRING || v.type == TP_DATA) {
        _tp_list_appendx(tp,tp->black,v);
        return;
    }
    _tp_list_appendx(tp,tp->grey,v);
}

void tp_follow(TP,tp_obj v) {
    int type = v.type;
    if (type == TP_LIST) {
        int n;
        for (n=0; n<v.list.val->len; n++) {
            tp_grey(tp,v.list.val->items[n]);
        }
    }
    if (type == TP_DICT) {
        int i;
        for (i=0; i<v.dict.val->len; i++) {
            int n = _tp_dict_next(tp,v.dict.val);
            tp_grey(tp,v.dict.val->items[n].key);
            tp_grey(tp,v.dict.val->items[n].val);
        }
        tp_grey(tp,v.dict.val->meta);
    }
    if (type == TP_FNC) {
        tp_grey(tp,v.fnc.info->self);
        tp_grey(tp,v.fnc.info->globals);
        tp_grey(tp,v.fnc.info->code);
    }
}

void tp_reset(TP) {
    int n;
    _tp_list *tmp;
    for (n=0; n<tp->black->len; n++) {
        *tp->black->items[n].gci.data = 0;
    }
    tmp = tp->white;
    tp->white = tp->black;
    tp->black = tmp;
}

void tp_gc_init(TP) {
    tp->white = _tp_list_new(tp);
    tp->grey = _tp_list_new(tp);
    tp->black = _tp_list_new(tp);
    tp->steps = 0;
}

void tp_gc_deinit(TP) {
    _tp_list_free(tp, tp->white);
    _tp_list_free(tp, tp->grey);
    _tp_list_free(tp, tp->black);
}

void tp_delete(TP,tp_obj v) {
    int type = v.type;
    if (type == TP_LIST) {
        _tp_list_free(tp, v.list.val);
        return;
    } else if (type == TP_DICT) {
        _tp_dict_free(tp, v.dict.val);
        return;
    } else if (type == TP_STRING) {
        tp_free(tp, v.string.info);
        return;
    } else if (type == TP_DATA) {
        if (v.data.info->free) {
            v.data.info->free(tp,v);
        }
        tp_free(tp, v.data.info);
        return;
    } else if (type == TP_FNC) {
        tp_free(tp, v.fnc.info);
        return;
    }
    tp_raise(,tp_string("(tp_delete) TypeError: ?"));
}

void tp_collect(TP) {
    int n;
    for (n=0; n<tp->white->len; n++) {
        tp_obj r = tp->white->items[n];
        if (*r.gci.data) { continue; }
        tp_delete(tp,r);
    }
    tp->white->len = 0;
    tp_reset(tp);
}

void _tp_gcinc(TP) {
    tp_obj v;
    if (!tp->grey->len) {
        return;
    }
    v = _tp_list_pop(tp,tp->grey,tp->grey->len-1,"_tp_gcinc");
    tp_follow(tp,v);
    _tp_list_appendx(tp,tp->black,v);
}

void tp_full(TP) {
    while (tp->grey->len) {
        _tp_gcinc(tp);
    }
    tp_collect(tp);
    tp_follow(tp,tp->root);
}

void tp_gcinc(TP) {
    tp->steps += 1;
    if (tp->steps < TP_GCMAX || tp->grey->len > 0) {
        _tp_gcinc(tp); _tp_gcinc(tp);
    }
    if (tp->steps < TP_GCMAX || tp->grey->len > 0) { return; }
    tp->steps = 0;
    tp_full(tp);
    return;
}

tp_obj tp_track(TP,tp_obj v) {
    tp_gcinc(tp);
    tp_grey(tp,v);
    return v;
}

/**/

/* File: Operations
 * Various tinypy operations.
 */

/* Function: tp_str
 * String representation of an object.
 *
 * Returns a string object representating self.
 */
tp_obj tp_str(TP,tp_obj self) {
    int type = self.type;
    if (type == TP_STRING) { return self; }
    if (type == TP_NUMBER) {
        tp_num v = self.number.val;
        if ((fabs(v)-fabs((long)v)) < 0.000001) { return tp_printf(tp,"%ld",(long)v); }
        return tp_printf(tp,"%f",v);
    } else if(type == TP_DICT) {
        return tp_printf(tp,"<dict 0x%x>",self.dict.val);
    } else if(type == TP_LIST) {
        return tp_printf(tp,"<list 0x%x>",self.list.val);
    } else if (type == TP_NONE) {
        return tp_string("None");
    } else if (type == TP_DATA) {
        return tp_printf(tp,"<data 0x%x>",self.data.val);
    } else if (type == TP_FNC) {
        return tp_printf(tp,"<fnc 0x%x>",self.fnc.info);
    }
    return tp_string("<?>");
}

/* Function: tp_bool
 * Check the truth value of an object
 *
 * Returns false if v is a numeric object with a value of exactly 0, v is of
 * type None or v is a string list or dictionary with a length of 0. Else true
 * is returned.
 */
int tp_bool(TP,tp_obj v) {
    switch(v.type) {
        case TP_NUMBER: return v.number.val != 0;
        case TP_NONE: return 0;
        case TP_STRING: return v.string.len != 0;
        case TP_LIST: return v.list.val->len != 0;
        case TP_DICT: return v.dict.val->len != 0;
    }
    return 1;
}


/* Function: tp_has
 * Checks if an object contains a key.
 *
 * Returns tp_True if self[k] exists, tp_False otherwise.
 */
tp_obj tp_has(TP,tp_obj self, tp_obj k) {
    int type = self.type;
    if (type == TP_DICT) {
        if (_tp_dict_find(tp,self.dict.val,k) != -1) { return tp_True; }
        return tp_False;
    } else if (type == TP_STRING && k.type == TP_STRING) {
        return tp_number(_tp_str_index(self,k)!=-1);
    } else if (type == TP_LIST) {
        return tp_number(_tp_list_find(tp,self.list.val,k)!=-1);
    }
    tp_raise(tp_None,tp_string("(tp_has) TypeError: iterable argument required"));
}

/* Function: tp_del
 * Remove a dictionary entry.
 *
 * Removes the key k from self. Also works on classes and objects.
 *
 * Note that unlike with Python, you cannot use this to remove list items.
 */
void tp_del(TP,tp_obj self, tp_obj k) {
    int type = self.type;
    if (type == TP_DICT) {
        _tp_dict_del(tp,self.dict.val,k,"tp_del");
        return;
    }
    tp_raise(,tp_string("(tp_del) TypeError: object does not support item deletion"));
}


/* Function: tp_iter
 * Iterate through a list or dict.
 *
 * If self is a list/string/dictionary, this will iterate over the
 * elements/characters/keys respectively, if k is an increasing index
 * starting with 0 up to the length of the object-1.
 *
 * In the case of a list of string, the returned items will correspond to the
 * item at index k. For a dictionary, no guarantees are made about the order.
 * You also cannot call the function with a specific k to get a specific
 * item -- it is only meant for iterating through all items, calling this
 * function len(self) times. Use <tp_get> to retrieve a specific item, and
 * <tp_len> to get the length.
 *
 * Parameters:
 * self - The object over which to iterate.
 * k - You must pass 0 on the first call, then increase it by 1 after each call,
 *     and don't call the function with k >= len(self).
 *
 * Returns:
 * The first (k = 0) or next (k = 1 .. len(self)-1) item in the iteration.
 */
tp_obj tp_iter(TP,tp_obj self, tp_obj k) {
    int type = self.type;
    if (type == TP_LIST || type == TP_STRING) { return tp_get(tp,self,k); }
    if (type == TP_DICT && k.type == TP_NUMBER) {
        return self.dict.val->items[_tp_dict_next(tp,self.dict.val)].key;
    }
    tp_raise(tp_None,tp_string("(tp_iter) TypeError: iteration over non-sequence"));
}


/* Function: tp_get
 * Attribute lookup.
 *
 * This returns the result of using self[k] in actual code. It works for
 * dictionaries (including classes and instantiated objects), lists and strings.
 *
 * As a special case, if self is a list, self[None] will return the first
 * element in the list and subsequently remove it from the list.
 */
tp_obj tp_get(TP,tp_obj self, tp_obj k) {
    int type = self.type;
    tp_obj r;
    if (type == TP_DICT) {
        TP_META_BEGIN(self,"__get__");
            return tp_call(tp,meta,tp_params_v(tp,1,k));
        TP_META_END;
        if (self.dict.dtype && _tp_lookup(tp,self,k,&r)) { return r; }
        return _tp_dict_get(tp,self.dict.val,k,"tp_get");
    } else if (type == TP_LIST) {
        if (k.type == TP_NUMBER) {
            int l = tp_len(tp,self).number.val;
            int n = k.number.val;
            n = (n<0?l+n:n);
            return _tp_list_get(tp,self.list.val,n,"tp_get");
        } else if (k.type == TP_STRING) {
            if (tp_cmp(tp,tp_string("append"),k) == 0) {
                return tp_method(tp,self,tp_append);
            } else if (tp_cmp(tp,tp_string("pop"),k) == 0) {
                return tp_method(tp,self,tp_pop);
            } else if (tp_cmp(tp,tp_string("index"),k) == 0) {
                return tp_method(tp,self,tp_index);
            } else if (tp_cmp(tp,tp_string("sort"),k) == 0) {
                return tp_method(tp,self,tp_sort);
            } else if (tp_cmp(tp,tp_string("extend"),k) == 0) {
                return tp_method(tp,self,tp_extend);
            } else if (tp_cmp(tp,tp_string("*"),k) == 0) {
                tp_params_v(tp,1,self);
                r = tp_copy(tp);
                self.list.val->len=0;
                return r;
            }
        } else if (k.type == TP_NONE) {
            return _tp_list_pop(tp,self.list.val,0,"tp_get");
        }
    } else if (type == TP_STRING) {
        if (k.type == TP_NUMBER) {
            int l = self.string.len;
            int n = k.number.val;
            n = (n<0?l+n:n);
            if (n >= 0 && n < l) { return tp_string_n(tp->chars[(unsigned char)self.string.val[n]],1); }
        } else if (k.type == TP_STRING) {
            if (tp_cmp(tp,tp_string("join"),k) == 0) {
                return tp_method(tp,self,tp_join);
            } else if (tp_cmp(tp,tp_string("split"),k) == 0) {
                return tp_method(tp,self,tp_split);
            } else if (tp_cmp(tp,tp_string("index"),k) == 0) {
                return tp_method(tp,self,tp_str_index);
            } else if (tp_cmp(tp,tp_string("strip"),k) == 0) {
                return tp_method(tp,self,tp_strip);
            } else if (tp_cmp(tp,tp_string("replace"),k) == 0) {
                return tp_method(tp,self,tp_replace);
            }
        }
    }

    if (k.type == TP_LIST) {
        int a,b,l;
        tp_obj tmp;
        l = tp_len(tp,self).number.val;
        tmp = tp_get(tp,k,tp_number(0));
        if (tmp.type == TP_NUMBER) { a = tmp.number.val; }
        else if(tmp.type == TP_NONE) { a = 0; }
        else { tp_raise(tp_None,tp_string("(tp_get) TypeError: indices must be numbers")); }
        tmp = tp_get(tp,k,tp_number(1));
        if (tmp.type == TP_NUMBER) { b = tmp.number.val; }
        else if(tmp.type == TP_NONE) { b = l; }
        else { tp_raise(tp_None,tp_string("(tp_get) TypeError: indices must be numbers")); }
        a = _tp_max(0,(a<0?l+a:a)); b = _tp_min(l,(b<0?l+b:b));
        if (type == TP_LIST) {
            return tp_list_n(tp,b-a,&self.list.val->items[a]);
        } else if (type == TP_STRING) {
            return tp_string_sub(tp,self,a,b);
        }
    }

    tp_raise(tp_None,tp_string("(tp_get) TypeError: ?"));
}

/* Function: tp_iget
 * Failsafe attribute lookup.
 *
 * This is like <tp_get>, except it will return false if the attribute lookup
 * failed. Otherwise, it will return true, and the object will be returned
 * over the reference parameter r.
 */
int tp_iget(TP,tp_obj *r, tp_obj self, tp_obj k) {
    if (self.type == TP_DICT) {
        int n = _tp_dict_find(tp,self.dict.val,k);
        if (n == -1) { return 0; }
        *r = self.dict.val->items[n].val;
        tp_grey(tp,*r);
        return 1;
    }
    if (self.type == TP_LIST && !self.list.val->len) { return 0; }
    *r = tp_get(tp,self,k); tp_grey(tp,*r);
    return 1;
}

/* Function: tp_set
 * Attribute modification.
 *
 * This is the counterpart of tp_get, it does the same as self[k] = v would do
 * in actual tinypy code.
 */
void tp_set(TP,tp_obj self, tp_obj k, tp_obj v) {
    int type = self.type;

    if (type == TP_DICT) {
        TP_META_BEGIN(self,"__set__");
            tp_call(tp,meta,tp_params_v(tp,2,k,v));
            return;
        TP_META_END;
        _tp_dict_set(tp,self.dict.val,k,v);
        return;
    } else if (type == TP_LIST) {
        if (k.type == TP_NUMBER) {
            _tp_list_set(tp,self.list.val,k.number.val,v,"tp_set");
            return;
        } else if (k.type == TP_NONE) {
            _tp_list_append(tp,self.list.val,v);
            return;
        } else if (k.type == TP_STRING) {
            if (tp_cmp(tp,tp_string("*"),k) == 0) {
                tp_params_v(tp,2,self,v); tp_extend(tp);
                return;
            }
        }
    }
    tp_raise(,tp_string("(tp_set) TypeError: object does not support item assignment"));
}

tp_obj tp_add(TP,tp_obj a, tp_obj b) {
    if (a.type == TP_NUMBER && a.type == b.type) {
        return tp_number(a.number.val+b.number.val);
    } else if (a.type == TP_STRING && a.type == b.type) {
        int al = a.string.len, bl = b.string.len;
        tp_obj r = tp_string_t(tp,al+bl);
        char *s = r.string.info->s;
        memcpy(s,a.string.val,al); memcpy(s+al,b.string.val,bl);
        return tp_track(tp,r);
    } else if (a.type == TP_LIST && a.type == b.type) {
        tp_obj r;
        tp_params_v(tp,1,a);
        r = tp_copy(tp);
        tp_params_v(tp,2,r,b);
        tp_extend(tp);
        return r;
    }
    tp_raise(tp_None,tp_string("(tp_add) TypeError: ?"));
}

tp_obj tp_mul(TP,tp_obj a, tp_obj b) {
    if (a.type == TP_NUMBER && a.type == b.type) {
        return tp_number(a.number.val*b.number.val);
    } else if ((a.type == TP_STRING && b.type == TP_NUMBER) ||
               (a.type == TP_NUMBER && b.type == TP_STRING)) {
        if(a.type == TP_NUMBER) {
            tp_obj c = a; a = b; b = c;
        }
        int al = a.string.len; int n = b.number.val;
        if(n <= 0) {
            tp_obj r = tp_string_t(tp,0);
            return tp_track(tp,r);
        }
        tp_obj r = tp_string_t(tp,al*n);
        char *s = r.string.info->s;
        int i; for (i=0; i<n; i++) { memcpy(s+al*i,a.string.val,al); }
        return tp_track(tp,r);
    }
    tp_raise(tp_None,tp_string("(tp_mul) TypeError: ?"));
}

/* Function: tp_len
 * Returns the length of an object.
 *
 * Returns the number of items in a list or dict, or the length of a string.
 */
tp_obj tp_len(TP,tp_obj self) {
    int type = self.type;
    if (type == TP_STRING) {
        return tp_number(self.string.len);
    } else if (type == TP_DICT) {
        return tp_number(self.dict.val->len);
    } else if (type == TP_LIST) {
        return tp_number(self.list.val->len);
    }

    tp_raise(tp_None,tp_string("(tp_len) TypeError: len() of unsized object"));
}

int tp_cmp(TP,tp_obj a, tp_obj b) {
    if (a.type != b.type) { return a.type-b.type; }
    switch(a.type) {
        case TP_NONE: return 0;
        case TP_NUMBER: return _tp_sign(a.number.val-b.number.val);
        case TP_STRING: {
            int l = _tp_min(a.string.len,b.string.len);
            int v = memcmp(a.string.val,b.string.val,l);
            if (v == 0) {
                v = a.string.len-b.string.len;
            }
            return v;
        }
        case TP_LIST: {
            int n,v; for(n=0;n<_tp_min(a.list.val->len,b.list.val->len);n++) {
        tp_obj aa = a.list.val->items[n]; tp_obj bb = b.list.val->items[n];
            if (aa.type == TP_LIST && bb.type == TP_LIST) { v = aa.list.val-bb.list.val; } else { v = tp_cmp(tp,aa,bb); }
            if (v) { return v; } }
            return a.list.val->len-b.list.val->len;
        }
        case TP_DICT: return a.dict.val - b.dict.val;
        case TP_FNC: return a.fnc.info - b.fnc.info;
        case TP_DATA: return (char*)a.data.val - (char*)b.data.val;
    }
    tp_raise(0,tp_string("(tp_cmp) TypeError: ?"));
}

#define TP_OP(name,expr) \
    tp_obj name(TP,tp_obj _a,tp_obj _b) { \
    if (_a.type == TP_NUMBER && _a.type == _b.type) { \
        tp_num a = _a.number.val; tp_num b = _b.number.val; \
        return tp_number(expr); \
    } \
    tp_raise(tp_None,tp_string("(" #name ") TypeError: unsupported operand type(s)")); \
}

TP_OP(tp_bitwise_and,((long)a)&((long)b));
TP_OP(tp_bitwise_or,((long)a)|((long)b));
TP_OP(tp_bitwise_xor,((long)a)^((long)b));
TP_OP(tp_mod,((long)a)%((long)b));
TP_OP(tp_lsh,((long)a)<<((long)b));
TP_OP(tp_rsh,((long)a)>>((long)b));
TP_OP(tp_sub,a-b);
TP_OP(tp_div,a/b);
TP_OP(tp_pow,pow(a,b));

tp_obj tp_bitwise_not(TP, tp_obj a) {
    if (a.type == TP_NUMBER) {
        return tp_number(~(long)a.number.val);
    }
    tp_raise(tp_None,tp_string("(tp_bitwise_not) TypeError: unsupported operand type"));
}

/**/
/* File: VM
 * Functionality pertaining to the virtual machine.
 */

tp_vm *_tp_init(void) {
    int i;
    tp_vm *tp = (tp_vm*)calloc(sizeof(tp_vm),1);
    tp->time_limit = TP_NO_LIMIT;
    tp->clocks = clock();
    tp->time_elapsed = 0.0;
    tp->mem_limit = TP_NO_LIMIT;
    tp->mem_exceeded = 0;
    tp->mem_used = sizeof(tp_vm);
    tp->cur = 0;
    tp->jmp = 0;
    tp->ex = tp_None;
    tp->root = tp_list_nt(tp);
    for (i=0; i<256; i++) { tp->chars[i][0]=i; }
    tp_gc_init(tp);
    tp->_regs = tp_list(tp);
    for (i=0; i<TP_REGS; i++) { tp_set(tp,tp->_regs,tp_None,tp_None); }
    tp->builtins = tp_dict(tp);
    tp->modules = tp_dict(tp);
    tp->_params = tp_list(tp);
    for (i=0; i<TP_FRAMES; i++) { tp_set(tp,tp->_params,tp_None,tp_list(tp)); }
    tp_set(tp,tp->root,tp_None,tp->builtins);
    tp_set(tp,tp->root,tp_None,tp->modules);
    tp_set(tp,tp->root,tp_None,tp->_regs);
    tp_set(tp,tp->root,tp_None,tp->_params);
    tp_set(tp,tp->builtins,tp_string("MODULES"),tp->modules);
    tp_set(tp,tp->modules,tp_string("BUILTINS"),tp->builtins);
    tp_set(tp,tp->builtins,tp_string("BUILTINS"),tp->builtins);
    tp_obj sys = tp_dict(tp);
    tp_set(tp, sys, tp_string("version"), tp_string("tinypy 1.2+SVN"));
    tp_set(tp,tp->modules, tp_string("sys"), sys);
    tp->regs = tp->_regs.list.val->items;
    tp_full(tp);
    return tp;
}


/* Function: tp_deinit
 * Destroys a VM instance.
 *
 * When you no longer need an instance of tinypy, you can use this to free all
 * memory used by it. Even when you are using only a single tinypy instance, it
 * may be good practice to call this function on shutdown.
 */
void tp_deinit(TP) {
    while (tp->root.list.val->len) {
        _tp_list_pop(tp,tp->root.list.val,0,"tp_deinit");
    }
    tp_full(tp); tp_full(tp);
    tp_delete(tp,tp->root);
    tp_gc_deinit(tp);
    tp->mem_used -= sizeof(tp_vm);
    free(tp);
}

/* tp_frame_*/
void tp_frame(TP,tp_obj globals,tp_obj code,tp_obj *ret_dest) {
    tp_frame_ f;
    f.globals = globals;
    f.code = code;
    f.cur = (tp_code*)f.code.string.val;
    f.jmp = 0;
/*     fprintf(stderr,"tp->cur: %d\n",tp->cur);*/
    f.regs = (tp->cur <= 0?tp->regs:tp->frames[tp->cur].regs+tp->frames[tp->cur].cregs);

    f.regs[0] = f.globals;
    f.regs[1] = f.code;
    f.regs += TP_REGS_EXTRA;

    f.ret_dest = ret_dest;
    f.lineno = 0;
    f.line = tp_string("");
    f.name = tp_string("?");
    f.fname = tp_string("?");
    f.cregs = 0;
/*     return f;*/
    if (f.regs+(256+TP_REGS_EXTRA) >= tp->regs+TP_REGS || tp->cur >= TP_FRAMES-1) {
        tp_raise(,tp_string("(tp_frame) RuntimeError: stack overflow"));
    }
    tp->cur += 1;
    tp->frames[tp->cur] = f;
}

void _tp_raise(TP,tp_obj e) {
    /*char *x = 0; x[0]=0;*/
    if (!tp || !tp->jmp) {
#ifndef CPYTHON_MOD
        printf("\nException:\n"); tp_echo(tp,e); printf("\n");
        exit(-1);
#else
        tp->ex = e;
        longjmp(tp->nextexpr,1);
#endif
    }
    if (e.type != TP_NONE) { tp->ex = e; }
    tp_grey(tp,e);
    longjmp(tp->buf,1);
}

void tp_print_stack(TP) {
    int i;
    printf("\n");
    for (i=0; i<=tp->cur; i++) {
        if (!tp->frames[i].lineno) { continue; }
        printf("File \""); tp_echo(tp,tp->frames[i].fname); printf("\", ");
        printf("line %d, in ",tp->frames[i].lineno);
        tp_echo(tp,tp->frames[i].name); printf("\n ");
        tp_echo(tp,tp->frames[i].line); printf("\n");
    }
    printf("\nException:\n"); tp_echo(tp,tp->ex); printf("\n");
}

void tp_handle(TP) {
    int i;
    for (i=tp->cur; i>=0; i--) {
        if (tp->frames[i].jmp) { break; }
    }
    if (i >= 0) {
        tp->cur = i;
        tp->frames[i].cur = tp->frames[i].jmp;
        tp->frames[i].jmp = 0;
        return;
    }
#ifndef CPYTHON_MOD
    tp_print_stack(tp);
    exit(-1);
#else
    longjmp(tp->nextexpr,1);
#endif
}

/* Function: tp_call
 * Calls a tinypy function.
 *
 * Use this to call a tinypy function.
 *
 * Parameters:
 * tp - The VM instance.
 * self - The object to call.
 * params - Parameters to pass.
 *
 * Example:
 * > tp_call(tp,
 * >     tp_get(tp, tp->builtins, tp_string("foo")),
 * >     tp_params_v(tp, tp_string("hello")))
 * This will look for a global function named "foo", then call it with a single
 * positional parameter containing the string "hello".
 */
tp_obj tp_call(TP,tp_obj self, tp_obj params) {
    /* I'm not sure we should have to do this, but
    just for giggles we will. */
    tp->params = params;

    if (self.type == TP_DICT) {
        if (self.dict.dtype == 1) {
            tp_obj meta; if (_tp_lookup(tp,self,tp_string("__new__"),&meta)) {
                _tp_list_insert(tp,params.list.val,0,self);
                return tp_call(tp,meta,params);
            }
        } else if (self.dict.dtype == 2) {
            TP_META_BEGIN(self,"__call__");
                return tp_call(tp,meta,params);
            TP_META_END;
        }
    }
    if (self.type == TP_FNC && !(self.fnc.ftype&1)) {
        tp_obj r = _tp_tcall(tp,self);
        tp_grey(tp,r);
        return r;
    }
    if (self.type == TP_FNC) {
        tp_obj dest = tp_None;
        tp_frame(tp,self.fnc.info->globals,self.fnc.info->code,&dest);
        if ((self.fnc.ftype&2)) {
            tp->frames[tp->cur].regs[0] = params;
            _tp_list_insert(tp,params.list.val,0,self.fnc.info->self);
        } else {
            tp->frames[tp->cur].regs[0] = params;
        }
        tp_run(tp,tp->cur);
        return dest;
    }
    tp_params_v(tp,1,self); tp_print(tp);
    tp_raise(tp_None,tp_string("(tp_call) TypeError: object is not callable"));
}


void tp_return(TP, tp_obj v) {
    tp_obj *dest = tp->frames[tp->cur].ret_dest;
    if (dest) { *dest = v; tp_grey(tp,v); }
/*     memset(tp->frames[tp->cur].regs,0,TP_REGS_PER_FRAME*sizeof(tp_obj));
       fprintf(stderr,"regs:%d\n",(tp->frames[tp->cur].cregs+1));*/
    memset(tp->frames[tp->cur].regs-TP_REGS_EXTRA,0,(TP_REGS_EXTRA+tp->frames[tp->cur].cregs)*sizeof(tp_obj));
    tp->cur -= 1;
}

enum {
    TP_IEOF,TP_IADD,TP_ISUB,TP_IMUL,TP_IDIV,TP_IPOW,TP_IBITAND,TP_IBITOR,TP_ICMP,TP_IGET,TP_ISET,
    TP_INUMBER,TP_ISTRING,TP_IGGET,TP_IGSET,TP_IMOVE,TP_IDEF,TP_IPASS,TP_IJUMP,TP_ICALL,
    TP_IRETURN,TP_IIF,TP_IDEBUG,TP_IEQ,TP_ILE,TP_ILT,TP_IDICT,TP_ILIST,TP_INONE,TP_ILEN,
    TP_ILINE,TP_IPARAMS,TP_IIGET,TP_IFILE,TP_INAME,TP_INE,TP_IHAS,TP_IRAISE,TP_ISETJMP,
    TP_IMOD,TP_ILSH,TP_IRSH,TP_IITER,TP_IDEL,TP_IREGS,TP_IBITXOR, TP_IIFN,
    TP_INOT, TP_IBITNOT,
    TP_ITOTAL
};

/* char *tp_strings[TP_ITOTAL] = {
       "EOF","ADD","SUB","MUL","DIV","POW","BITAND","BITOR","CMP","GET","SET","NUM",
       "STR","GGET","GSET","MOVE","DEF","PASS","JUMP","CALL","RETURN","IF","DEBUG",
       "EQ","LE","LT","DICT","LIST","NONE","LEN","LINE","PARAMS","IGET","FILE",
       "NAME","NE","HAS","RAISE","SETJMP","MOD","LSH","RSH","ITER","DEL","REGS",
       "BITXOR", "IFN", "NOT", "BITNOT",
   };*/

#define VA ((int)e.regs.a)
#define VB ((int)e.regs.b)
#define VC ((int)e.regs.c)
#define RA regs[e.regs.a]
#define RB regs[e.regs.b]
#define RC regs[e.regs.c]
#define UVBC (unsigned short)(((VB<<8)+VC))
#define SVBC (short)(((VB<<8)+VC))
#define GA tp_grey(tp,RA)
#define SR(v) f->cur = cur; return(v);


int tp_step(TP) {
    tp_frame_ *f = &tp->frames[tp->cur];
    tp_obj *regs = f->regs;
    tp_code *cur = f->cur;
    while(1) {
    #ifdef TP_SANDBOX
    tp_bounds(tp,cur,1);
    #endif
    tp_code e = *cur;
    /*
     fprintf(stderr,"%2d.%4d: %-6s %3d %3d %3d\n",tp->cur,cur - (tp_code*)f->code.string.val,tp_strings[e.i],VA,VB,VC);
       int i; for(i=0;i<16;i++) { fprintf(stderr,"%d: %s\n",i,TP_xSTR(regs[i])); }
    */
    switch (e.i) {
        case TP_IEOF: tp_return(tp,tp_None); SR(0); break;
        case TP_IADD: RA = tp_add(tp,RB,RC); break;
        case TP_ISUB: RA = tp_sub(tp,RB,RC); break;
        case TP_IMUL: RA = tp_mul(tp,RB,RC); break;
        case TP_IDIV: RA = tp_div(tp,RB,RC); break;
        case TP_IPOW: RA = tp_pow(tp,RB,RC); break;
        case TP_IBITAND: RA = tp_bitwise_and(tp,RB,RC); break;
        case TP_IBITOR:  RA = tp_bitwise_or(tp,RB,RC); break;
        case TP_IBITXOR:  RA = tp_bitwise_xor(tp,RB,RC); break;
        case TP_IMOD:  RA = tp_mod(tp,RB,RC); break;
        case TP_ILSH:  RA = tp_lsh(tp,RB,RC); break;
        case TP_IRSH:  RA = tp_rsh(tp,RB,RC); break;
        case TP_ICMP: RA = tp_number(tp_cmp(tp,RB,RC)); break;
        case TP_INE: RA = tp_number(tp_cmp(tp,RB,RC)!=0); break;
        case TP_IEQ: RA = tp_number(tp_cmp(tp,RB,RC)==0); break;
        case TP_ILE: RA = tp_number(tp_cmp(tp,RB,RC)<=0); break;
        case TP_ILT: RA = tp_number(tp_cmp(tp,RB,RC)<0); break;
        case TP_IBITNOT:  RA = tp_bitwise_not(tp,RB); break;
        case TP_INOT: RA = tp_number(!tp_bool(tp,RB)); break;
        case TP_IPASS: break;
        case TP_IIF: if (tp_bool(tp,RA)) { cur += 1; } break;
        case TP_IIFN: if (!tp_bool(tp,RA)) { cur += 1; } break;
        case TP_IGET: RA = tp_get(tp,RB,RC); GA; break;
        case TP_IITER:
            if (RC.number.val < tp_len(tp,RB).number.val) {
                RA = tp_iter(tp,RB,RC); GA;
                RC.number.val += 1;
                #ifdef TP_SANDBOX
                tp_bounds(tp,cur,1);
                #endif
                cur += 1;
            }
            break;
        case TP_IHAS: RA = tp_has(tp,RB,RC); break;
        case TP_IIGET: tp_iget(tp,&RA,RB,RC); break;
        case TP_ISET: tp_set(tp,RA,RB,RC); break;
        case TP_IDEL: tp_del(tp,RA,RB); break;
        case TP_IMOVE: RA = RB; break;
        case TP_INUMBER:
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,sizeof(tp_num)/4);
            #endif
            RA = tp_number(*(tp_num*)(*++cur).string.val);
            cur += sizeof(tp_num)/4;
            continue;
        case TP_ISTRING: {
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,(UVBC/4)+1);
            #endif
            /* RA = tp_string_n((*(cur+1)).string.val,UVBC); */
            int a = (*(cur+1)).string.val-f->code.string.val;
            RA = tp_string_sub(tp,f->code,a,a+UVBC),
            cur += (UVBC/4)+1;
            }
            break;
        case TP_IDICT: RA = tp_dict_n(tp,VC/2,&RB); break;
        case TP_ILIST: RA = tp_list_n(tp,VC,&RB); break;
        case TP_IPARAMS: RA = tp_params_n(tp,VC,&RB); break;
        case TP_ILEN: RA = tp_len(tp,RB); break;
        case TP_IJUMP: cur += SVBC; continue; break;
        case TP_ISETJMP: f->jmp = SVBC?cur+SVBC:0; break;
        case TP_ICALL:
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,1);
            #endif
            f->cur = cur + 1;  RA = tp_call(tp,RB,RC); GA;
            return 0; break;
        case TP_IGGET:
            if (!tp_iget(tp,&RA,f->globals,RB)) {
                RA = tp_get(tp,tp->builtins,RB); GA;
            }
            break;
        case TP_IGSET: tp_set(tp,f->globals,RA,RB); break;
        case TP_IDEF: {
/*            RA = tp_def(tp,(*(cur+1)).string.val,f->globals);*/
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,SVBC);
            #endif
            int a = (*(cur+1)).string.val-f->code.string.val;
            RA = tp_def(tp,
                /*tp_string_n((*(cur+1)).string.val,(SVBC-1)*4),*/
                tp_string_sub(tp,f->code,a,a+(SVBC-1)*4),
                f->globals);
            cur += SVBC; continue;
            }
            break;

        case TP_IRETURN: tp_return(tp,RA); SR(0); break;
        case TP_IRAISE: _tp_raise(tp,RA); SR(0); break;
        case TP_IDEBUG:
            tp_params_v(tp,3,tp_string("DEBUG:"),tp_number(VA),RA); tp_print(tp);
            break;
        case TP_INONE: RA = tp_None; break;
        case TP_ILINE:
            #ifdef TP_SANDBOX
            tp_bounds(tp,cur,VA);
            #endif
            ;
            int a = (*(cur+1)).string.val-f->code.string.val;
/*            f->line = tp_string_n((*(cur+1)).string.val,VA*4-1);*/
            f->line = tp_string_sub(tp,f->code,a,a+VA*4-1);
/*             fprintf(stderr,"%7d: %s\n",UVBC,f->line.string.val);*/
            cur += VA; f->lineno = UVBC;
            break;
        case TP_IFILE: f->fname = RA; break;
        case TP_INAME: f->name = RA; break;
        case TP_IREGS: f->cregs = VA; break;
        default:
            tp_raise(0,tp_string("(tp_step) RuntimeError: invalid instruction"));
            break;
    }
    #ifdef TP_SANDBOX
    tp_time_update(tp);
    tp_mem_update(tp);
    tp_bounds(tp,cur,1);
    #endif
    cur += 1;
    }
    SR(0);
}

void _tp_run(TP,int cur) {
    tp->jmp += 1; if (setjmp(tp->buf)) { tp_handle(tp); }
    while (tp->cur >= cur && tp_step(tp) != -1);
    tp->jmp -= 1;
}

void tp_run(TP,int cur) {
    jmp_buf tmp;
    memcpy(tmp,tp->buf,sizeof(jmp_buf));
    _tp_run(tp,cur);
    memcpy(tp->buf,tmp,sizeof(jmp_buf));
}


tp_obj tp_ez_call(TP, const char *mod, const char *fnc, tp_obj params) {
    tp_obj tmp;
    tmp = tp_get(tp,tp->modules,tp_string(mod));
    tmp = tp_get(tp,tmp,tp_string(fnc));
    return tp_call(tp,tmp,params);
}

tp_obj _tp_import(TP, tp_obj fname, tp_obj name, tp_obj code) {
    tp_obj g;

    if (!((fname.type != TP_NONE && _tp_str_index(fname,tp_string(".tpc"))!=-1) || code.type != TP_NONE)) {
        return tp_ez_call(tp,"py2bc","import_fname",tp_params_v(tp,2,fname,name));
    }

    if (code.type == TP_NONE) {
        tp_params_v(tp,1,fname);
        code = tp_load(tp);
    }

    g = tp_dict(tp);
    tp_set(tp,g,tp_string("__name__"),name);
    tp_set(tp,g,tp_string("__code__"),code);
    tp_set(tp,g,tp_string("__dict__"),g);
    tp_frame(tp,g,code,0);
    tp_set(tp,tp->modules,name,g);

    if (!tp->jmp) { tp_run(tp,tp->cur); }

    return g;
}


/* Function: tp_import
 * Imports a module.
 *
 * Parameters:
 * fname - The filename of a file containing the module's code.
 * name - The name of the module.
 * codes - The module's code.  If this is given, fname is ignored.
 * len - The length of the bytecode.
 *
 * Returns:
 * The module object.
 */
tp_obj tp_import(TP, const char * fname, const char * name, void *codes, int len) {
    tp_obj f = fname?tp_string(fname):tp_None;
    tp_obj bc = codes?tp_string_n((const char*)codes,len):tp_None;
    return _tp_import(tp,f,tp_string(name),bc);
}



tp_obj tp_exec_(TP) {
    tp_obj code = TP_OBJ();
    tp_obj globals = TP_OBJ();
    tp_obj r = tp_None;
    tp_frame(tp,globals,code,&r);
    tp_run(tp,tp->cur);
    return r;
}


tp_obj tp_import_(TP) {
    tp_obj mod = TP_OBJ();
    tp_obj r;

    if (tp_has(tp,tp->modules,mod).number.val) {
        return tp_get(tp,tp->modules,mod);
    }

    r = _tp_import(tp,tp_add(tp,mod,tp_string(".tpc")),mod,tp_None);
    return r;
}

void tp_builtins(TP) {
    tp_obj o;
    struct {const char *s;void *f;} b[] = {
    {"print",tp_print}, {"range",tp_range}, {"min",tp_min},
    {"max",tp_max}, {"bind",tp_bind}, {"copy",tp_copy},
    {"import",tp_import_}, {"len",tp_len_}, {"assert",tp_assert},
    {"str",tp_str2}, {"float",tp_float}, {"system",tp_system},
    {"istype",tp_istype}, {"chr",tp_chr}, {"save",tp_save},
    {"load",tp_load}, {"fpack",tp_fpack}, {"abs",tp_abs},
    {"int",tp_int}, {"exec",tp_exec_}, {"exists",tp_exists},
    {"mtime",tp_mtime}, {"number",tp_float}, {"round",tp_round},
    {"ord",tp_ord}, {"merge",tp_merge}, {"getraw",tp_getraw},
    {"setmeta",tp_setmeta}, {"getmeta",tp_getmeta},
    {"bool", tp_builtins_bool},
    #ifdef TP_SANDBOX
    {"sandbox",tp_sandbox_},
    #endif
    {0,0},
    };
    int i; for(i=0; b[i].s; i++) {
        tp_set(tp,tp->builtins,tp_string(b[i].s),tp_fnc(tp,(tp_obj (*)(tp_vm *))b[i].f));
    }

    o = tp_object(tp);
    tp_set(tp,o,tp_string("__call__"),tp_fnc(tp,tp_object_call));
    tp_set(tp,o,tp_string("__new__"),tp_fnc(tp,tp_object_new));
    tp_set(tp,tp->builtins,tp_string("object"),o);
}


void tp_args(TP,int argc, char *argv[]) {
    tp_obj self = tp_list(tp);
    int i;
    for (i=1; i<argc; i++) { _tp_list_append(tp,self.list.val,tp_string(argv[i])); }
    tp_set(tp,tp->builtins,tp_string("ARGV"),self);
}

tp_obj tp_main(TP,char *fname, void *code, int len) {
    return tp_import(tp,fname,"__main__",code, len);
}

/* Function: tp_compile
 * Compile some tinypy code.
 *
 */
tp_obj tp_compile(TP, tp_obj text, tp_obj fname) {
    return tp_ez_call(tp,"BUILTINS","compile",tp_params_v(tp,2,text,fname));
}

/* Function: tp_exec
 * Execute VM code.
 */
tp_obj tp_exec(TP, tp_obj code, tp_obj globals) {
    tp_obj r=tp_None;
    tp_frame(tp,globals,code,&r);
    tp_run(tp,tp->cur);
    return r;
}

tp_obj tp_eval(TP, const char *text, tp_obj globals) {
    tp_obj code = tp_compile(tp,tp_string(text),tp_string("<eval>"));
    return tp_exec(tp,code,globals);
}

/* Function: tp_init
 * Initializes a new virtual machine.
 *
 * The given parameters have the same format as the parameters to main, and
 * allow passing arguments to your tinypy scripts.
 *
 * Returns:
 * The newly created tinypy instance.
 */
tp_vm *tp_init(int argc, char *argv[]) {
    tp_vm *tp = _tp_init();
    tp_builtins(tp);
    tp_args(tp,argc,argv);
    tp_compiler(tp);
    return tp;
}

#ifndef TP_COMPILER
#define TP_COMPILER 1
#endif

#ifdef TP_SANDBOX
#endif

void tp_compiler(TP);

tp_obj tp_None = {TP_NONE};

#if TP_COMPILER
void tp_compiler(TP) {
    tp_import(tp,0,"tokenize",tp_tokenize,sizeof(tp_tokenize));
    tp_import(tp,0,"parse",tp_parse,sizeof(tp_parse));
    tp_import(tp,0,"encode",tp_encode,sizeof(tp_encode));
    tp_import(tp,0,"py2bc",tp_py2bc,sizeof(tp_py2bc));
    tp_ez_call(tp,"py2bc","_init",tp_None);
}
#else
void tp_compiler(TP) { }
#endif

/**/

void tp_sandbox(TP, double time_limit, unsigned long mem_limit) {
    tp->time_limit = time_limit;
    tp->mem_limit = mem_limit;
}

void tp_mem_update(TP) {
/*    static long maxmem = 0;
    if (tp->mem_used/1024 > maxmem) {
        maxmem = tp->mem_used/1024;
        fprintf(stderr,"%ld k\n",maxmem);
    }*/
    if((!tp->mem_exceeded) &&
       (tp->mem_used > tp->mem_limit) &&
       (tp->mem_limit != TP_NO_LIMIT)) {
        tp->mem_exceeded = 1;
        tp_raise(,tp_string("(tp_mem_update) SandboxError: memory limit exceeded"));
    }
}

void tp_time_update(TP) {
    clock_t tmp = tp->clocks;
    if(tp->time_limit != TP_NO_LIMIT)
    {
        tp->clocks = clock();
        tp->time_elapsed += ((double) (tp->clocks - tmp) / CLOCKS_PER_SEC) * 1000.0;
        if(tp->time_elapsed >= tp->time_limit)
            tp_raise(,tp_string("(tp_time_update) SandboxError: time limit exceeded"));
    }
}

#ifdef TP_SANDBOX

void *tp_malloc(TP, unsigned long bytes) {
    unsigned long *ptr = (unsigned long *) calloc(bytes + sizeof(unsigned long), 1);
    if(ptr) {
        *ptr = bytes;
        tp->mem_used += bytes + sizeof(unsigned long);
    }
    tp_mem_update(tp);
    return ptr+1;
}

void tp_free(TP, void *ptr) {
    unsigned long *temp = (unsigned long *) ptr;
    if(temp) {
        --temp;
        tp->mem_used -= (*temp + sizeof(unsigned long));
        free(temp);
    }
    tp_mem_update(tp);
}

void *tp_realloc(TP, void *ptr, unsigned long bytes) {
    unsigned long *temp = (unsigned long *) ptr;
    int diff;
    if(temp && bytes) {
        --temp;
        diff = bytes - *temp;
        *temp = bytes;
        tp->mem_used += diff;
        temp = (unsigned long *) realloc(temp, bytes+sizeof(unsigned long));
        return temp+1;
    }
    else if(temp && !bytes) {
        tp_free(tp, temp);
        return NULL;
    }
    else if(!temp && bytes) {
        return tp_malloc(tp, bytes);
    }
    else {
        return NULL;
    }
}

#endif

tp_obj tp_sandbox_(TP) {
    tp_num time = TP_NUM();
    tp_num mem = TP_NUM();
    tp_sandbox(tp, time, mem);
    tp_del(tp, tp->builtins, tp_string("sandbox"));
    tp_del(tp, tp->builtins, tp_string("mtime"));
    tp_del(tp, tp->builtins, tp_string("load"));
    tp_del(tp, tp->builtins, tp_string("save"));
    tp_del(tp, tp->builtins, tp_string("system"));
    return tp_None;
}

void tp_bounds(TP, tp_code *cur, int n) {
    char *s = (char *)(cur + n);
    tp_obj code = tp->frames[tp->cur].code;
    if (s < code.string.val || s > (code.string.val+code.string.len)) {
        tp_raise(,tp_string("(tp_bounds) SandboxError: bytecode bounds reached"));
    }
}
