#ifndef DEFS_H
#define DEFS_H
#include <stddef.h>
#include <stdio.h>

extern FILE *out;

extern int nroff;
extern int local_lynx;

typedef struct STRDEF STRDEF;
struct STRDEF {
    int nr,slen;
    char *st;
    STRDEF *next;
};

typedef struct INTDEF INTDEF;
struct INTDEF {
    int nr;
    int val;
    int incr;
    INTDEF *next;
};

extern STRDEF *chardef, *strdef, *defdef;
extern INTDEF *intdef;

#define V(A,B) ((A)*256+(B))

extern void stdinit(void);
extern void print_sig(void);
extern char *lookup_abbrev(char *);
extern void include_file_html(char *);
extern void man_page_html(char*, char *);
extern void ftp_html(char *);
extern void www_html(char *);
extern void mailto_html(char *);
extern void url_html(char *);
extern void *xmalloc(size_t size);
extern void *xrealloc(void *ptr, size_t size);
extern char *xstrdup(const char *s);
#endif