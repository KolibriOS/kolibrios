/*
** Small-C Compiler -- Part 2 -- Front End and Miscellaneous.
** Copyright 1982, 1983, 1985, 1988 J. E. Hendrix
** Copyright 1998 H T Walheim
** All rights reserved.
*/

#include <stdio.h>
#include "cc.h"

extern char
 *symtab, *macn, *macq, *pline, *mline,  optimize,
  alarm, *glbptr, *line, *lptr, *cptr, *cptr2,  *cptr3,
 *locptr, msname[NAMESIZE],  pause,  quote[2];

extern int
  *wq,  ccode,  ch,  csp,  eof,  errflag,  iflevel,
  input,  input2,  listfp,  macptr,  nch,
  nxtlab,  op[16],  opindex,  opsize,  output,  pptr,
  skiplevel,  *wqptr;

/********************** input functions **********************/

preprocess() {
  int k;
  char c;
  if(ccode) {
    line = mline;
    ifline();
    if(eof) return;
    }
  else {
    inline();
    return;
    }
  pptr = -1;
  while(ch != NEWLINE && ch) {
    if(white()) {
      keepch(' ');
      while(white()) gch();
      }
    else if(ch == '"') {
      keepch(ch);
      gch();
      while(ch != '"' || (*(lptr-1) == 92 && *(lptr-2) != 92)) {
        if(ch == NULL) {
          error("no quote");
          break;
          }
        keepch(gch());
        }
      gch();
      keepch('"');
      }
    else if(ch == 39) {
      keepch(39);
      gch();
      while(ch != 39 || (*(lptr-1) == 92 && *(lptr-2) != 92)) {
        if(ch == NULL) {
          error("no apostrophe");
          break;
          }
        keepch(gch());
        }
      gch();
      keepch(39);
      }
    else if(ch == '/' && nch == '*')
      {
	bump(2);
	while((ch == '*' && nch == '/') == 0)
	  {
	    if(ch)
	      bump(1);
	    else
	      {
		ifline();
		if(eof)
		  break;
	      }
	  }
	bump(2);
      }
    else  if(ch == '/' && nch == '/')
      {
	bump(2);
	while(ch != NEWLINE)
	  {
	    if(ch)
	      bump(1);
	    else
	      {
		if(eof)
		  break;
	      }
	  }
	bump(1);
      }
    else if(an(ch)) {
      k = 0;
      while(an(ch) && k < NAMEMAX) {
        msname[k++] = ch;
        gch();
        }
      msname[k] = NULL;
      if(search(msname, macn, NAMESIZE+2, MACNEND, MACNBR, 0)) {
        k = getint(cptr+NAMESIZE, 2/*INTSIZE*/);
        while(c = macq[k++]) keepch(c);
        while(an(ch)) gch();
        }
      else {
        k = 0;
        while(c = msname[k++]) keepch(c);
        }
      }
    else keepch(gch());
    }
  if(pptr >= LINEMAX) error("line too long");
  keepch(NULL);
  line = pline;
  bump(0);
  }

keepch(c)  char c; {
  if(pptr < LINEMAX) pline[++pptr] = c;
  }

ifline() {
  while(1) {
    inline();
    if(eof) return;
    if(match("#ifdef")) {
      ++iflevel;
      if(skiplevel) continue;
      symname(msname);
      if(search(msname, macn, NAMESIZE+2, MACNEND, MACNBR, 0) == 0)
        skiplevel = iflevel;
      continue;
      }
    if(match("#ifndef")) {
      ++iflevel;
      if(skiplevel) continue;
      symname(msname);
      if(search(msname, macn, NAMESIZE+2, MACNEND, MACNBR, 0))
        skiplevel = iflevel;
      continue;
      }
    if(match("#else")) {
      if(iflevel) {
        if(skiplevel == iflevel) skiplevel = 0;
        else if(skiplevel == 0)  skiplevel = iflevel;
        }
      else noiferr();
      continue;
      }
    if(match("#endif")) {
      if(iflevel) {
        if(skiplevel == iflevel) skiplevel = 0;
        --iflevel;
        }
      else noiferr();
      continue;
      }
    if(skiplevel) continue;
    if(ch == 0) continue;
    break;
    }
  }

