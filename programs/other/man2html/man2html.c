/*
** This program was written by Richard Verhoeven (NL:5482ZX35)
** at the Eindhoven University of Technology. Email: rcb5@win.tue.nl
**
** Permission is granted to distribute, modify and use this program
** as long as this comment is not removed or changed.
*/

/* BSD mandoc stuff added by Michael Hamilton. */

/* This program is rather buggy, but in spite of that it often works.
   Improved things a little - April 1997 & January 1998 & Dec 2001 -
   aeb@cwi.nl. */

/* some code added by Tsukasa Hamnao. */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>

#ifdef _KOLIBRI
#include <sys/ksys.h>
#include <sys/dir.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "defs.h"
static char version[] = "1.6f-1";

/* BSD mandoc Bd/Ed example(?) blocks */
#define BD_LITERAL  1
#define BD_INDENT   2

#define SIZE(a)    (sizeof(a)/sizeof(*a))

static char NEWLINE[2]="\n";
static char idxlabel[6] = "ixAAA";

#ifdef _KOLIBRI
    #define INDEXFILE "/tmp0/1/manindex.list"
#else
    #define INDEXFILE "./manindex.list"
#endif

char *fname;
//char *directory;
FILE *idxfile;

char eqndelimopen=0, eqndelimclose=0;
char escapesym='\\', nobreaksym='\'', controlsym='.', fieldsym=0, padsym=0;

char *buffer=NULL;
int buffpos=0, buffmax=0;
int scaninbuff=0;
int still_dd=0;
int tabstops[20] = { 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96 };
int maxtstop=12;
int curpos=0;

static char *scan_troff(char *c, int san, char **result);
static char *scan_troff_mandoc(char *c, int san, char **result);

static char **argument=NULL;

static char charb[3];

FILE *out;

static char *
expand_char(int nr)
{
    STRDEF *h;

    if (!nr)
        return NULL;

    h = chardef;
    if (h->nr != V('*','*')) {
        fprintf(out, "chardef corrupted\n");
        exit(1);
    }

    for (h = chardef; h; h = h->next)
        if (h->nr == nr) {
            curpos += h->slen;
            return h->st;
        }
    charb[0] = nr/256;
    charb[1] = nr%256;
    charb[2] = 0;
    curpos += 2;
    return charb;
}

static char *
expand_string(int nr)
{
    STRDEF *h;

    if (!nr)
        return NULL;
    for (h = strdef; h; h = h->next)
        if (h->nr == nr) {
            curpos += h->slen;
            return h->st;
        }
    return NULL;
}


static char outbuffer[1024];
static int obp=0;
static int no_newline_output=0;        /* boolean, set by \c */
static int newline_for_fun=0;
static int output_possible=0;
static int out_length=0;

static void
add_links(char *c)
{
    /*
    ** Add the links to the output.
    ** At the moment the following are recognized:
    **
    ** name(*)                 -> ../man?/name.*
    ** method://string         -> method://string
    ** www.host.name           -> http://www.host.name
    ** ftp.host.name           -> ftp://ftp.host.name
    ** name@host               -> mailto:name@host
    ** <name.h>                -> file:/usr/include/name.h   (guess)
    **
    ** Other possible links to add in the future:
    **
    ** /dir/dir/file  -> file:/dir/dir/file
    */
    int i,j,nr;
    char *f, *g, *h;
    char *idtest[6]; /* url, mailto, www, ftp, manpage, include file */

    out_length+=strlen(c);

    nr=0;
    idtest[0]=strstr(c+1,"://");
    /* idtest[1]=strchr(c+1,'@'); */
    idtest[1]=NULL; /* don't create mailto links. */
    idtest[2]=strstr(c,"www.");
    idtest[3]=strstr(c,"ftp.");
    idtest[4]=strchr(c+1,'(');
    idtest[5]=strstr(c+1,".h&gt;");
    for (i=0; i<6; i++) nr += (idtest[i]!=NULL);
    while (nr) {
    j=-1;
    for (i=0; i<6; i++)
        if (idtest[i] && (j<0 || idtest[i]<idtest[j])) j=i;
    switch (j) {
    case 5: /* <name.h> */
        f=idtest[5];
        h=f+2;
        g=f;
        while (g>c && g[-1]!=';') g--;
        if (g!=c) {
        char t;
        t=*g;
        *g=0;
        fprintf(out, "%s",c);
        *g=t;*h=0;
        include_file_html(g);
        c=f+6;
        } else {
        f[5]=0;
        fprintf(out, "%s",c);
        f[5]=';';
        c=f+5;
        }
        break;
    case 4:            /* manpage? */
        f=idtest[j];
        /* find section - accept (1), (3F), (3Xt), (n), (l) */
        g=strchr(f,')');
        if (g && g-f<7    /* section has length at most 5, like 3Xlib */
                        /* preceded by name or html markup */
          && (isalnum(f[-1]) || f[-1]=='>')
                                /* section is n or l or starts with a digit */
          && strchr("123456789nl", f[1])
          && (g-f == 2 || (g-f == 3 && isdigit(f[1]) && isalpha(f[2]))
                       || (f[2] == 'X' && isdigit(f[1])))
           ) {
        /* this might be a link */
        h=f-1;
        /* skip html markup */
        while (h>c && *h=='>') {
            while (h!=c && *h!='<') h--;
            if (h!=c) h--;
        }
        if (isalnum(*h)) {
            char t,te,tg,*e;
            e=h+1;
            while (h>c && (isalnum(h[-1]) || h[-1]=='_' ||
                    h[-1]=='-' || h[-1]=='.' || h[-1]==':'))
            h--;
            t=*h; *h=0;
            fprintf(out, "%s", c);
            *h=t;
            tg=*g; *g=0;
            te=*e; *e=0;
            man_page_html(f+1, h);     /* section, page */
            *e=te;
            *g=tg;
            c=e;
        }
        }
        *f=0;
        fprintf(out, "%s", c);
        *f='(';
        idtest[4]=f-1;
        c=f;
        break; /* manpage */
    case 3: /* ftp */
    case 2: /* www */
        g=f=idtest[j];
        while (*g && (isalnum(*g) || *g=='_' || *g=='-' || *g=='+' ||
              *g=='.')) g++;
        if (g[-1]=='.') g--;
        if (g-f>4) {
        char t;
        t=*f; *f=0;
        fprintf(out, "%s",c);
        *f=t; t=*g;*g=0;
        if (j==3)
             ftp_html(f);
        else
             www_html(f);
        *g=t;
        c=g;
        } else {
        f[3]=0;
        fprintf(out, "%s",c);
        c=f+3;
        f[3]='.';
        }
        break;
    case 1: /* mailto */
        g=f=idtest[1];
        while (g>c && (isalnum(g[-1]) || g[-1]=='_' || g[-1]=='-' ||
               g[-1]=='+' || g[-1]=='.' || g[-1]=='%')) g--;
        h=f+1;
        while (*h && (isalnum(*h) || *h=='_' || *h=='-' || *h=='+' ||
              *h=='.')) h++;
        if (h[-1]=='.') h--;
        if (h-f>4 && f-g>1) {
        char t;
        t=*g;
        *g=0;
        fprintf(out, "%s",c);
        *g=t;t=*h;*h=0;
        mailto_html(g);
        *h=t;
        c=h;
        } else {
        *f=0;
        fprintf(out, "%s",c);
        *f='@';
        idtest[1]=c;
        c=f;
        }
        break;
    case 0: /* url */
        g=f=idtest[0];
        while (g>c && isalpha(g[-1]) && islower(g[-1])) g--;
        h=f+3;
        while (*h && !isspace(*h) && *h!='<' && *h!='>' && *h!='"' &&
           *h!='&') h++;
        if (f-g>2 && f-g<7 && h-f>3) {
        char t;
        t=*g;
        *g=0;
        fprintf(out, "%s", c);
        *g=t; t=*h; *h=0;
        url_html(g);
        *h=t;
        c=h;
        } else {
        f[1]=0;
        fprintf(out, "%s", c);
        f[1]='/';
        c=f+1;
        }
        break;
    default:
        break;
    }
    nr=0;
    if (idtest[0] && idtest[0]<c) idtest[0]=strstr(c+1,"://");
    if (idtest[1] && idtest[1]<c) idtest[1]=strchr(c+1,'@');
    if (idtest[2] && idtest[2]<c) idtest[2]=strstr(c,"www.");
    if (idtest[3] && idtest[3]<c) idtest[3]=strstr(c,"ftp.");
    if (idtest[4] && idtest[4]<c) idtest[4]=strchr(c+1,'(');
    if (idtest[5] && idtest[5]<c) idtest[5]=strstr(c+1,".h&gt;");
    for (i=0; i<6; i++) nr += (idtest[i]!=NULL);
    }
    fprintf(out, "%s", c);
}

int current_font=0;
int current_size=0;
int fillout = 1;

/*
 * Kludge: remove \a - in the context
 *   .TH NAME 2 date "Version" "Title"
 * we got output \aTitle\a.
 */
static void
out_html(char *c) {
    if (!c)
        return;
    if (no_newline_output) {    /* remove \n if present */
        int i=0;
        while (c[i]) {
            if (!no_newline_output)
                c[i-1]=c[i];
            if (c[i]=='\n')
                no_newline_output=0;
            i++;
        }
        if (!no_newline_output)
            c[i-1]=0;
    }
    if (scaninbuff) {
        while (*c) {
            if (buffpos >= buffmax) {
                buffer = xrealloc(buffer, buffmax*2);
                buffmax = buffmax*2;
            }
            if (*c != '\a')
                buffer[buffpos++] = *c;
            c++;
        }
    } else if (output_possible) {
        while (*c) {
            if (*c != '\a')
                outbuffer[obp++] = *c;
            if (*c == '\n' || obp > 1000) {
                outbuffer[obp] = 0;
                add_links(outbuffer);
                obp = 0;
            }
            c++;
        }
    }
}

/* --------------------------------------------------------------- */
/* All references to dl_set and itemdepth are here.                */
/* --------------------------------------------------------------- */
static int itemdepth=0;
static int dl_set[30]= { 0 };
#define noDL  0
#define DL    1
#define UL    2
#define OL    3
static char *dl_open[4] = { "", "<DL COMPACT>\n", "<UL>", "<OL>" };
static char *dl_close[4] = { "", "</DL>\n", "</UL>", "</OL>" };

static inline void
dl_begin(void) {
     if (itemdepth < SIZE(dl_set) && dl_set[itemdepth] == noDL) {
      out_html(dl_open[DL]);
      dl_set[itemdepth]=DL;
     }
     out_html("<DT>");
}

static inline void
dl_end(void) {
     if (itemdepth < SIZE(dl_set)) {
      int type = dl_set[itemdepth];
      if (type == DL) {
           out_html(dl_close[type]);
           dl_set[itemdepth]=noDL;
      }
     }
}

static inline void
dl_newlevel(void) {
     itemdepth++;
     if (itemdepth < SIZE(dl_set))
      dl_set[itemdepth]=noDL;
     out_html("<DL COMPACT><DT><DD>");
}

static inline void
dl_endlevel(void) {
     if (itemdepth) {
      dl_end();
      out_html("</DL>\n");
      itemdepth--;
     }
}

static inline void
dl_down(void) {
     while (itemdepth)
      dl_endlevel();
     dl_end();
}

static inline int
dl_type(int type) {
     return (itemdepth < SIZE(dl_set) && dl_set[itemdepth] == type);
}

static inline void
dl_newlevel_type(int type) {
     itemdepth++;
     if (itemdepth < SIZE(dl_set)) {
      dl_set[itemdepth]=type;
      out_html(dl_open[type]);
     }
}

