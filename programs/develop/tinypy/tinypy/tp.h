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

#ifdef __GNUC__
#define tp_inline __inline__
#endif

#ifdef _MSC_VER
#define tp_inline __inline
#endif

#ifndef tp_inline
#error "Unsuported compiler"
#endif

#define tp_malloc(x) calloc((x),1)
#define tp_realloc(x,y) realloc(x,y)
#define tp_free(x) free(x)

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
} tp_dict_;
typedef struct tp_fnc_ {
    int type;
    struct _tp_fnc *info;
    int ftype;
    void *val;
} tp_fnc_;
typedef struct tp_data_ {
    int type;
    struct _tp_data *info;
    void *val;
    int magic;
} tp_data_;

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
} _tp_dict;
typedef struct _tp_fnc {
    int gci;
    tp_obj self;
    tp_obj globals;
} _tp_fnc;


typedef union tp_code {
    unsigned char i;
    struct { unsigned char i,a,b,c; } regs;
    struct { char val[4]; } string;
    struct { float val; } number;
} tp_code;

typedef struct tp_frame_ {
    tp_code *codes;
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
/* #define TP_REGS_PER_FRAME 256*/
#define TP_REGS 16384
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
    int jmp;
    tp_obj ex;
    char chars[256][2];
    int cur;
    /* gc*/
    _tp_list *white;
    _tp_list *grey;
    _tp_list *black;
    _tp_dict *strings;
    int steps;
} tp_vm;

#define TP tp_vm *tp
typedef struct _tp_data {
    int gci;
    void (*free)(TP,tp_obj);
} _tp_data;

/* NOTE: these are the few out of namespace items for convenience*/
#define tp_True tp_number(1)
#define tp_False tp_number(0)
#define TP_CSTR(v) ((tp_str(tp,(v))).string.val)

extern tp_obj tp_None;

void tp_set(TP,tp_obj,tp_obj,tp_obj);
tp_obj tp_get(TP,tp_obj,tp_obj);
tp_obj tp_len(TP,tp_obj);
tp_obj tp_str(TP,tp_obj);
int tp_cmp(TP,tp_obj,tp_obj);
void _tp_raise(TP,tp_obj);
tp_obj tp_printf(TP,char const *fmt,...);
tp_obj tp_track(TP,tp_obj);
void tp_grey(TP,tp_obj);

/* __func__ __VA_ARGS__ __FILE__ __LINE__ */
#define tp_raise(r,fmt,...) { \
    _tp_raise(tp,tp_printf(tp,fmt,__VA_ARGS__)); \
    return r; \
}
#define TP_OBJ() (tp_get(tp,tp->params,tp_None))
tp_inline static tp_obj tp_type(TP,int t,tp_obj v) {
    if (v.type != t) { tp_raise(tp_None,"_tp_type(%d,%s)",t,TP_CSTR(v)); }
    return v;
}
#define TP_TYPE(t) tp_type(tp,t,TP_OBJ())
#define TP_NUM() (TP_TYPE(TP_NUMBER).number.val)
#define TP_STR() (TP_CSTR(TP_TYPE(TP_STRING)))
#define TP_DEFAULT(d) (tp->params.list.val->len?tp_get(tp,tp->params,tp_None):(d))
#define TP_LOOP(e) \
    int __l = tp->params.list.val->len; \
    int __i; for (__i=0; __i<__l; __i++) { \
    (e) = _tp_list_get(tp,tp->params.list.val,__i,"TP_LOOP");
#define TP_END \
    }

tp_inline static int _tp_min(int a, int b) { return (a<b?a:b); }
tp_inline static int _tp_max(int a, int b) { return (a>b?a:b); }
tp_inline static int _tp_sign(tp_num v) { return (v<0?-1:(v>0?1:0)); }

tp_inline static tp_obj tp_number(tp_num v) {
    tp_obj val = {TP_NUMBER};
    val.number.val = v;
    return val;
}

tp_inline static tp_obj tp_string(char const *v) {
    tp_obj val;
    tp_string_ s = {TP_STRING, 0, v, 0};
    s.len = strlen(v);
    val.string = s;
    return val;
}

tp_inline static tp_obj tp_string_n(char const *v,int n) {
    tp_obj val;
    tp_string_ s = {TP_STRING, 0,v,n};
    val.string = s;
    return val;
}

#endif