inline() {           /* numerous revisions */
  int k, unit;

  if(input == EOF) openfile();
  if(eof) return;
  if((unit = input2) == EOF) unit = input;
  if(fgets(line, LINEMAX, unit) == NULL) {
    fclose(unit);
    if(input2 != EOF)
         input2 = EOF;
    else input  = EOF;
    *line = NULL;
    }
  else if(listfp)
    {
    if(listfp == output) fputc(';', output);
    fputs(line, listfp);
    }
#ifdef _MSC_VER
  else
    {
      fputc(';', output);
      fputs(line, output);
    }
#endif     
  bump(0);
  }

inbyte()  {
  while(ch == 0) {
    if(eof) return 0;
    preprocess();
    }
  return gch();
  }

/********************* scanning functions ********************/

/*
** test if next input string is legal symbol name
*/
symname(sname) char *sname; {
  int k;char c;
  blanks();
  if(alpha(ch) == 0) return (*sname = 0);
  k = 0;
  while(an(ch)) {
    sname[k] = gch();
    if(k < NAMEMAX) ++k;
    }
  sname[k] = 0;
  return 1;
  }

need(str)  char *str; {
  if(match(str) == 0) error("missing token");
  }

ns()  {
  if(match(";") == 0) error("no semicolon");
  else errflag = 0;
  }

match(lit)  char *lit; {
  int k;
  blanks();
  if(k = streq(lptr, lit)) {
    bump(k);
    return 1;
    }
  return 0;
  }

streq(str1, str2)  char str1[], str2[]; {
  int k;
  k = 0;
  while (str2[k]) {
    if(str1[k] != str2[k]) return 0;
    ++k;
    }
  return k;
 }

amatch(lit, len)  char *lit; int len; {
  int k;
  blanks();
  if(k = astreq(lptr, lit, len)) {
    bump(k);
    return 1;
    }
  return 0;
 }

astreq(str1, str2, len)  char str1[], str2[]; int len; {
  int k;
  k = 0;
  while (k < len) {
    if(str1[k] != str2[k]) break;
    /*
    ** must detect end of symbol table names terminated by
    ** symbol length in binary
    */
    if(str2[k] < ' ') break;
    if(str1[k] < ' ') break;
    ++k;
    }
  if(an(str1[k]) || an(str2[k])) return 0;
  return k;
  }

nextop(list) char *list; {
  char op[4];
  opindex = 0;
  blanks();
  while(1) {
    opsize = 0;
    while(*list > ' ') op[opsize++] = *list++;
    op[opsize] = 0;
    if(opsize = streq(lptr, op))
      if(*(lptr+opsize) != '=' && 
         *(lptr+opsize) != *(lptr+opsize-1))
         return 1;
    if(*list) {
      ++list;
      ++opindex;
      }
    else return 0;
    }
  }

blanks() {
  while(1) {
    while(ch) {
      if(white()) gch();
      else return;
      }
    if(line == mline) return;
    preprocess();
    if(eof) break;
    }
  }

white() {
  return (*lptr <= ' ' && *lptr);
  }

gch() {
  int c;
  if(c = ch) bump(1);
  return c;
  }

bump(n) int n; {
  if(n) lptr += n;
  else  lptr  = line;
  if(ch = nch = *lptr) nch = *(lptr+1);
  }

kill() {
  *line = 0;
  bump(0);
  }

skip() {
  if(an(inbyte()))
       while(an(ch)) gch();
  else while(an(ch) == 0) {
    if(ch == 0) break;
    gch();
    }
  blanks();
  }

endst() {
  blanks();
  return (streq(lptr, ";") || ch == 0);
  }

/*********** symbol table management functions ***********/

addsym(sname, id, type, size, value, lgpp, class)
char *sname, id, type;
int size, value, *lgpp, class;
  {
    if(lgpp == &glbptr)
      {
	if(cptr2 = findglb(sname))
	  return cptr2;
	if(cptr == 0)
	  {
	    error("global symbol table overflow");
	    return 0;
	  }
      }
    else
      {
	if(locptr > (ENDLOC-SYMMAX))
	  {
	    error("local symbol table overflow");
	    exit(ERRCODE);
	  }
	cptr = *lgpp;
      }
    cptr[IDENT] = id;
    cptr[TYPE]  = type;
    cptr[CLASS] = class;
    putint(size, cptr + SIZE, INTSIZE);
    putint(value, cptr + OFFSET, INTSIZE);
    cptr3 = cptr2 = cptr + NAME;
    while(an(*sname))
      *cptr2++ = *sname++;

    if(lgpp == &locptr)
      {
	*cptr2 = cptr2 - cptr3;         /* set length */
	*lgpp = ++cptr2;
      }
    return cptr;
  }