static inline void
dl_endlevel_type(void) {
     if (itemdepth) {
      if (itemdepth < SIZE(dl_set))
           out_html(dl_close[dl_set[itemdepth]]);
      itemdepth--;
     }
}
/* --------------------------------------------------------------- */
/* This stuff is broken.
It generates
   <DT><B>TIOCLINUX, subcode=0<DD>
   Dump the screen.
   </B><I>argp</I> points to a
from
   .IP "\fBTIOCLINUX, subcode=0"
   Dump the screen.
   \fIargp\fP points to a
Bug 1: incorrect nesting: </B> is needed before <DD>.
Bug 2: incorrect font: after the .IP things are roman again.
*/

#define FO0 ""
#define FC0 ""
#define FO1 "<I>"
#define FC1 "</I>"
#define FO2 "<B>"
#define FC2 "</B>"
#define FO3 "<TT>"
#define FC3 "</TT>"

char *switchfont[16] = { "", FC0 FO1, FC0 FO2, FC0 FO3,
                         FC1 FO0, "", FC1 FO2, FC1 FO3,
                         FC2 FO0, FC2 FO1, ""     , FC2 FO3,
                         FC3 FO0, FC3 FO1, FC3 FO2, ""};

static char *
change_to_font(int nr)
{
  int i;
  switch (nr) {
  case '0': nr++;
  case '1': case '2': case '3': case '4':
       nr = nr-'1'; break;
  case V('C','W'): nr=3; break;
  case 'L': nr=3; break;
  case 'B': nr=2; break;
  case 'I': nr=1; break;
  case 0: case 1: case 2: case 3:
       break;
  case 'P': case 'R':
  default: nr=0; break;
  }
  i= current_font*4+nr%4;
  current_font=nr%4;
  return switchfont[i];
}

static char sizebuf[200];

static char *
change_to_size(int nr)
{
  int i;
  switch (nr) {
  case '0': case '1': case '2': case '3': case '4': case '5': case '6':
  case '7': case '8': case '9': nr=nr-'0'; break;
  case '\0': break;
  default: nr=current_size+nr; if (nr>9) nr=9; if (nr< -9) nr=-9; break;
  }
  if (nr==current_size) return "";
  i=current_font;
  sizebuf[0]=0;
  strcat(sizebuf, change_to_font(0));
  if (current_size) strcat(sizebuf, "</FONT>");
  current_size=nr;
  if (nr) {
    int l;
    strcat(sizebuf, "<FONT SIZE=\"");
    l=strlen(sizebuf);
    if (nr>0) sizebuf[l++]='+'; else sizebuf[l++]='-',nr=-nr;
    sizebuf[l++]=nr+'0';
    sizebuf[l++]='"';
    sizebuf[l++]='>';
    sizebuf[l]=0;
  }
  strcat(sizebuf, change_to_font(i));
  return sizebuf;
}

int asint=0;
int intresult=0;

#define SKIPEOL while (*c && *c++!='\n')

static int skip_escape=0;
static int single_escape=0;

static char *
scan_escape(char *c) {
    char *h=NULL;
    char b[5];
    INTDEF *intd;
    int exoutputp,exskipescape;
    int i,j;

    intresult=0;
    switch (*c) {
    case 'e': h="\\"; curpos++;break;
    case '0':
    case ' ': h="&nbsp;";curpos++; break;
    case '|': h=""; break;
    case '"': SKIPEOL; c--; h=""; break;
    case '$':
    if (argument) {
        c++;
        i=(*c -'1');
        if (!(h=argument[i])) h="";
    }
    break;
    case 'z':
    c++;
    if (*c=='\\') { c=scan_escape(c+1); c--;h=""; }
    else {
        b[0]=*c;
        b[1]=0;
        h="";
    }
    break;
    case 'k': c++; if (*c=='(') c+=2;
    case '^':
    case '!':
    case '%':
    case 'a':
    case 'd':
    case 'r':
    case 'u':
    case '\n':
    case '&': h=""; break;
    case '(':
    c++;
    i= c[0]*256+c[1];
    c++;
    h = expand_char(i);
    break;
    case '*':
    c++;
    if (*c=='(') {
        c++;
        i= c[0]*256+c[1];
        c++;
    } else
        i= *c *256+' ';
    h = expand_string(i);
    break;
    case 'f':
    c++;
    if (*c=='\\') {
        c++;
        c=scan_escape(c);
        c--;
        i=intresult;
    } else     if (*c != '(')
        i=*c;
    else {
        c++;
        i=c[0]*256+c[1];
        c++;
    }
    if (!skip_escape) h=change_to_font(i); else h="";
    break;
    case 's':
    c++;
    j=0;i=0;
    if (*c=='-') {j= -1; c++;} else if (*c=='+') {j=1; c++;}
    if (*c=='0') c++; else if (*c=='\\') {
        c++;
        c=scan_escape(c);
        i=intresult; if (!j) j=1;
    } else
        while (isdigit(*c) && (!i || (!j && i<4))) i=i*10+(*c++)-'0';
    if (!j) { j=1; if (i) i=i-10; }
    if (!skip_escape) h=change_to_size(i*j); else h="";
    c--;
    break;
    case 'n':
    c++;
    j=0;
    switch (*c) {
    case '+': j=1; c++; break;
    case '-': j=-1; c++; break;
    default: break;
    }
    if (*c=='(') {
        c++;
        i=V(c[0],c[1]);
        c=c+1;
    } else {
        i=V(c[0],' ');
    }
    intd=intdef;
    while (intd && intd->nr!=i) intd=intd->next;
    if (intd) {
        intd->val=intd->val+j*intd->incr;
        intresult=intd->val;
    } else {
        switch (i) {
        case V('.','s'): intresult=current_size; break;
        case V('.','f'): intresult=current_font; break;
        default: intresult=0; break;
        }
    }
    h="";
    break;
    case 'w':
    c++;
    i=*c;
    c++;
    exoutputp=output_possible;
    exskipescape=skip_escape;
    output_possible=0;
    skip_escape=1;
    j=0;
    while (*c!=i) {
        j++;
        if (*c==escapesym) c=scan_escape(c+1); else c++;
    }
    output_possible=exoutputp;
    skip_escape=exskipescape;
    intresult=j;
    break;
    case 'l': h="<HR>"; curpos=0;
    case 'b':
    case 'v':
    case 'x':
    case 'o':
    case 'L':
    case 'h':
    c++;
    i=*c;
    c++;
    exoutputp=output_possible;
    exskipescape=skip_escape;
    output_possible=0;
    skip_escape=1;
    while (*c != i)
        if (*c==escapesym) c=scan_escape(c+1);
        else c++;
    output_possible=exoutputp;
    skip_escape=exskipescape;
    break;
    case 'c': no_newline_output=1; break;
    case '{': newline_for_fun++; h="";break;
    case '}': if (newline_for_fun) newline_for_fun--; h="";break;
    case 'p': h="<BR>\n";curpos=0; break;
    case 't': h="\t";curpos=(curpos+8)&0xfff8; break;
    case '<': h="&lt;";curpos++; break;
    case '>': h="&gt;";curpos++; break;
    case '\\': if (single_escape) { c--; break;}
    default: b[0]=*c; b[1]=0; h=b; curpos++; break;
    }
    c++;
    if (!skip_escape) out_html(h);
    return c;
}

typedef struct TABLEITEM TABLEITEM;

struct TABLEITEM {
    char *contents;
    int size,align,valign,colspan,rowspan,font,vleft,vright,space,width;
    TABLEITEM *next;
};

static TABLEITEM emptyfield = {NULL,0,0,0,1,1,0,0,0,0,0,NULL};
typedef struct TABLEROW TABLEROW;

struct TABLEROW {
    TABLEITEM *first;
    TABLEROW *prev, *next;
};

static char *tableopt[]= { "center", "expand", "box", "allbox", "doublebox",
               "tab", "linesize", "delim", NULL };
static int tableoptl[] = { 6,6,3,6,9,3,8,5,0};


static void clear_table(TABLEROW *table)
{
    TABLEROW *tr1,*tr2;
    TABLEITEM *ti1,*ti2;

    tr1=table;
    while (tr1->prev) tr1=tr1->prev;
    while (tr1) {
    ti1=tr1->first;
    while (ti1) {
        ti2=ti1->next;
        if (ti1->contents) free(ti1->contents);
        free(ti1);
        ti1=ti2;
    }
    tr2=tr1;
    tr1=tr1->next;
    free(tr2);
    }
}

char *scan_expression(char *c, int *result);

static char *scan_format(char *c, TABLEROW **result, int *maxcol)
{
    TABLEROW *layout, *currow;
    TABLEITEM *curfield;
    int i,j;
    if (*result) {
    clear_table(*result);
    }
    layout= currow=(TABLEROW*) xmalloc(sizeof(TABLEROW));
    currow->next=currow->prev=NULL;
    currow->first=curfield=(TABLEITEM*) xmalloc(sizeof(TABLEITEM));
    *curfield=emptyfield;
    while (*c && *c!='.') {
    switch (*c) {
    case 'C': case 'c': case 'N': case 'n':
    case 'R': case 'r': case 'A': case 'a':
    case 'L': case 'l': case 'S': case 's':
    case '^': case '_':
        if (curfield->align) {
        curfield->next=(TABLEITEM*)xmalloc(sizeof(TABLEITEM));
        curfield=curfield->next;
        *curfield=emptyfield;
        }
        curfield->align=toupper(*c);
        c++;
        break;
    case 'i': case 'I': case 'B': case 'b':
        curfield->font = toupper(*c);
        c++;
        break;
    case 'f': case 'F':
        c++;
        curfield->font = toupper(*c);
        c++;
        if (!isspace(*c)) c++;
        break;
    case 't': case 'T': curfield->valign='t'; c++; break;
    case 'p': case 'P':
        c++;
        i=j=0;
        if (*c=='+') { j=1; c++; }
        if (*c=='-') { j=-1; c++; }
        while (isdigit(*c)) i=i*10+(*c++)-'0';
        if (j) curfield->size= i*j; else curfield->size=j-10;
        break;
    case 'v': case 'V':
    case 'w': case 'W':
//        c=scan_expression(c+2,&curfield->width);
             c++;
         if (*c == '(') {
            c=scan_expression(c+1,&curfield->width);
         } else {
             i=0;
             while (isdigit(*c)) i=i*10+(*c++)-'0';
            curfield->width=i;
         }
        break;
    case '|':
        if (curfield->align) curfield->vleft++;
        else curfield->vright++;
        c++;
        break;
    case 'e': case 'E':
        c++;
        break;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        i=0;
        while (isdigit(*c)) i=i*10+(*c++)-'0';
        curfield->space=i;
        break;
    case ',': case '\n':
        currow->next=(TABLEROW*)xmalloc(sizeof(TABLEROW));
        currow->next->prev=currow;
        currow=currow->next;
        currow->next=NULL;
        curfield=currow->first=(TABLEITEM*)xmalloc(sizeof(TABLEITEM));
        *curfield=emptyfield;
        c++;
        break;
    default:
        c++;
        break;
    }
    }
    if (*c=='.') while (*c++!='\n');
    *maxcol=0;
    currow=layout;
    while (currow) {
    curfield=layout->first;
    i=0;
    while (curfield) {
        i++;
        curfield=curfield->next;
    }
    if (i>*maxcol) *maxcol=i;
    currow=currow->next;
    }
    *result=layout;
    return c;
}