/*
** search for symbol match
** on return cptr points to slot found or empty slot
*/
search(sname, buf, len, end, max, off)
  char *sname, *buf, *end;  int len, max, off; {
  cptr  =
  cptr2 = buf+((hash(sname)%(max-1))*len);
  while(*cptr != NULL) {
    if(astreq(sname, cptr+off, NAMEMAX)) return 1;
    if((cptr = cptr+len) >= end) cptr = buf;
    if(cptr == cptr2) return (cptr = 0);
    }
  return 0;
  }

hash(sname) char *sname; {
  int i, c;
  i = 0;
  while(c = *sname++) i = (i << 1) + c;
  return i;
  }

findglb(sname)  char *sname; {
  if(search(sname, STARTGLB, SYMMAX, ENDGLB, NUMGLBS, NAME))
    return cptr;
  return 0;
  }

findloc(sname)  char *sname;  {
  cptr = locptr - 1;  /* search backward for block locals */
  while(cptr > STARTLOC) {
    cptr = cptr - *cptr;
    if(astreq(sname, cptr, NAMEMAX)) return (cptr - NAME);
    cptr = cptr - NAME - 1;
    }
  return 0;
  }

nextsym(entry) char *entry; {
  entry = entry + NAME;
  while(*entry++ >= ' ');    /* find length byte */
  return entry;
  }

/******** while queue management functions *********/  

addwhile(ptr)  int ptr[]; {
  int k;
  ptr[WQSP]   = csp;         /* and stk ptr */
  ptr[WQLOOP] = getlabel();  /* and looping label */
  ptr[WQEXIT] = getlabel();  /* and exit label */
  if(wqptr == WQMAX) {
    error("control statement nesting limit");
    exit(ERRCODE);
    }
  k = 0;
  while (k < WQSIZ) *wqptr++ = ptr[k++];
  }

readwhile(ptr) int *ptr; {
  if(ptr <= wq) {
    error("out of context");
    return 0;
    }
  else return (ptr - WQSIZ);
 }

delwhile() {
  if(wqptr > wq) wqptr -= WQSIZ;
  }

/****************** utility functions ********************/  

/*
** test if c is alphabetic
*/
alpha(c)  char c; {
  return (isalpha(c) || c == '_');
  }

/*
** test if given character is alphanumeric
*/
an(c)  char c; {
  return (alpha(c) || isdigit(c));
  }

/*
** return next avail internal label number
*/
getlabel() {
  return(++nxtlab);
  }

/*
** get integer of length len from address addr
** (byte sequence set by "putint")
*/
getint(addr, len) char *addr; int len; {
  int i;
  i = *(addr + --len);  /* high order byte sign extended */
  while(len--) i = (i << 8) | *(addr + len) & 255;
  return i;
  }

/*
** put integer i of length len into address addr
** (low byte first)
*/
putint(i, addr, len) char *addr; int i, len; {
  while(len--) {
    *addr++ = i;
    i = i >> 8;
    }
  }

lout(line, fd) char *line; int fd; {
  fputs(line, fd);
  fputc(NEWLINE, fd);
  }

/******************* error functions *********************/  

illname() {
  error("illegal symbol");
  skip();
  }

multidef(sname)  char *sname; {
  error("already defined");
  }

needlval() {
  error("must be lvalue");
  }

noiferr() {
  error("no matching #if...");
  errflag = 0;
  }

error(msg)
char msg[];
  {
    if(errflag)
      return;
    else
      errflag = 1;
    
    lout(line, stderr);
    errout(msg, stderr);
    if(alarm)
      fputc(7, stderr);
    if(pause)
      while(fgetc(stderr) != NEWLINE);
    if(listfp > 0)
      errout(msg, listfp);
  }

errout(msg, fp) char msg[]; int fp; {
  int k;
  k = line+2;
  while(k++ <= lptr) fputc(' ', fp);
  lout("/\\", fp);
  fputs("**** ", fp); lout(msg, fp);
  }