static TABLEROW *
next_row(TABLEROW *tr)
{
    if (tr->next) {
    tr=tr->next;
    if (!tr->next) next_row(tr);
    return tr;
    } else {
    TABLEITEM *ti, *ti2;
    tr->next=(TABLEROW*)xmalloc(sizeof(TABLEROW));
    tr->next->prev=tr;
    ti=tr->first;
    tr=tr->next;
    tr->next=NULL;
    if (ti) tr->first=ti2=(TABLEITEM*) xmalloc(sizeof(TABLEITEM));
    else tr->first=ti2=NULL;
    while (ti!=ti2) {
        *ti2=*ti;
        ti2->contents=NULL;
        if ((ti=ti->next)) {
        ti2->next=(TABLEITEM*) xmalloc(sizeof(TABLEITEM));
        }
        ti2=ti2->next;
    }
    return tr;
    }
}

char itemreset[20]="\\fR\\s0";

static char *
scan_table(char *c) {
    char *h;
    char *g;
    int center=0, expand=0, box=0, border=0, linesize=1;
    int i,j,maxcol=0, finished=0;
    int oldfont, oldsize,oldfillout;
    char itemsep='\t';
    TABLEROW *layout=NULL, *currow;
    TABLEITEM *curfield;
    while (*c++!='\n');        /* skip TS */
    h=c;
    if (*h=='.') return c-1;
    oldfont=current_font;
    oldsize=current_size;
    oldfillout=fillout;
    out_html(change_to_font(0));
    out_html(change_to_size(0));
    if (!fillout) {
    fillout=1;
    out_html("</PRE>");
    }
    while (*h && *h!='\n') h++;
    if (h[-1]==';') {
    /* scan table options */
    while (c<h) {
        while (isspace(*c)) c++;
        for (i=0; tableopt[i] && strncmp(tableopt[i],c,tableoptl[i]);i++);
        c=c+tableoptl[i];
        switch (i) {
        case 0: center=1; break;
        case 1: expand=1; break;
        case 2: box=1; break;
        case 3: border=1; break;
        case 4: box=2; break;
        case 5: while (*c++!='('); itemsep=*c++; break;
        case 6: while (*c++!='('); linesize=0;
        while (isdigit(*c)) linesize=linesize*10+(*c++)-'0';
        break;
        case 7: while (*c!=')') c++;
        default: break;
        }
        c++;
    }
    c=h+1;
    }
    /* scan layout */
    c=scan_format(c,&layout, &maxcol);
    currow=layout;
    next_row(currow);
    curfield=layout->first;
    i=0;
    while (!finished && *c) {
    /* search item */
    h=c;
    if ((*c=='_' || *c=='=') && (c[1]==itemsep || c[1]=='\n')) {
        if (c[-1]=='\n' && c[1]=='\n') {
        if (currow->prev) {
            currow->prev->next=(TABLEROW*) xmalloc(sizeof(TABLEROW));
            currow->prev->next->next=currow;
            currow->prev->next->prev=currow->prev;
            currow->prev=currow->prev->next;
        } else {
            currow->prev=layout=(TABLEROW*) xmalloc(sizeof(TABLEROW));
            currow->prev->prev=NULL;
            currow->prev->next=currow;
        }
        curfield=currow->prev->first=
            (TABLEITEM*) xmalloc(sizeof(TABLEITEM));
        *curfield=emptyfield;
        curfield->align=*c;
        curfield->colspan=maxcol;
        curfield=currow->first;
        c=c+2;
        } else {
        if (curfield) {
            curfield->align=*c;
            do {
            curfield=curfield->next;
            } while (curfield && curfield->align=='S');
        }
        if (c[1]=='\n') {
            currow=next_row(currow);
            curfield=currow->first;
        }
        c=c+2;
        }
    } else if (*c=='T' && c[1]=='{') {
        h=c+2;
        c=strstr(h,"\nT}");
        c++;
        *c=0;
        g=NULL;
        scan_troff(h, 0, &g);
        scan_troff(itemreset, 0, &g);
        *c='T';
        c+=3;
        if (curfield) {
        curfield->contents=g;
        do {
            curfield=curfield->next;
        } while (curfield && curfield->align=='S');
        } else
        if (g) free(g);
        if (c[-1]=='\n') {
        currow=next_row(currow);
        curfield=currow->first;
        }
    } else if (*c=='.' && c[1]=='T' && c[2]=='&' && c[-1]=='\n') {
        TABLEROW *hr;
        while (*c++!='\n');
        hr=currow;
        currow=currow->prev;
        hr->prev=NULL;
        c=scan_format(c,&hr, &i);
        hr->prev=currow;
        currow->next=hr;
        currow=hr;
        next_row(currow);
        curfield=currow->first;
    } else if (*c=='.' && c[1]=='T' && c[2]=='E' && c[-1]=='\n') {
        finished=1;
        while (*c++!='\n');
        if (currow->prev)
        currow->prev->next=NULL;
        currow->prev=NULL;
        clear_table(currow);
    } else if (*c=='.' && c[-1]=='\n' && !isdigit(c[1])) {
        /* skip troff request inside table (usually only .sp ) */
        while (*c++!='\n');
    } else {
        h=c;
        while (*c && (*c!=itemsep || c[-1]=='\\') &&
           (*c!='\n' || c[-1]=='\\')) c++;
        i=0;
        if (*c==itemsep) {i=1; *c='\n'; }
        if (h[0]=='\\' && h[2]=='\n' &&
        (h[1]=='_' || h[1]=='^')) {
        if (curfield) {
            curfield->align=h[1];
            do {
            curfield=curfield->next;
            } while (curfield && curfield->align=='S');
        }
        h=h+3;
        } else {
        g=NULL;
        h=scan_troff(h,1,&g);
        scan_troff(itemreset,0,&g);
        if (curfield) {
            curfield->contents=g;
            do {
            curfield=curfield->next;
            } while (curfield && curfield->align=='S');
        } else if (g) free(g);
        }
        if (i) *c=itemsep;
        c=h;
        if (c[-1]=='\n') {
        currow=next_row(currow);
        curfield=currow->first;
        }
    }
    }
    /* calculate colspan and rowspan */
#if 0
    currow=layout;
    while (currow->next) currow=currow->next;
    while (currow) {
    TABLEITEM *ti, *ti1=NULL, *ti2=NULL;
    ti=currow->first;
    if (currow->prev) ti1=currow->prev->first;
    while (ti) {
        switch (ti->align) {
        case 'S':
        if (ti2) {
            ti2->colspan++;
            if (ti2->rowspan<ti->rowspan) ti2->rowspan=ti->rowspan;
        }
        break;
        case '^':
        if (ti1) ti1->rowspan++;
        default:
        if (!ti2) ti2=ti;
        else {
            do {
            ti2=ti2->next;
            } while (ti2 && curfield->align=='S');
        }
        break;
        }
        ti=ti->next;
        if (ti1) ti1=ti1->next;
    }
    currow=currow->prev;
    }
#endif
    /* produce html output */
    if (center) out_html("<CENTER>");
    if (box==2) out_html("<TABLE BORDER><TR><TD>");
    out_html("<TABLE");
    if (box || border) {
    out_html(" BORDER");
    if (!border) out_html("><TR><TD><TABLE");
    if (expand) out_html(" WIDTH=100%");
    }
    out_html(">\n");
    currow=layout;
    while (currow) {
    j=0;
    out_html("<TR VALIGN=top>");
    curfield=currow->first;
    while (curfield) {
        if (curfield->align!='S' && curfield->align!='^') {
        out_html("<TD");
        switch (curfield->align) {
        case 'N':
            curfield->space+=4;
        case 'R':
            out_html(" ALIGN=right");
            break;
        case 'C':
            out_html(" ALIGN=center");
        default:
            break;
        }
        if (!curfield->valign && curfield->rowspan>1)
            out_html(" VALIGN=center");
        if (curfield->colspan>1) {
            char buf[5];
            out_html(" COLSPAN=");
            sprintf(buf, "%i", curfield->colspan);
            out_html(buf);
        }
        if (curfield->rowspan>1) {
            char buf[5];
            out_html(" ROWSPAN=");
            sprintf(buf, "%i", curfield->rowspan);
            out_html(buf);
        }
        j=j+curfield->colspan;
        out_html(">");
        if (curfield->size) out_html(change_to_size(curfield->size));
        if (curfield->font) out_html(change_to_font(curfield->font));
        switch (curfield->align) {
        case '=': out_html("<HR><HR>"); break;
        case '_': out_html("<HR>"); break;
        default:
            if (curfield->contents) out_html(curfield->contents);
            break;
        }
        if (curfield->space)
            for (i=0; i<curfield->space;i++) out_html("&nbsp;");
        if (curfield->font) out_html(change_to_font(0));
        if (curfield->size) out_html(change_to_size(0));
        if (j>=maxcol && curfield->align>'@' && curfield->align!='_')
            out_html("<BR>");
        out_html("</TD>");
        }
        curfield=curfield->next;
    }
    out_html("</TR>\n");
    currow=currow->next;
    }
    if (box && !border) out_html("</TABLE>");
    out_html("</TABLE>");
    if (box==2) out_html("</TABLE>");
    if (center) out_html("</CENTER>\n");
    else out_html("\n");
    if (!oldfillout) out_html("<PRE>");
    fillout=oldfillout;
    out_html(change_to_size(oldsize));
    out_html(change_to_font(oldfont));
    return c;
}

char *scan_expression(char *c, int *result) {
    int value=0,value2,sign=1,opex=0;
    char oper='c';

    if (*c=='!') {
    c=scan_expression(c+1, &value);
    value= (!value);
    } else if (*c=='n') {
    c++;
    value=nroff;
    } else if (*c=='t') {
    c++;
    value=1-nroff;
    } else if (*c=='\'' || *c=='"' || *c<' ' || (*c=='\\' && c[1]=='(')) {
    /* ?string1?string2?
    ** test if string1 equals string2.
    */
    char *st1=NULL, *st2=NULL, *h;
    char *tcmp=NULL;
    char sep;
    sep=*c;
    if (sep=='\\') {
        tcmp=c;
        c=c+3;
    }
    c++;
    h=c;
    while (*c!= sep && (!tcmp || strncmp(c,tcmp,4))) c++;
    *c='\n';
    scan_troff(h, 1, &st1);
    *c=sep;
    if (tcmp) c=c+3;
    c++;
    h=c;
    while (*c!=sep && (!tcmp || strncmp(c,tcmp,4))) c++;
    *c='\n';
    scan_troff(h,1,&st2);
    *c=sep;
    if (!st1 && !st2) value=1;
    else if (!st1 || !st2) value=0;
    else value=(!strcmp(st1, st2));
    if (st1) free(st1);
    if (st2) free(st2);
    if (tcmp) c=c+3;
    c++;
    } else {
    while (*c && !isspace(*c) && *c!=')') {
        opex=0;
        switch (*c) {
        case '(':
        c=scan_expression(c+1, &value2);
        value2=sign*value2;
        opex=1;
        break;
        case '.':
        case '0': case '1':
        case '2': case '3':
        case '4': case '5':
        case '6': case '7':
        case '8': case '9': {
        int num=0,denum=1;
        value2=0;
        while (isdigit(*c)) value2=value2*10+((*c++)-'0');
        if (*c=='.') {
            c++;
            while (isdigit(*c)) {
            num=num*10+((*c++)-'0');
            denum=denum*10;
            }
        }
        if (isalpha(*c)) {
            /* scale indicator */
            switch (*c) {
            case 'i': /* inch -> 10pt */
            value2=value2*10+(num*10+denum/2)/denum;
            num=0;
            break;
            default:
            break;
            }
            c++;
        }
        value2=value2+(num+denum/2)/denum;
        value2=sign*value2;
        opex=1;
        break;
        }
        case '\\':
        c=scan_escape(c+1);
        value2=intresult*sign;
        if (isalpha(*c)) c++; /* scale indicator */
        opex=1;
        break;
        case '-':
        if (oper) { sign=-1; c++; break; }
        case '>':
        case '<':
        case '+':
        case '/':
        case '*':
        case '%':
        case '&':
        case '=':
        case ':':
        if (c[1]=='=') oper=(*c++) +16; else oper=*c;
        c++;
        break;
        default: c++; break;
        }
        if (opex) {
        sign=1;
        switch (oper) {
        case 'c': value=value2; break;
        case '-': value=value-value2; break;
        case '+': value=value+value2; break;
        case '*': value=value*value2; break;
        case '/': if (value2) value=value/value2; break;
        case '%': if (value2) value=value%value2; break;
        case '<': value=(value<value2); break;
        case '>': value=(value>value2); break;
        case '>'+16: value=(value>=value2); break;
        case '<'+16: value=(value<=value2); break;
        case '=': case '='+16: value=(value==value2); break;
        case '&': value = (value && value2); break;
        case ':': value = (value || value2); break;
        default: fprintf(stderr,
                 "man2html: Unknown operator %c.\n", oper);
        }
        oper=0;
        }
    }
    if (*c==')') c++;
    }
    *result=value;
    return c;
}

static void
trans_char(char *c, char s, char t) {
    char *sl = c;
    int slash = 0;

    while (*sl && (*sl != '\n' || slash)) {
        if (!slash) {
            if (*sl == escapesym)
                slash = 1;
            else if (*sl == s)
                *sl = t;
        } else
            slash = 0;
        sl++;
    }
}

/*
 * Read STR until end-of-line (not preceded by \).
 * Find whitespace separated words, and store starts in WORDS of lth MAXN.
 * Return number of words in N.
 * Replace each end-of-word by the character EOW (usually \n or 0).
 * Return pointer to last char seen (either \n or 0).
 *
 * A part \"... is skipped.
 * Quotes not preceded by \ are replaced by \a.
 */
static char *
fill_words(char *str, char *words[], int maxn, int *n, char eow) {
    char *s = str;
    int backslash = 0;
    int skipspace = 0;    /* 1 if space is not end-of-word */

    *n = 0;
    words[*n] = s;
    while (*s && (*s != '\n' || backslash)) {
        if (!backslash) {
            if (*s == '"') {
                *s = '\a';
                skipspace = !skipspace;
            } else if (*s == escapesym) {
                backslash = 1;
            } else if ((*s == ' ' || *s == '\t') && !skipspace) {
                *s = eow;
                if (words[*n] != s && *n < maxn-1)
                    (*n)++;
                words[*n] = s+1;
            }
        } else {
            if (*s == '"') {
                s--;
                *s = eow;
                if (words[*n] != s && *n < maxn-1)
                    (*n)++;
                s++;
                while (*s && *s != '\n') s++;
                words[*n] = s;
                s--;
            }
            backslash = 0;
        }
        s++;
    }
    if (s != words[*n])
        (*n)++;
    return s;
}


char *section_list[] = {
    "1", "User Commands ",
    "1C", "User Commands",
    "1G", "User Commands",
    "1S", "User Commands",
    "1V", "User Commands ",
    "2", "System Calls",
    "2V", "System Calls",
    "3", "C Library Functions",
    "3C", "Compatibility Functions",
    "3F", "Fortran Library Routines",
    "3K", "Kernel VM Library Functions",
    "3L", "Lightweight Processes Library",
    "3M", "Mathematical Library",
    "3N", "Network Functions",
    "3R", "RPC Services Library",
    "3S", "Standard I/O Functions",
    "3V", "C Library Functions",
    "3X", "Miscellaneous Library Functions",
    "4", "Devices and Network Interfaces",
    "4F", "Protocol Families",
    "4I", "Devices and Network Interfaces",
    "4M", "Devices and Network Interfaces",
    "4N", "Devices and Network Interfaces",
    "4P", "Protocols",
    "4S", "Devices and Network Interfaces",
    "4V", "Devices and Network Interfaces",
    "5", "File Formats",
    "5V", "File Formats",
    "6", "Games and Demos",
    "7", "Environments, Tables, and Troff Macros",
    "7V", "Environments, Tables, and Troff Macros",
    "8", "Maintenance Commands",
    "8C", "Maintenance Commands",
    "8S", "Maintenance Commands",
    "8V", "Maintenance Commands",
    "L", "Local Commands",
/* for Solaris:
    "1", "User Commands",
    "1B", "SunOS/BSD Compatibility Package Commands",
    "1b", "SunOS/BSD Compatibility Package Commands",
    "1C", "Communication Commands ",
    "1c", "Communication Commands",
    "1F", "FMLI Commands ",
    "1f", "FMLI Commands",
    "1G", "Graphics and CAD Commands ",
    "1g", "Graphics and CAD Commands ",
    "1M", "Maintenance Commands",
    "1m", "Maintenance Commands",
    "1S", "SunOS Specific Commands",
    "1s", "SunOS Specific Commands",
    "2", "System Calls",
    "3", "C Library Functions",
    "3B", "SunOS/BSD Compatibility Library Functions",
    "3b", "SunOS/BSD Compatibility Library Functions",
    "3C", "C Library Functions",
    "3c", "C Library Functions",
    "3E", "C Library Functions",
    "3e", "C Library Functions",
    "3F", "Fortran Library Routines",
    "3f", "Fortran Library Routines",
    "3G", "C Library Functions",
    "3g", "C Library Functions",
    "3I", "Wide Character Functions",
    "3i", "Wide Character Functions",
    "3K", "Kernel VM Library Functions",
    "3k", "Kernel VM Library Functions",
    "3L", "Lightweight Processes Library",
    "3l", "Lightweight Processes Library",
    "3M", "Mathematical Library",
    "3m", "Mathematical Library",
    "3N", "Network Functions",
    "3n", "Network Functions",
    "3R", "Realtime Library",
    "3r", "Realtime Library",
    "3S", "Standard I/O Functions",
    "3s", "Standard I/O Functions",
    "3T", "Threads Library",
    "3t", "Threads Library",
    "3W", "C Library Functions",
    "3w", "C Library Functions",
    "3X", "Miscellaneous Library Functions",
    "3x", "Miscellaneous Library Functions",
    "4", "File Formats",
    "4B", "SunOS/BSD Compatibility Package File Formats",
    "4b", "SunOS/BSD Compatibility Package File Formats",
    "5", "Headers, Tables, and Macros",
    "6", "Games and Demos",
    "7", "Special Files",
    "7B", "SunOS/BSD Compatibility Special Files",
    "7b", "SunOS/BSD Compatibility Special Files",
    "8", "Maintenance Procedures",
    "8C", "Maintenance Procedures",
    "8c", "Maintenance Procedures",
    "8S", "Maintenance Procedures",
    "8s", "Maintenance Procedures",
    "9", "DDI and DKI",
    "9E", "DDI and DKI Driver Entry Points",
    "9e", "DDI and DKI Driver Entry Points",
    "9F", "DDI and DKI Kernel Functions",
    "9f", "DDI and DKI Kernel Functions",
    "9S", "DDI and DKI Data Structures",
    "9s", "DDI and DKI Data Structures",
    "L", "Local Commands",
*/
    NULL, "Misc. Reference Manual Pages",
    NULL, NULL
};

static char *
section_name(char *c)
{
    int i=0;

    if (!c) return "";
    while (section_list[i] && strcmp(c,section_list[i])) i=i+2;
    if (section_list[i+1]) return section_list[i+1];
    else return c;
}

int manidxlen = 0;
char *manidx = NULL;
int subs = 0;
int mip = 0;    /* current offset in manidx[] */
char label[5]="lbAA";

static void
manidx_need(int m) {
    if (mip + m >= manidxlen) {
        manidxlen += 10000;
        manidx = xrealloc(manidx, manidxlen);
    }
}

static void
add_to_index(int level, char *item)
{
    char *c = NULL;

    label[3]++;
    if (label[3]>'Z') {
    label[3]='A';
    label[2]++;
    }

    if (level != subs) {
    manidx_need(6);
    if (subs) {
        strcpy(manidx+mip, "</DL>\n");
        mip += 6;
    } else {
        strcpy(manidx+mip, "<DL>\n");
        mip += 5;
    }
    }
    subs = level;

    scan_troff(item, 1, &c);
    manidx_need(100 + strlen(c));
    sprintf(manidx+mip, "<DT><A HREF=\"#%s\">%s</A><DD>\n", label, c);
    if (c) free(c);
    while (manidx[mip]) mip++;
}

static char *
skip_till_newline(char *c)
{
    int lvl=0;

    while (*c && (*c!='\n' || lvl>0)) {
    if (*c=='\\') {
        c++;
        if (*c=='}') lvl--; else if (*c=='{') lvl++;
    }
    c++;
    }
    c++;
    if (lvl<0 && newline_for_fun) {
    newline_for_fun = newline_for_fun+lvl;
    if (newline_for_fun<0) newline_for_fun=0;
    }
    return c;
}

int ifelseval=0;

static char *
scan_request(char *c) {
                                  /* BSD Mandoc stuff - by Michael Hamilton */
    static int mandoc_synopsis=0; /* True if we are in the synopsis section */
    static int mandoc_command=0;  /* True if this is mandoc page */
    static int mandoc_bd_options; /* Only copes with non-nested Bd's */
    static int inXo=0;

    int i,j,mode = 0;
    char *h;
    char *wordlist[20];
    int words;
    char *sl;
    STRDEF *owndef;

    while (*c == ' ' || *c == '\t')
        c++;
    if (c[0] == '\n')
        return c+1;
    if (c[1] == '\n')
        j = 1;
    else
        j = 2;
    while (c[j] == ' ' || c[j] == '\t')
        j++;
    if (c[0] == escapesym) {
    /* some pages use .\" .\$1 .\} */
    /* .\$1 is too difficult/stupid */
    if (c[1] == '$')
        c = skip_till_newline(c);
    else
        c = scan_escape(c+1);
    } else {
    i=V(c[0],c[1]);
    switch (i) {
    case V('a','b'):
        h=c+j;
        while (*h && *h !='\n') h++;
        *h=0;
        if (scaninbuff && buffpos) {
        buffer[buffpos]=0;
        fprintf(out, "%s\n", buffer);
        }
        fprintf(stderr, "%s\n", c+2);        /* XXX */
        exit(0);
        break;
    case V('d','i'):
        {
        STRDEF *de;
        c=c+j;
        i=V(c[0],c[1]);
        if (*c == '\n') { c++;break; }
        while (*c && *c!='\n') c++;
        c++;
        h=c;
        while (*c && strncmp(c,".di",3)) while (*c && *c++!='\n');
        *c=0;
        de=strdef;
        while (de && de->nr !=i) de=de->next;
        if (!de) {
            de=(STRDEF*) xmalloc(sizeof(STRDEF));
            de->nr=i;
            de->slen=0;
            de->next=strdef;
            de->st=NULL;
            strdef=de;
        } else {
            if (de->st) free(de->st);
            de->slen=0;
            de->st=NULL;
        }
        scan_troff(h,0,&de->st);
        *c='.';
        while (*c && *c++!='\n');
        break;
        }
    case V('d','s'):
        mode=1;
    case V('a','s'):
        {
        STRDEF *de;
        int oldcurpos=curpos;
        c=c+j;
        while (*c == ' ') c++;
        i=V(c[0],c[1]);
        j=0;
        while (c[j] && c[j]!='\n') j++;
        if (j<3) { c=c+j; break; }
        if (c[1] == ' ') c=c+1; else c=c+2;
        while (isspace(*c)) c++;
        if (*c == '"') c++;
        de=strdef;
        while (de && de->nr != i) de=de->next;
        single_escape=1;
        curpos=0;
        if (!de) {
            char *h;
            de=(STRDEF*) xmalloc(sizeof(STRDEF));
            de->nr=i;
            de->slen=0;
            de->next=strdef;
            de->st=NULL;
            strdef=de;
            h=NULL;
            c=scan_troff(c, 1, &h);
            de->st=h;
            de->slen=curpos;
        } else {
            if (mode) {        /* .ds */
            char *h=NULL;
            c=scan_troff(c, 1, &h);
            free(de->st);    /* segfault XXX */
            de->slen=curpos;
            de->st=h;
            } else {        /* .as */
            c=scan_troff(c,1,&de->st);     /* XXX */
            de->slen+=curpos;
            }
        }
        single_escape=0;
        curpos=oldcurpos;
        }
        break;
    case V('b','r'):
        if (still_dd) out_html("<DD>");
        else out_html("<BR>\n");
        curpos=0;
        c=c+j;
        if (c[0] == escapesym) { c=scan_escape(c+1); }
        c=skip_till_newline(c);break;
    case V('c','2'):
        c=c+j;
        if (*c!='\n') { nobreaksym=*c; }
        else nobreaksym='\'';
        c=skip_till_newline(c);
        break;
    case V('c','c'):
        c=c+j;
        if (*c!='\n') { controlsym=*c; }
        else controlsym='.';
        c=skip_till_newline(c);
        break;
    case V('c','e'):
        c=c+j;
        if (*c == '\n') { i=1; }
        else {
        i=0;
        while ('0'<=*c && *c<='9') {
            i=i*10+*c-'0';
            c++;
        }
        }
        c=skip_till_newline(c);
        /* center next i lines */
        if (i>0) {
        out_html("<CENTER>\n");
        while (i && *c) {
            char *line=NULL;
            c=scan_troff(c,1, &line);
            if (line && strncmp(line, "<BR>", 4)) {
            out_html(line);
            out_html("<BR>\n");
            i--;
            }
        }
        out_html("</CENTER>\n");
        curpos=0;
        }
        break;
    case V('e','c'):
        c=c+j;
        if (*c!='\n') { escapesym=*c; }
        else escapesym='\\';
        break;
        c=skip_till_newline(c);
    case V('e','o'):
        escapesym=0;
        c=skip_till_newline(c);
        break;
    case V('e','x'):
        exit(0);
        break;
    case V('f','c'):
        c=c+j;
        if  (*c == '\n') {
        fieldsym=padsym=0;
        } else {
        fieldsym=c[0];
        padsym=c[1];
        }
        c=skip_till_newline(c);
        break;
    case V('f','i'):
        if (!fillout) {
        out_html(change_to_font(0));
        out_html(change_to_size('0'));
        out_html("</PRE>\n");
        }
        curpos=0;
        fillout=1;
        c=skip_till_newline(c);
        break;
    case V('f','t'):
        c=c+j;
        if (*c == '\n') {
        out_html(change_to_font(0));
        } else {
        if (*c == escapesym) {
            int fn;
            c=scan_expression(c, &fn);
            c--;
            out_html(change_to_font(fn));
        } else {
            out_html(change_to_font(*c));
            c++;
        }
        }
        c=skip_till_newline(c);
        break;
    case V('e','l'):
        /* .el anything : else part of if else */
        if (ifelseval) {
        c=c+j;
        c[-1]='\n';
        c=scan_troff(c,1,NULL);
        } else
        c=skip_till_newline(c+j);
        break;
    case V('i','e'):
        /* .ie c anything : then part of if else */
    case V('i','f'):
        /* .if c anything
         * .if !c anything
         * .if N anything
         * .if !N anything
         * .if 'string1'string2' anything
         * .if !'string1'string2' anything
         */
        c=c+j;
        c=scan_expression(c, &i);
        ifelseval=!i;
        if (i) {
        *c='\n';
        c++;
        c=scan_troff(c,1,NULL);
        } else
        c=skip_till_newline(c);
        break;
    case V('i','g'):    /* .ig: ignore until .. */
        {
        char *endwith="..\n";
        i=3;
        c=c+j;
        while (*c == ' ') c++;
        if (*c == escapesym && c[1] == '"')
            while (*c != '\n') c++;
        if (*c!='\n') {    /* .ig yy: ignore until .yy, then call .yy */
            endwith=c-1;i=1;
            c[-1]='.';
            while (*c && *c!='\n') c++,i++;
        }
        c++;
        while (*c && strncmp(c,endwith,i))
             while (*c && *c++!='\n');
        while (*c && *c++!='\n');
        break;
        }
    case V('n','f'):
        if (fillout) {
        out_html(change_to_font(0));
        out_html(change_to_size('0'));
        out_html("<PRE>\n");
        }
        curpos=0;
        fillout=0;
        c=skip_till_newline(c);
        break;
    case V('p','s'):
        c=c+j;
        if (*c == '\n') {
        out_html(change_to_size('0'));
        } else {
        j=0;i=0;
        if (*c == '-') { j= -1;c++; } else if (*c == '+') { j=1;c++;}
        c=scan_expression(c, &i);
        if (!j) { j=1; if (i>5) i=i-10; }
        out_html(change_to_size(i*j));
        }
        c=skip_till_newline(c);
        break;
    case V('s','p'):
        c=c+j;
        if (fillout) out_html("<P>"); else {
        out_html(NEWLINE);
        NEWLINE[0]='\n';
        }
        curpos=0;
        c=skip_till_newline(c);
        break;
    case V('s','o'):
        {
        FILE *f;
        int l; char *buf;
        char *name = NULL;

        curpos=0;
        c += j;            /* skip .so part and whitespace */
        if (*c == '/') {
            h = c;
        } else {        /* .so man3/cpow.3 -> ../man3/cpow.3 */
            h = c-3;
            h[0] = '.';
            h[1] = '.';
            h[2] = '/';
        }
        while (*c != '\n') c++;
        while (c[-1] == ' ') c--;
        while (*c != '\n') *c++ = 0;
        *c = 0;
        scan_troff(h,1, &name);
        if (name[3] == '/') h=name+3; else h=name;
        l = 0;
#ifdef _KOLIBRI
        ksys_bdfe_t bdfe;
        _ksys_file_get_info(h, &bdfe);
        l = bdfe.size;
#else
        struct stat stbuf;
        if (stat(h, &stbuf)!=-1) l=stbuf.st_size;
#endif

        buf = (char*) xmalloc((l+4)*sizeof(char));
#if NOCGI
                if (!out_length) {
            char *t,*s;
            t=strrchr(fname, '/');
            if (!t) t=fname;
            //fprintf(stderr, "ln -s %s.html %s.html\n", h, t);
            s=strrchr(t, '.');if (!s) s=t;
            fprintf(out,"<HTML><HEAD><TITLE> Manpage of %s</TITLE>\n"
               "</HEAD><BODY>\n"
               "See the manpage for <A HREF=\"%s.html\">%s</A>.\n"
               "</BODY></HTML>\n",
               s, h, h);
        } else
#endif
                {
            /* this works alright, except for section 3 */
            if (!l || !(f = fopen(h,"r"))) {
             fprintf(stderr,
                "man2html: unable to open or read file %s\n", h);
             out_html("<BLOCKQUOTE>"
                  "man2html: unable to open or read file\n");
             out_html(h);
             out_html("</BLOCKQUOTE>\n");
            } else {
            i=fread(buf+1,1,l,f);
            fclose(f);
            buf[0]=buf[l]='\n';
            buf[l+1]=buf[l+2]=0;
            scan_troff(buf+1,0,NULL);
            }
            if (buf) free(buf);
        }
        *c++='\n';
        break;
        }
    case V('t','a'):
        c=c+j;
        j=0;
        while (*c!='\n') {
        sl=scan_expression(c, &tabstops[j]);
        if (*c == '-' || *c == '+') tabstops[j]+=tabstops[j-1];
        c=sl;
        while (*c == ' ' || *c == '\t') c++;
        if (j+1 < SIZE(tabstops))
            j++;
        }
        maxtstop=j;
        curpos=0;
        break;
    case V('t','i'):
#if 0
        dl_down();
#endif
        out_html("<BR>\n");
        c=c+j;
        c=scan_expression(c, &j);
        for (i=0; i<j; i++) out_html("&nbsp;");
        curpos=j;
        c=skip_till_newline(c);
        break;
    case V('t','m'):
        c=c+j;
        h=c;
        while (*c!='\n') c++;
        *c=0;
        fprintf(stderr,"%s\n", h);        /* XXX */
        *c='\n';
        break;
    case V('B',' '):
    case V('B','\n'):
    case V('I',' '):
    case V('I','\n'):
            /* parse one line in a certain font */
        out_html(change_to_font(*c));
        trans_char(c, '"', '\a');
        c=c+j;
        if (*c == '\n') c++;
        c=scan_troff(c, 1, NULL);
        out_html(change_to_font('R'));
        out_html(NEWLINE);
        if (fillout) curpos++; else curpos=0;
        break;
    case V('O','P'):  /* groff manpages use this construction */
            /* .OP a b : [ <B>a</B> <I>b</I> ] */
        mode=1;
        c[0]='B'; c[1]='I';
        out_html(change_to_font('R'));
        out_html("[");
        curpos++;
    case V('B','R'):
    case V('B','I'):
    case V('I','B'):
    case V('I','R'):
    case V('R','B'):
    case V('R','I'):
        {
        char font[2];
        font[0] = c[0]; font[1] = c[1];
        c = c+j;
        if (*c == '\n') c++;
        sl = fill_words(c, wordlist, SIZE(wordlist), &words, '\n');
        c = sl+1;
        /* .BR name (section)
        ** indicates a link. It will be added in the output routine.
        */
        for (i=0; i<words; i++) {
            if (mode) { out_html(" "); curpos++; }
            wordlist[i][-1]=' ';
            out_html(change_to_font(font[i&1]));
            scan_troff(wordlist[i],1,NULL);
        }
        out_html(change_to_font('R'));
        if (mode) { out_html(" ]"); curpos++;}
        out_html(NEWLINE); if (!fillout) curpos=0; else curpos++;
        }
        break;
    case V('D','T'):
        maxtstop = SIZE(tabstops);
        for (j=0; j<maxtstop; j++)
        tabstops[j]=(j+1)*8;
        c=skip_till_newline(c); break;
    case V('I','P'):
        sl = fill_words(c+j, wordlist, SIZE(wordlist), &words, '\n');
        c = sl+1;
        dl_begin();
            if (words) {
        scan_troff(wordlist[0], 1,NULL);
        }
        out_html("<DD>");
        curpos = 0;
        break;
    case V('T','P'):
        dl_begin();
        c=skip_till_newline(c);
        /* somewhere a definition ends with '.TP' */
        if (!*c) still_dd=1; else {
        c=scan_troff(c,1,NULL);
        out_html("<DD>");
        }
        curpos=0;
        break;
    case V('I','X'):
            /* general index */
        sl = fill_words(c+j, wordlist, SIZE(wordlist), &words, '\n');
        c = sl+1;
        j = 4;
        while (idxlabel[j] == 'Z') idxlabel[j--]='A';
        idxlabel[j]++;
#ifdef MAKEINDEX
        if (idxfile) {
         fprintf(idxfile, "%s@%s@", fname, idxlabel);
         for (j=0; j<words; j++) {
              h=NULL;
              scan_troff(wordlist[j], 1, &h);
              fprintf(idxfile, "_\b@%s", h);
              free(h);
         }
         fprintf(idxfile,"\n");
        }
#endif
            out_html("<A NAME=\"");
        out_html(idxlabel);
        /* this will not work in mosaic (due to a bug).
        ** Adding '&nbsp;' between '>' and '<' solves it, but creates
        ** some space. A normal space does not work.
        */
        out_html("\"></A>");
        break;
    case V('L','P'):
    case V('P','P'):
        dl_end();
        if (fillout) out_html("<P>\n"); else {
        out_html(NEWLINE);
        NEWLINE[0]='\n';
        }
        curpos=0;
        c=skip_till_newline(c);
        break;
    case V('H','P'):
        dl_begin();
        still_dd=1;
        c=skip_till_newline(c);
        curpos=0;
        break;
    case V('P','D'):
         c=skip_till_newline(c);
         break;
    case V('R','s'):        /* BSD mandoc */
    case V('R','S'):
        sl = fill_words(c+j, wordlist, SIZE(wordlist), &words, '\n');
        j = 1;
        if (words>0) scan_expression(wordlist[0], &j);
        if (j>=0) {
        dl_newlevel();
        c=skip_till_newline(c);
        curpos=0;
        break;
        }
    case V('R','e'):        /* BSD mandoc */
    case V('R','E'):
        dl_endlevel();
        c=skip_till_newline(c);
        curpos=0;
        break;
    case V('S','B'):
        out_html(change_to_size(-1));
        out_html(change_to_font('B'));
        c=scan_troff(c+j, 1, NULL);
        out_html(change_to_font('R'));
        out_html(change_to_size('0'));
        break;
    case V('S','M'):
        c=c+j;
        if (*c == '\n') c++;
        out_html(change_to_size(-1));
        trans_char(c,'"','\a');
        c=scan_troff(c,1,NULL);
        out_html(change_to_size('0'));
        break;
    case V('S','s'):        /* BSD mandoc */
        mandoc_command = 1;
    case V('S','S'):
        mode=1;
        goto sh_below;
    case V('S','h'):        /* BSD mandoc */
        mandoc_command = 1;
    case V('S','H'):
    sh_below:
        c=c+j;
        if (*c == '\n') c++;
        dl_down();
        out_html(change_to_font(0));
        out_html(change_to_size(0));
        if (!fillout) {
        fillout=1;
        out_html("</PRE>");
        }
        trans_char(c,'"', '\a');
        add_to_index(mode, c);
        out_html("<A NAME=\"");
        out_html(label);
        /* &nbsp; for mosaic users */
        if (mode) out_html("\">&nbsp;</A>\n<H3>");
        else out_html("\">&nbsp;</A>\n<H2>");
        mandoc_synopsis = (strncmp(c, "SYNOPSIS", 8) == 0);
        c = (mandoc_command ? scan_troff_mandoc : scan_troff)(c,1,NULL);
        if (mode) out_html("</H3>\n");
        else out_html("</H2>\n");
        curpos=0;
        break;
    case V('T','S'):
        c=scan_table(c);
        break;
    case V('D','t'):        /* BSD mandoc */
        mandoc_command = 1;
    case V('T','H'):
        if (!output_possible) {
        sl = fill_words(c+j, wordlist, SIZE(wordlist), &words, 0);
        *sl = 0;
        if (words > 1) {
            output_possible=1;
            out_html("<HTML><HEAD><TITLE>Manpage of ");
            out_html(wordlist[0]);
            out_html("</TITLE>\n");
            out_html("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\n");
            out_html("</HEAD><BODY>\n<H1>");
            out_html(wordlist[0]);
            out_html("</H1>\nSection: ");
            if (words>4)
            out_html(wordlist[4]);
            else
            out_html(section_name(wordlist[1]));
            out_html(" (");
            out_html(wordlist[1]);
            if (words>2) {
            out_html(")<BR>Updated: ");
            scan_troff(wordlist[2], 1, NULL);
            } else out_html(")");
            out_html("<BR><A HREF=\"#index\">Index</A>\n");
            man_page_html(0,0);    /* Return to Main Contents */
            *sl='\n';
            out_html("<HR>\n");
            if (mandoc_command) out_html("<BR>BSD mandoc<BR>");
        }
        c = sl+1;
        } else
        c = skip_till_newline(c);
        curpos=0;
        break;
    case V('T','X'):
        sl=fill_words(c+j, wordlist, SIZE(wordlist), &words, '\n');
        *sl=0;
        out_html(change_to_font('I'));
        if (words>1) wordlist[1][-1]=0;
        c=lookup_abbrev(wordlist[0]);
        curpos+=strlen(c);
        out_html(c);
        out_html(change_to_font('R'));
        if (words>1)
        out_html(wordlist[1]);
        *sl='\n';
        c=sl+1;
        break;
    case V('r','m'):
            /* .rm xx : Remove request, macro or string */
    case V('r','n'):
            /* .rn xx yy : Rename request, macro or string xx to yy */
        {
        STRDEF *de;
        c=c+j;
        i=V(c[0],c[1]);
        c=c+2;
        while (isspace(*c) && *c!='\n') c++;
        j=V(c[0],c[1]);
        while (*c && *c!='\n') c++;
        c++;
        de=strdef;
        while (de && de->nr!=j) de=de->next;
        if (de) {
            if (de->st) free(de->st);
            de->nr=0;
        }
        de=strdef;
        while (de && de->nr!=i) de=de->next;
        if (de) de->nr=j;
        break;
        }
    case V('n','x'):
            /* .nx filename : next file. */
    case V('i','n'):
            /* .in +-N : Indent */
        c=skip_till_newline(c);
        break;
    case V('n','r'):
            /* .nr R +-N M: define and set number register R by +-N;
        **  auto-increment by M
        */
        {
        INTDEF *intd;
        c=c+j;
        i=V(c[0],c[1]);
        c=c+2;
        intd=intdef;
        while (intd && intd->nr!=i) intd=intd->next;
        if (!intd) {
            intd = (INTDEF*) xmalloc(sizeof(INTDEF));
            intd->nr=i;
            intd->val=0;
            intd->incr=0;
            intd->next=intdef;
            intdef=intd;
        }
        while (*c == ' ' || *c == '\t') c++;
        c=scan_expression(c,&intd->val);
        if (*c!='\n') {
            while (*c == ' ' || *c == '\t') c++;
            c=scan_expression(c,&intd->incr);
        }
        c=skip_till_newline(c);
        break;
        }
    case V('a','m'):
            /* .am xx yy : append to a macro. */
            /* define or handle as .ig yy */
        mode=1;
    case V('d','e'):
            /* .de xx yy : define or redefine macro xx; end at .yy (..) */
            /* define or handle as .ig yy */
        {
        STRDEF *de;
        int olen=0;
        c=c+j;
        sl=fill_words(c, wordlist, SIZE(wordlist), &words, '\n');
        i=V(c[0],c[1]);j=2;
        if (words == 1) wordlist[1]=".."; else {
            wordlist[1]--;
            wordlist[1][0]='.';
            j=3;
        }
        c=sl+1;
        sl=c;
        while (*c && strncmp(c,wordlist[1],j)) c=skip_till_newline(c);
        de=defdef;
        while (de && de->nr!= i) de=de->next;
        if (mode && de) olen=strlen(de->st);
        j=olen+c-sl;
        h= (char*) xmalloc((j*2+4)*sizeof(char));
        if (h) {
            for (j=0; j<olen; j++)
            h[j]=de->st[j];
            if (!j || h[j-1]!='\n')
            h[j++]='\n';
            while (sl!=c) {
            if (sl[0] == '\\' && sl[1] == '\\') {
                h[j++]='\\'; sl++;
            } else
                h[j++]=*sl;
            sl++;
            }
            h[j]=0;
            if (de) {
            if (de->st) free(de->st);
            de->st=h;
            } else {
            de = (STRDEF*) xmalloc(sizeof(STRDEF));
            de->nr=i;
            de->next=defdef;
            de->st=h;
            defdef=de;
            }
        }
        }
        c=skip_till_newline(c);
        break;

    /* ----- BSD mandoc stuff below ----- */
    case V('U','x'):    /* BSD mandoc */
        c=c+j;
        out_html("UNIX");
        c=skip_till_newline(c);
        break;
    case V('A','t'):    /* BSD mandoc - called with arg V */
        c=c+j;
        out_html("AT&amp;T System");
        break;
    case V('B','l'):    /* BSD mandoc */
    {
        char *nl, t=0 /* just for gcc */;
        c=c+j;
        nl = strchr(c,'\n');
        if (nl) {
         t = *nl;
         *nl = 0;
        }
        if (strstr(c, "-bullet")) /* HTML Unnumbered List */
         dl_newlevel_type(UL);
        else if (strstr(c, "-enum")) /* HTML Ordered List */
         dl_newlevel_type(OL);
        else             /* HTML Descriptive List */
         dl_newlevel_type(DL);
        if (nl)
         *nl = t;
        if (fillout) out_html("<P>\n"); else {
         out_html(NEWLINE);
         NEWLINE[0]='\n';
        }
        curpos=0;
        c=skip_till_newline(c);
        break;
    }
    case V('E','l'):    /* BSD mandoc */
         c=c+j;
         dl_endlevel_type();
         if (fillout) out_html("<P>\n"); else {
          out_html(NEWLINE);
          NEWLINE[0]='\n';
         }
         curpos=0;
         c=skip_till_newline(c);
         break;
    case V('I','t'):    /* BSD mandoc */
         c=c+j;
         if (dl_type(DL)) {
          out_html("<DT>");
          out_html(change_to_font('B'));
          if (*c == '\n') {
               /* Don't allow embedded comms after a newline */
               c++;
               c=scan_troff(c,1,NULL);
          } else {
               /* Do allow embedded comms on the same line. */
               c=scan_troff_mandoc(c,1,NULL);
          }
          out_html(change_to_font('R'));
          out_html(NEWLINE);
          if (inXo)
               still_dd = 1;
          else
               out_html("<DD>");
         } else if (dl_type(UL) || dl_type(OL)) {
          out_html("<LI>");
          c=scan_troff_mandoc(c,1,NULL);
          out_html(NEWLINE);
         }
         if (fillout) curpos++; else curpos=0;
         break;
    case V('X','o'):    /* BSD mandoc */
         c=c+j;
         inXo = 1;
         break;
    case V('X','c'):    /* BSD mandoc - Xc closes an Xo */
         c=c+j;
         if (inXo) {
          if (still_dd)
               out_html("<DD>");
          inXo = 0;
         }
         break;
    case V('S','m'):    /* BSD mandoc - called with arg on/off */
         c=skip_till_newline(c);
         break;
    case V('B','k'):    /* BSD mandoc */
    case V('E','k'):    /* BSD mandoc */
    case V('D','d'):    /* BSD mandoc */
    case V('O','s'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('B','t'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         out_html(" is currently in beta test.");
         if (fillout) curpos++; else curpos=0;
         break;
    case V('B','x'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         out_html("BSD ");
         c=scan_troff_mandoc(c, 1, NULL);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('D','l'):    /* BSD mandoc */
         c=c+j;
         out_html(NEWLINE);
         out_html("<BLOCKQUOTE>");
         out_html(change_to_font('L'));
         if (*c == '\n') c++;
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(change_to_font('R'));
         out_html("</BLOCKQUOTE>");
         if (fillout) curpos++; else curpos=0;
         break;
    case V('B','d'):    /* BSD mandoc */
    {            /* Seems like a kind of example/literal mode */
         char *nl, t=0 /* just for gcc */;
         c=c+j;
         nl = strchr(c,'\n');
         if (nl) {
          t = *nl;
          *nl = 0;
         }
         out_html(NEWLINE);
         mandoc_bd_options = 0; /* Remember options for terminating Bl */
         if (strstr(c, "-offset indent")) {
          mandoc_bd_options |= BD_INDENT;
          out_html("<BLOCKQUOTE>\n");
         }
         if (strstr(c, "-literal") || strstr(c, "-unfilled")) {
          if (fillout) {
               mandoc_bd_options |= BD_LITERAL;
               out_html(change_to_font(0));
               out_html(change_to_size('0'));
               out_html("<PRE>\n");
          }
          curpos=0;
          fillout=0;
         }
         if (nl)
          *nl = t;
         c=skip_till_newline(c);
         break;
    }
    case V('E','d'):    /* BSD mandoc */
         if (mandoc_bd_options & BD_LITERAL) {
          if (!fillout) {
               out_html(change_to_font(0));
               out_html(change_to_size('0'));
               out_html("</PRE>\n");
          }
         }
         if (mandoc_bd_options & BD_INDENT)
          out_html("</BLOCKQUOTE>\n");
         curpos=0;
         fillout=1;
         c=skip_till_newline(c);
         break;
    case V('B','e'):    /* BSD mandoc */
         c=c+j;
         if (fillout) out_html("<P>"); else {
          out_html(NEWLINE);
          NEWLINE[0]='\n';
         }
         curpos=0;
         c=skip_till_newline(c);
         break;
    case V('X','r'):    /* BSD mandoc */
    {
         /* Translate xyz 1 to xyz(1)
          * Allow for multiple spaces.  Allow the section to be missing.
          */
         char buff[100];
         char *bufptr;
         trans_char(c,'"','\a');
         bufptr = buff;
         c = c+j;
         if (*c == '\n') c++; /* Skip spaces */
         while (isspace(*c) && *c != '\n') c++;
         while (isalnum(*c) && bufptr < buff + SIZE(buff)-4) {
          /* Copy the xyz part */
          *bufptr++ = *c++;
         }
         while (isspace(*c) && *c != '\n') c++;    /* Skip spaces */
         if (isdigit(*c)) { /* Convert the number if there is one */
          *bufptr++ = '(';
          while (isalnum(*c) && bufptr < buff + SIZE(buff)-3) {
               *bufptr++ = *c++;
          }
          *bufptr++ = ')';
         }
         while (*c != '\n' && bufptr < buff + SIZE(buff)-2) {
          /* Copy the remainder */
          if (!isspace(*c)) {
               *bufptr++ = *c;
          }
          c++;
         }
         *bufptr++ = '\n';
         *bufptr = 0;
         scan_troff_mandoc(buff, 1, NULL);
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
    }
    break;
    case V('F','l'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         out_html("-");
         if (*c!='\n') {
          out_html(change_to_font('B'));
          c=scan_troff_mandoc(c, 1, NULL);
          out_html(change_to_font('R'));
         }
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('P','a'):    /* BSD mandoc */
    case V('P','f'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('P','p'):    /* BSD mandoc */
         if (fillout) out_html("<P>\n"); else {
          out_html(NEWLINE);
          NEWLINE[0]='\n';
         }
         curpos=0;
         c=skip_till_newline(c);
         break;
    case V('D','q'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         out_html("``");
         c=scan_troff_mandoc(c, 1, NULL);
         out_html("''");
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('O','p'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         out_html(change_to_font('R'));
         out_html("[");
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(change_to_font('R'));
         out_html("]");
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('O','o'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         out_html(change_to_font('R'));
         out_html("[");
         c=scan_troff_mandoc(c, 1, NULL);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('O','c'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(change_to_font('R'));
         out_html("]");
         if (fillout) curpos++; else curpos=0;
         break;
    case V('P','q'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         out_html("(");
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(")");
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('Q','l'):    /* BSD mandoc */
    {            /* Single quote first word in the line */
         char *sp;
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         sp = c;
         do {        /* Find first whitespace after the
                 * first word that isn't a mandoc macro
                 */
          while (*sp && isspace(*sp)) sp++;
          while (*sp && !isspace(*sp)) sp++;
         } while (*sp && isupper(*(sp-2)) && islower(*(sp-1)));

            /* Use a newline to mark the end of text to
             * be quoted
             */
         if (*sp) *sp = '\n';
         out_html("`");    /* Quote the text */
         c=scan_troff_mandoc(c, 1, NULL);
         out_html("'");
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    }
    case V('S','q'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         out_html("`");
         c=scan_troff_mandoc(c, 1, NULL);
         out_html("'");
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('A','r'):    /* BSD mandoc */
         /* parse one line in italics */
         out_html(change_to_font('I'));
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') {    /* An empty Ar means "file ..." */
          out_html("file ...");
         } else {
          c=scan_troff_mandoc(c, 1, NULL);
         }
         out_html(change_to_font('R'));
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('A','d'):    /* BSD mandoc */
    case V('E','m'):    /* BSD mandoc */
    case V('V','a'):    /* BSD mandoc */
         /* parse one line in italics */
         out_html(change_to_font('I'));
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(change_to_font('R'));
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('N','d'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         out_html(" - ");
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('N','m'):    /* BSD mandoc */
    {
         static char *mandoc_name = 0;
         trans_char(c,'"','\a');
         c=c+j;
         if (mandoc_synopsis) {
          /*
           * Break lines only in the Synopsis.
           * The Synopsis section seems to be treated
           * as a special case - Bummer!
           */
          static int count = 0; /* Don't break on the first Nm */
          if (count) {
               out_html("<BR>");
          } else {
               char *end, t=0 /* just for gcc */;
               end = strchr(c, '\n');
               if (end) {
                t = *end;
                *end = 0;
               }
               if (mandoc_name)
                free(mandoc_name);
               mandoc_name = xstrdup(c);
               if (end)
                *end = t;
          }
          count++;
         }
         out_html(change_to_font('B'));
         while (*c == ' ' || *c == '\t') c++;
         if (*c == '\n') {
          /*
           * If Nm has no argument, use one from an earlier
           * Nm command that did have one.  Hope there aren't
           * too many commands that do this.
           */
          if (mandoc_name)
               out_html(mandoc_name);
         } else {
          c=scan_troff_mandoc(c, 1, NULL);
         }
         out_html(change_to_font('R'));
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    }
    case V('C','d'):    /* BSD mandoc */
    case V('C','m'):    /* BSD mandoc */
    case V('I','c'):    /* BSD mandoc */
    case V('M','s'):    /* BSD mandoc */
    case V('O','r'):    /* BSD mandoc */
    case V('S','y'):    /* BSD mandoc */
         /* parse one line in bold */
         out_html(change_to_font('B'));
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(change_to_font('R'));
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('D','v'):    /* BSD mandoc */
    case V('E','v'):    /* BSD mandoc */
    case V('F','r'):    /* BSD mandoc */
    case V('L','i'):    /* BSD mandoc */
    case V('N','o'):    /* BSD mandoc */
    case V('N','s'):    /* BSD mandoc */
    case V('T','n'):    /* BSD mandoc */
    case V('n','N'):    /* BSD mandoc */
         trans_char(c,'"','\a');
         c=c+j;
         if (*c == '\n') c++;
         out_html(change_to_font('B'));
         c=scan_troff_mandoc(c, 1, NULL);
         out_html(change_to_font('R'));
         out_html(NEWLINE);
         if (fillout) curpos++; else curpos=0;
         break;
    case V('%','A'):    /* BSD mandoc biblio stuff */
    case V('%','D'):
    case V('%','N'):
    case V('%','O'):
    case V('%','P'):
    case V('%','Q'):
    case V('%','V'):
         c=c+j;
         if (*c == '\n') c++;
         c=scan_troff(c, 1, NULL); /* Don't allow embedded mandoc coms */
         if (fillout) curpos++; else curpos=0;
         break;
    case V('%','B'):
    case V('%','J'):
    case V('%','R'):
    case V('%','T'):
         c=c+j;
         out_html(change_to_font('I'));
         if (*c == '\n') c++;
         c=scan_troff(c, 1, NULL); /* Don't allow embedded mandoc coms */
         out_html(change_to_font('R'));
         if (fillout) curpos++; else curpos=0;
         break;
        /* ----- end of BSD mandoc stuff ----- */

     default:
             /* search macro database of self-defined macros */
         owndef = defdef;
        while (owndef && owndef->nr!=i) owndef=owndef->next;
        if (owndef) {
        char **oldargument;
        int deflen;
        int onff;
        sl=fill_words(c+j, wordlist, SIZE(wordlist), &words, '\n');
        c=sl+1;
        *sl=0;
        for (i=1; i<words; i++)
            wordlist[i][-1]=0;
        for (i=0; i<words; i++) {
            char *h=NULL;
            if (mandoc_command)
             scan_troff_mandoc(wordlist[i],1,&h);
            else
             scan_troff(wordlist[i],1,&h);
            wordlist[i]=h;
        }
        for (i=words; i<SIZE(wordlist); i++)
            wordlist[i]=NULL;
        deflen = strlen(owndef->st);
        owndef->st[deflen+1]='a';
        for (i=0; (owndef->st[deflen+2+i] = owndef->st[i]); i++);
        oldargument=argument;
        argument=wordlist;
        onff=newline_for_fun;
        if (mandoc_command)
             scan_troff_mandoc(owndef->st+deflen+2, 0, NULL);
        else
             scan_troff(owndef->st+deflen+2, 0, NULL);
        newline_for_fun=onff;
        argument=oldargument;
        for (i=0; i<words; i++) if (wordlist[i]) free(wordlist[i]);
        *sl='\n';
        } else if (mandoc_command &&
               ((isupper(*c) && islower(c[1]))
            || (islower(*c) && isupper(c[1])))) {
         /*
          * Let through any BSD mandoc commands that haven't
          * been dealt with.
          * I don't want to miss anything out of the text.
          */
               char buf[4];
               strncpy(buf,c,2);
               buf[2] = ' ';
               buf[3] = 0;
               out_html(buf);  /* Print the command (it might just be text). */
               c=c+j;
               trans_char(c,'"','\a');
               if (*c == '\n') c++;     /* really? */
               out_html(change_to_font('R'));
               c=scan_troff(c, 1, NULL);
               out_html(NEWLINE);
               if (fillout) curpos++; else curpos=0;
        } else
        c=skip_till_newline(c);
        break;
    }
    }
    if (fillout) { out_html(NEWLINE); curpos++; }
    NEWLINE[0]='\n';
    return c;
}

static int contained_tab=0;
static int mandoc_line=0;/* Signals whether to look for embedded mandoc cmds */

static char *
scan_troff(char *c, int san, char **result) {   /* san : stop at newline */
    char *h;
    char intbuff[500];
    int ibp=0;
#define FLUSHIBP  if (ibp) { intbuff[ibp]=0; out_html(intbuff); ibp=0; }
    char *exbuffer;
    int exbuffpos, exbuffmax, exscaninbuff, exnewline_for_fun;
    int usenbsp = 0;

    exbuffer = buffer;
    exbuffpos = buffpos;
    exbuffmax = buffmax;
    exnewline_for_fun = newline_for_fun;
    exscaninbuff = scaninbuff;
    newline_for_fun = 0;
    if (result) {
    if (*result) {
        buffer = *result;
        buffpos = strlen(buffer);
        buffmax = buffpos;
    } else {
        buffer = (char *) xmalloc(1000*sizeof(char));
        buffpos = 0;
        buffmax = 1000;
    }
    scaninbuff = 1;
    }
    h = c;
    /* start scanning */
    while (*h && (!san || newline_for_fun || *h != '\n')) {
    if (*h == escapesym) {
        h++;
        FLUSHIBP;
        h = scan_escape(h);
    } else if (*h == controlsym && h[-1] == '\n') {
        h++;
        FLUSHIBP;
        h = scan_request(h);
        if (san && h[-1] == '\n') h--;
    } else if (mandoc_line
           && *(h) && isupper(*(h))
           && *(h+1) && islower(*(h+1))
           && *(h+2) && isspace(*(h+2))) {
         /* BSD imbedded command eg ".It Fl Ar arg1 Fl Ar arg2" */
         FLUSHIBP;
         h = scan_request(h);
         if (san && h[-1] == '\n') h--;
    } else if (*h == nobreaksym && h[-1] == '\n') {
        h++;
        FLUSHIBP;
        h = scan_request(h);
        if (san && h[-1] == '\n') h--;
    } else {
        if (h[-1] == '\n' && still_dd && isalnum(*h)) {
        /* sometimes a .HP request is not followed by a .br request */
        FLUSHIBP;
        out_html("<DD>");
        curpos=0;
        still_dd=0;
        }
        switch (*h) {
        case '&':
        intbuff[ibp++]='&';
        intbuff[ibp++]='a';
        intbuff[ibp++]='m';
        intbuff[ibp++]='p';
        intbuff[ibp++]=';';
        curpos++;
        break;
        case '<':
        intbuff[ibp++]='&';
        intbuff[ibp++]='l';
        intbuff[ibp++]='t';
        intbuff[ibp++]=';';
        curpos++;
        break;
        case '>':
        intbuff[ibp++]='&';
        intbuff[ibp++]='g';
        intbuff[ibp++]='t';
        intbuff[ibp++]=';';
        curpos++;
        break;
        case '"':
        intbuff[ibp++]='&';
        intbuff[ibp++]='q';
        intbuff[ibp++]='u';
        intbuff[ibp++]='o';
        intbuff[ibp++]='t';
        intbuff[ibp++]=';';
        curpos++;
        break;
        case '\n':
        if (h[-1] == '\n' && fillout) {
            intbuff[ibp++]='<';
            intbuff[ibp++]='P';
            intbuff[ibp++]='>';
        }
        if (contained_tab && fillout) {
            intbuff[ibp++]='<';
            intbuff[ibp++]='B';
            intbuff[ibp++]='R';
            intbuff[ibp++]='>';
        }
        contained_tab=0;
        curpos=0;
        usenbsp=0;
        intbuff[ibp++]='\n';
        break;
        case '\t':
        {
            int curtab=0;
            contained_tab=1;
            FLUSHIBP;
            /* like a typewriter, not like TeX */
            tabstops[SIZE(tabstops)-1] = curpos+1;
            while (curtab < maxtstop && tabstops[curtab] <= curpos)
            curtab++;
            if (curtab < maxtstop) {
            if (!fillout) {
                while (curpos<tabstops[curtab]) {
                intbuff[ibp++]=' ';
                if (ibp>480) { FLUSHIBP; }
                curpos++;
                }
            } else {
                out_html("<TT>");
                while (curpos < tabstops[curtab]) {
                out_html("&nbsp;");
                curpos++;
                }
                out_html("</TT>");
            }
            }
        }
        break;
        default:
        if (*h == ' ' && (h[-1] == '\n' || usenbsp)) {
            FLUSHIBP;
            if (!usenbsp && fillout) {
            out_html("<BR>");
            curpos=0;
            }
            usenbsp=fillout;
            if (usenbsp) out_html("&nbsp;"); else intbuff[ibp++]=' ';
        } else if (*h > 31 && *h < 127) {
            intbuff[ibp++]=*h;
        } else if (((unsigned char)(*h)) > 127) {
#ifdef NO_8BIT
            intbuff[ibp++]='&';
            intbuff[ibp++]='#';
            intbuff[ibp++]='0'+((unsigned char)(*h))/100;
            intbuff[ibp++]='0'+(((unsigned char)(*h))%100)/10;
            intbuff[ibp++]='0'+((unsigned char)(*h))%10;
            intbuff[ibp++]=';';
#else
            intbuff[ibp++]=*h;
#endif
        }
        curpos++;
        break;
        }
        if (ibp>480) FLUSHIBP;
        h++;
    }
    }
    FLUSHIBP;
    if (buffer) buffer[buffpos]=0;
    if (san && *h) h++;
    newline_for_fun=exnewline_for_fun;
    if (result) {
    *result = buffer;
    buffer=exbuffer;
    buffpos=exbuffpos;
    buffmax=exbuffmax;
    scaninbuff=exscaninbuff;
    }
    return h;
}

static char *scan_troff_mandoc(char *c, int san, char **result) {
     char *ret, *end = c;
     int oldval = mandoc_line;
     mandoc_line = 1;
     while (*end && *end != '\n') {
      end++;
     }

     if (end > c + 2
     && ispunct(*(end - 1))
     && isspace(*(end - 2)) && *(end - 2) != '\n') {
      /*
       * Don't format lonely punctuation. E.g. in "xyz ," format
       * the xyz and then append the comma removing the space.
       */
      *(end - 2) = '\n';
      ret = scan_troff(c, san, result);
      *(end - 2) = *(end - 1);
      *(end - 1) = ' ';
     } else {
      ret = scan_troff(c, san, result);
     }
     mandoc_line = oldval;
     return ret;
}

STRDEF *foundpages=NULL;

#define BUFSMSG 1024
static void
error_page(char *s, char *t, ...) {
    va_list p;
    char buf[BUFSMSG];
    char notify_buf[BUFSMSG+5];
    va_start(p, t);
    vsnprintf(buf, BUFSMSG, t, p);
    va_end(p);
#ifdef _KOLIBRI
    snprintf(notify_buf, BUFSMSG+5, "'%s' -E", buf);
    _ksys_exec("/sys/@notify", buf);
#else
    printf("%s %s\n", s, buf);
#endif
    exit(0);
}

char *
xstrdup(const char *s) {
     char *p = strdup(s);
     if (p == NULL)
      error_page("Out of memory",
             "Sorry, out of memory, aborting...\n");
     return p;
}

void *
xmalloc(size_t size) {
     void *p = malloc(size);
     if (p == NULL)
      error_page("Out of memory",
             "Sorry, out of memory, aborting...\n");
     return p;
}

void *
xrealloc(void *ptr, size_t size) {
     void *p = realloc(ptr,size);
     if (p == NULL)
      error_page("Out of memory",
             "Sorry, out of memory, aborting...\n");
     return p;
}

static void
usage(void) {
#ifdef _KOLIBRI
     _ksys_exec("/sys/@notify", "'Usage: man2html in_file [out_file.html]' -I");
#else
     puts("Usage: man2html in_file [out_file.html]"); 
#endif
     exit(0);
}

#if 0
static void
goto_dir(char *path, char **dir, char **name) {
     char *s, *t, *u;

     s = xstrdup(path);
     t = strrchr(s, '/');
     if (t) {
      *t = 0;
      u = strrchr(s, '/');
      *t = '/';
      if (u) {
          *u = 0;
#ifdef _KOLIBRI
          setcwd(s);
#else
          chdir(s);
#endif
          if (dir)
              *dir = s;
          if (name)
               *name = u+1;
#if 0
           else  /* complain or not - this need not be fatal */
            error_page("Error", "man2html: could not chdir to %s", s);
#endif
      }
     }
}
#endif

char 
*temp_file_name(char* in_path){
    char* in_name = basename(in_path);
    #define PST_SIZE 5
    const char* pst = ".html"; 
#ifdef _KOLIBRI
    #define TMP_PATH_SIZE 9
    const char* tmp_path = "/tmp0/1/";
#else
    #define TMP_PATH_SIZE 2
    const char* tmp_path = "./";
#endif
    char* full_name = xmalloc((TMP_PATH_SIZE+strlen(in_name)+PST_SIZE+1)*sizeof(char));
    strcpy(full_name, tmp_path);
    strcat(full_name, in_name);
    strcat(full_name, pst);
    return full_name;
}

int
main(int argc, char **argv) {
    FILE *f;
    int l, c;
    char *buf, *filename, *fnam = NULL;
    char *outfilename;
    
    if (argc < 2){
        usage();
    }
    
    /* Find filename */
    if (argc > 1) {
        fnam = argv[1];
        outfilename = temp_file_name(fnam);
    }
    if (argc > 2) {
        outfilename = argv[2];
    }

    if ((out = fopen(outfilename, "w")) == NULL) {
        error_page("Error!", "Cannot open file %s", outfilename);
        return 0;
    }

    filename = fnam;
    //directory = 0;

    /* Open input file */
     //goto_dir(fnam, &directory, &fnam);

     f = fopen(fnam, "r");
     if (f == NULL)
          error_page("File not found", "Could not open %s\n", filename);
     fname = fnam;

    /* Read entire file into buf[1..l] */
#define XTRA 5
    /* buf has 1 extra byte at the start, and XTRA extra bytes at the end */
    int ct;

    l = 0;

#ifdef _KOLIBRI
    ksys_bdfe_t bdfe;
    _ksys_file_get_info(filename, &bdfe);
    l = bdfe.size;
#else
    struct stat stbuf;
    if (stat(filename, &stbuf) != -1) l=stbuf.st_size;
#endif

    buf = (char *) xmalloc((l+1+XTRA)*sizeof(char));
    ct = fread(buf+1,1,l,f);
    if (ct < l)
        l = ct;
    fclose(f);

    buf[0] = '\n';
    buf[l+1] = '\n';
    buf[l+2] = buf[l+3] = 0;

#ifdef MAKEINDEX
    idxfile = fopen(INDEXFILE, "a");
#endif
    stdinit();
    scan_troff(buf+1,0,NULL);
    dl_down();
    out_html(change_to_font(0));
    out_html(change_to_size(0));
    if (!fillout) {
    fillout=1;
    out_html("</PRE>");
    }
    out_html(NEWLINE);
    if (output_possible) {
    /* &nbsp; for mosaic users */
    fprintf(out, "<HR>\n<A NAME=\"index\">&nbsp;</A><H2>Index</H2>\n<DL>\n");
    manidx[mip]=0;
    fprintf(out, "%s", manidx);
    if (subs) fprintf(out,"</DL>\n");
    fprintf(out, "</DL>\n");
    print_sig();
    fprintf(out, "</BODY>\n</HTML>\n");
    } else {
    if (!filename)
         filename = fname;
    if (*filename == '/')
         error_page("Invalid Manpage",
           "The requested file %s is not a valid (unformatted) "
           "man page.\nIf the file is a formatted manpage, "
           "you could try to load the\n"
           "<A HREF=\"file://localhost%s\">plain file</A>.\n",
           filename, filename);
    else
         error_page("Invalid Manpage",
           "The requested file %s is not a valid (unformatted) "
           "man page.", filename);
    }
    if (idxfile)
        fclose(idxfile);
    if (buf)
        free(buf);
#ifdef _KOLIBRI
    if(argc==2){
        _ksys_exec("/sys/network/webview", outfilename);
    }
#endif
    return 0;
}
