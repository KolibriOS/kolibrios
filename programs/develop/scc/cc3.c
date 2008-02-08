/*
** Small-C Compiler -- Part 3 -- Expression Analyzer.
** Copyright 1982, 1983, 1985, 1988 J. E. Hendrix
** Copyright 1998 H T Walheim
** All rights reserved.
*/

#include <stdio.h>
#include "cc.h"

#define ST 0   /* is[ST] - symbol table address, else 0 */
#define TI 1   /* is[TI] - type of indirect obj to fetch, else 0 */
#define TA 2   /* is[TA] - type of address, else 0 */
#define TC 3   /* is[TC] - type of constant (INT or UINT), else 0 */
#define CV 4   /* is[CV] - value of constant (+ auxiliary uses) */
#define OP 5   /* is[OP] - code of highest/last binary operator */
#define SA 6   /* is[SA] - stage address of "op 0" code, else 0 */

extern char
 *litq, *glbptr, *lptr,  ssname[NAMESIZE],  quote[2];
extern int
  ch,  csp,  litlab,  litptr,  nch,  op[16],  op2[16],
  opindex,  opsize, *snext;

/***************** lead-in functions *******************/

constexpr(val) int *val; {
  int constant;
  int *before, *start;
  setstage(&before, &start);
  expression(&constant, val);
  clearstage(before, 0);     /* scratch generated code */
  if(constant == 0) error("must be constant expression");
  return constant;
  }

null_is (is)
int is[7];
  {
    int i;

    for (i = 0; i < 7; ++i)
      {
	is[i] = 0;	/* <-- */
      }
  }
    
  
expression(con, val) int *con, *val;
  {
    int i;
    int is[7];

    null_is (is);
    
  if(level1(is)) fetch(is);
  *con = is[TC];
  *val = is[CV];
  }

test(label, parens)  int label, parens;  {
  int is[7];
  int *before, *start;

  null_is (is);
  
  if(parens) need("(");
  while(1) {
    setstage(&before, &start);
    if(level1(is)) fetch(is);
    if(match(",")) clearstage(before, start);
    else break;
    }
  if(parens) need(")");
  if(is[TC]) {             /* constant expression */
    clearstage(before, 0);
    if(is[CV]) return;
    gen(JMPm, label);
    return;
    }
  if(is[SA]) {             /* stage address of "oper 0" code */
    switch(is[OP]) {       /* operator code */
      case EQ12:
      case LE12u: zerojump(EQ10f, label, is); break;
      case NE12:
      case GT12u: zerojump(NE10f, label, is); break;
      case GT12:  zerojump(GT10f, label, is); break;
      case GE12:  zerojump(GE10f, label, is); break;
      case GE12u: clearstage(is[SA], 0);      break;
      case LT12:  zerojump(LT10f, label, is); break;
      case LT12u: zerojump(JMPm,  label, is); break;
      case LE12:  zerojump(LE10f, label, is); break;
      default:    gen(NE10f, label);          break;
      }
    }
  else gen(NE10f, label);
  clearstage(before, start);
  }

/*
** test primary register against zero and jump if false
*/
zerojump(oper, label, is) int oper, label, is[]; {
  clearstage(is[SA], 0);       /* purge conventional code */
  gen(oper, label);
  }

/***************** precedence levels ******************/
#ifdef _MSC_VER
level2();
level3();
level4();
level5();
level6();
level7();
level8();
level9();
level10();
level11();
level12();
level13();
#endif

level1(is) int is[];  {
  int k, is2[7], is3[2], oper, oper2;

  null_is (is2);
  null_is (is3);
  
  k = down1(level2, is);
  if(is[TC])
    {
#ifdef INT32
      gen(GETd1n, is[CV]);
#else
      gen(GETw1n, is[CV]);
#endif
    }
       if(match("|="))  {oper =        oper2 = OR12;}
  else if(match("^="))  {oper =        oper2 = XOR12;}
  else if(match("&="))  {oper =        oper2 = AND12;}
  else if(match("+="))  {oper =        oper2 = ADD12;}
  else if(match("-="))  {oper =        oper2 = SUB12;}
  else if(match("*="))  {oper = MUL12; oper2 = MUL12u;}
  else if(match("/="))  {oper = DIV12; oper2 = DIV12u;}
  else if(match("%="))  {oper = MOD12; oper2 = MOD12u;}
  else if(match(">>=")) {oper =        oper2 = ASR12;}
  else if(match("<<=")) {oper =        oper2 = ASL12;}
  else if(match("="))   {oper =        oper2 = 0;}
  else return k;
                        /* have an assignment operator */
  if(k == 0) {
    needlval();
    return 0;
    }
  is3[ST] = is[ST];
  is3[TI] = is[TI];
  if(is[TI]) {                             /* indirect target */
    if(oper) {                             /* ?= */
      gen(PUSH1, 0);                       /* save address */
      fetch(is);                           /* fetch left side */
      }
    down2(oper, oper2, level1, is, is2);   /* parse right side */
    if(oper) gen(POP2, 0);                 /* retrieve address */
    }
  else {                                   /* direct target */
    if(oper) {                             /* ?= */
      fetch(is);                           /* fetch left side */
      down2(oper, oper2, level1, is, is2); /* parse right side */
      }
    else {                                 /*  = */
      if(level1(is2)) fetch(is2);          /* parse right side */
      }
    }
  store(is3);                              /* store result */
  return 0;
  }

level2(is1)  int is1[]; {
  int is2[7], is3[7], k, flab, endlab, *before, *after;
  
  null_is (is2);
  null_is (is3);
  
  k = down1(level3, is1);                   /* expression 1 */
  if(match("?") == 0) return k;
  dropout(k, NE10f, flab = getlabel(), is1);
  if(down1(level2, is2)) fetch(is2);        /* expression 2 */
  else if(is2[TC])
    {
#ifdef INT32
      gen(GETd1n, is2[CV]);
#else
      gen(GETw1n, is2[CV]);
#endif
    }
  need(":");
  gen(JMPm, endlab = getlabel());
  gen(LABm, flab);
  if(down1(level2, is3)) fetch(is3);        /* expression 3 */
  else if(is3[TC])
    {
#ifdef INT32
      gen(GETd1n, is3[CV]);
#else
      gen(GETw1n, is3[CV]);
#endif
    }
  gen(LABm, endlab);

  is1[TC] = is1[CV] = 0;
  if(is2[TC] && is3[TC]) {                  /* expr1 ? const2 : const3 */
    is1[TA] = is1[TI] = is1[SA] = 0;
    }
  else if(is3[TC]) {                        /* expr1 ? var2 : const3 */
    is1[TA] = is2[TA];
    is1[TI] = is2[TI];
    is1[SA] = is2[SA];
    }
  else if((is2[TC])                         /* expr1 ? const2 : var3 */
       || (is2[TA] == is3[TA])) {           /* expr1 ? same2 : same3 */
    is1[TA] = is3[TA];
    is1[TI] = is3[TI];
    is1[SA] = is3[SA];
    }
  else error("mismatched expressions");
  return 0;
  }

level3 (is) int is[]; {return skim("||", EQ10f, 1, 0, level4,  is);}
level4 (is) int is[]; {return skim("&&", NE10f, 0, 1, level5,  is);}
level5 (is) int is[]; {return down("|",            0, level6,  is);}
level6 (is) int is[]; {return down("^",            1, level7,  is);}
level7 (is) int is[]; {return down("&",            2, level8,  is);}
level8 (is) int is[]; {return down("== !=",        3, level9,  is);}
level9 (is) int is[]; {return down("<= >= < >",    5, level10, is);}
level10(is) int is[]; {return down(">> <<",        9, level11, is);}
level11(is) int is[]; {return down("+ -",         11, level12, is);}
level12(is) int is[]; {return down("* / %",       13, level13, is);}

level13(is)  int is[];  {
  int k;
  char *ptr;
  if(match("++")) {                 /* ++lval */
    if(level13(is) == 0) {
      needlval();
      return 0;
      }
    step(rINC1, is, 0);
    return 0;
    }
  else if(match("--")) {            /* --lval */
    if(level13(is) == 0) {
      needlval();
      return 0;
      }
    step(rDEC1, is, 0);
    return 0;
    }
  else if(match("~")) {             /* ~ */
    if(level13(is)) fetch(is);
    gen(COM1, 0);
    is[CV] = ~ is[CV];
    return (is[SA] = 0);
    }
  else if(match("!")) {             /* ! */
    if(level13(is)) fetch(is);
    gen(LNEG1, 0);
    is[CV] = ! is[CV];
    return (is[SA] = 0);
    }
  else if(match("-")) {             /* unary - */
    if(level13(is)) fetch(is);
    gen(ANEG1, 0);
    is[CV] = -is[CV];
    return (is[SA] = 0);
    }
  else if(match("*")) {             /* unary * */
    if(level13(is)) fetch(is);
    if(ptr = is[ST]) is[TI] = ptr[TYPE];
    else             is[TI] = INT;
    is[SA] =       /* no (op 0) stage address */
    is[TA] =       /* not an address */
    is[TC] = 0;    /* not a constant */
    is[CV] = 1;    /* omit fetch() on func call */
    return 1;
    }
  else if(amatch("sizeof", 6)) {    /* sizeof() */
    int sz, p;  char *ptr, sname[NAMESIZE];
    if(match("(")) p = 1;
    else           p = 0;
    sz = 0;
    if     (amatch("unsigned", 8))  sz = INTSIZE;
    if     (amatch("int",      3))  sz = INTSIZE;
    else if(amatch("char",     4))  sz = 1;
    if(sz) {if(match("*"))          sz = PTRSIZE;}
    else if(symname(sname)
         && ((ptr = findloc(sname)) ||
             (ptr = findglb(sname)))
         && ptr[IDENT] != FUNCTION
         && ptr[IDENT] != LABEL)    sz = getint(ptr+SIZE, INTSIZE);
    else if(sz == 0) error("must be object or type");
    if(p) need(")");
    is[TC] = INT;
    is[CV] = sz;
    is[TA] = is[TI] = is[ST] = 0;
    return 0;
    }
  else if(match("&")) {             /* unary & */
    if(level13(is) == 0) {
      error("illegal address");
      return 0;
      }
    ptr = is[ST];
    is[TA] = ptr[TYPE];
    if(is[TI]) return 0;
    gen(POINT1m, ptr);
    is[TI] = ptr[TYPE];
    return 0;
    }
  else {
    k = level14(is);
    if(match("++")) {               /* lval++ */
      if(k == 0) {
        needlval();
        return 0;
        }
      step(rINC1, is, rDEC1);
      return 0;
      }
    else if(match("--")) {          /* lval-- */
      if(k == 0) {
        needlval();
        return 0;
        }
      step(rDEC1, is, rINC1);
      return 0;
      }
    else return k;
    }
  }

level14(is)  int *is; {
  int k, consta, val;
  char *ptr, *before, *start;
  k = primary(is);
  ptr = is[ST];
  blanks();
  if(ch == '[' || ch == '(') {
    int is2[7];                     /* allocate only if needed */

    null_is (is2);
    
    while(1) {
      if(match("[")) {              /* [subscript] */
        if(ptr == 0) {
          error("can't subscript");
          skip();
          need("]");
          return 0;
          }
        if(is[TA]) {if(k) fetch(is);}
        else       {error("can't subscript"); k = 0;}
        setstage(&before, &start);
        is2[TC] = 0;
        down2(0, 0, level1, is2, is2);
        need("]");
        if(is2[TC]) {
          clearstage(before, 0);
          if(is2[CV])
	    {             /* only add if non-zero */
	      if(ptr[TYPE] >> 2 == BPD)
		{
#ifdef INT32
		  gen(GETd2n, is2[CV] << LBPD);
#else
		  gen(GETw2n, is2[CV] << LBPD);
#endif
		}
	      else if(ptr[TYPE] >> 2 == BPW)
		{
#ifdef INT32
		  gen(GETd2n, is2[CV] << LBPW);
#else
		  gen(GETw2n, is2[CV] << LBPW);
#endif
		}
	      else
		{
#ifdef INT32
		  gen(GETd2n, is2[CV]);
#else
		  gen(GETw2n, is2[CV]);
#endif
		}
	      gen(ADD12, 0);
            }
          }
        else
	  {
	    if(ptr[TYPE] >> 2 == BPD)
	      {
		gen(DBL1, 0);
		gen(DBL1, 0);
	      }
	    else if(ptr[TYPE] >> 2 == BPW)
	      {
		gen(DBL1, 0);
	      }
	    gen(ADD12, 0);
          }
        is[TA] = 0;
        is[TI] = ptr[TYPE];
        k = 1;
        }
      else if(match("(")) {         /* function(...) */
        if(ptr == 0) callfunc(0);
        else if(ptr[IDENT] != FUNCTION) {
          if(k && !is[CV]) fetch(is);
          callfunc(0);
          }
        else callfunc(ptr);
        k = is[ST] = is[TC] = is[CV] = 0;
        }
      else return k;
      }
    }
  if(ptr && ptr[IDENT] == FUNCTION) {
    gen(POINT1m, ptr);
    is[ST] = 0;
    return 0;
    }
  return k;
  }

primary(is)  int *is; {
  char *ptr, sname[NAMESIZE];
  int k;
  if(match("(")) {                  /* (subexpression) */ 
    do k = level1(is); while(match(","));
    need(")");
    return k;
    }
  putint(0, is, 7 << LBPW);         /* clear "is" array */
  if(symname(sname)) {              /* is legal symbol */
    if(ptr = findloc(sname)) {      /* is local */
      if(ptr[IDENT] == LABEL) {
        experr();
        return 0;
        }
      gen(POINT1s, getint(ptr+OFFSET, INTSIZE));
      is[ST] = ptr;
      is[TI] = ptr[TYPE];
      if(ptr[IDENT] == ARRAY) {
        is[TA] = ptr[TYPE];
        return 0;
        }
      if(ptr[IDENT] == POINTER) {
        is[TI] = UINT;
        is[TA] = ptr[TYPE];
        }
      return 1;
      }
    if(ptr = findglb(sname)) {      /* is global */
      is[ST] = ptr;
      if(ptr[IDENT] != FUNCTION) {
        if(ptr[IDENT] == ARRAY) {
          gen(POINT1m, ptr);
          is[TI] = 
          is[TA] = ptr[TYPE];
          return 0;
          }
        if(ptr[IDENT] == POINTER)
          is[TA] = ptr[TYPE];
        return 1;
        }
      }
    else is[ST] = addsym(sname, FUNCTION, INT, 0, 0, &glbptr, AUTOEXT);
    return 0;
    }
  if(constant(is) == 0) experr();
  return 0;
  }

experr() {
  error("invalid expression");
#ifdef INT32
  gen(GETd1n, 0);
#else
  gen(GETw1n, 0);
#endif
  skip();
  }

/* attempt at right to left - do it later */
#ifdef LATER
pushargs (ptr, nargs)
char *ptr;
int nargs;
  {
    if (streq(lptr, ")") != 0)
      return;

    if(endst())
      return;

    if(ptr)
      {
	expression(&consta, &val);
	gen(PUSH1, 0);
      }
    else
      {
	gen(PUSH1, 0);
	expression(&consta, &val);
	gen(SWAP1s, 0);            /* don't push addr */
      }
    nargs = nargs + INTSIZE;         /* count args*BPW */

    if(match(",") == 0) break;
    
  }
#endif

callfunc(ptr)
char *ptr;      /* symbol table entry or 0 */
  {
    int nargs, consta, val;
    
    nargs = 0;
    blanks();                      /* already saw open paren */

    while(streq(lptr, ")") == 0)
      {
	if(endst())
	  break;
	if(ptr)
	  {
	    expression(&consta, &val);
	    gen(PUSH1, 0);
	  }
	else
	  {
	    gen(PUSH1, 0);
	    expression(&consta, &val);
	    gen(SWAP1s, 0);            /* don't push addr */
	  }
	nargs = nargs + INTSIZE;         /* count args*BPW */

	if(match(",") == 0) break;
      }
    need(")");
    if(ptr && (streq(ptr + NAME, "CCARGC") == 0))	/* accessing ... like va_args */
#ifdef INT32
      gen(ARGCNTn, nargs >> LBPD);		/* to get start of frame */
#else
      gen(ARGCNTn, nargs >> LBPW);		/* to get start of frame */
#endif
      
    if(ptr)
      gen(CALLm, ptr);
    else
      gen(CALL1, 0);
    gen(ADDSP, csp + nargs);
  }

/*
** true if is2's operand should be doubled
*/
fdouble (oper, is1, is2)
int oper, is1[], is2[];
{
  //////////////////////////////////////////////////////
  // This is a 'magic' function, its usage and function
  // are not so obvious
  //
  // Purpose:  when indexing an address we must know
  // what we are pointing at so that the indexsize is
  // proper, e,g,
  // charptr++,  should multiply index by a 1
  // shortptr++, should multiply index by a 2
  // intptr++,   should multiply index by a 4
  //
  // Algorithm:
  // IF
  // operation is ADD12 or SUB12
  // AND
  // is1 is an address (pointer, array or via & operator
  // AND
  // is2 is NOT an address (pointer, array or via & operator
  // THEN 
  // return the multiplication factor based on s1 (or true)
  // ELSE
  // return 0 (or false)
  //
  // Usage: The return value is used as a 'boolean'
  // for nonconstant values, indicating that runtime code
  // is necessary to do the necessary multiplication
  // For contant value the return value is used to do a compile-time
  // multiplication (shift actually)
  //

  if ((oper == ADD12 || oper == SUB12)
      && (is2[TA] == 0))
    {
      switch (is1[TA]>>2)
	{
	  default:
	  case 1:		// char
	    return (0);
	  case 2:		// short
	    return (1);
	  case 4:		// int
	    return (2);
	}
    }
  
  return (0);


  
/*
  - original code -
  if((oper != ADD12 && oper != SUB12)
     || (is1[TA] >> 2 != BPW)
     || (is2[TA]))
    
    return 0;

  
  return 1;
*/
}


step (oper, is, oper2) int oper, is[], oper2; {
  fetch(is);
  gen(oper, is[TA] ? (is[TA] >> 2) : 1);
  store(is);
  if(oper2) gen(oper2, is[TA] ? (is[TA] >> 2) : 1);
  }

store(is)
int is[];
  {
    char *ptr;
    
    if(is[TI])
      {                     /* putstk */
	if(is[TI] >> 2 == 1)
	  {
	    gen(PUTbp1, 0);
	  }
	else if(is[TI] >> 2 == 2)
	  {
	    gen(PUTwp1, 0);
	  }
	else
	  {
	    gen(PUTdp1, 0);
	  }
      }
    else
      {                          /* putmem */
	ptr = is[ST];
	if(ptr[IDENT] == POINTER)
	  {
#ifdef INT32	/* int and ptr-size are ALWAYS the same */
	  gen(PUTdm1, ptr);
#else
	  gen(PUTwm1, ptr);
#endif
	  }
	else if (ptr[TYPE] >> 2 == 1)
	  {
	    gen(PUTbm1, ptr);
	  }
	else if (ptr[TYPE] >> 2 == BPW)
	  {
	    gen(PUTwm1, ptr);
	  }
	else
	  {
	    gen(PUTdm1, ptr);
	  }
      }
  }

fetch(is)
int is[];
  {
    char *ptr;
    
    ptr = is[ST];
    if(is[TI])                          /* indirect */
      {
	if(is[TI] >> 2 == BPD)		/* pointer to DWORD */
	  {
	    gen(GETd1p,  0);
	  }
	else if(is[TI] >> 2 == BPW)	/* pointer to WORD */
	  {
	    /* if INT32 must make distinction between signed/unsigned <-- */
	    gen(GETw1p,  0);
	  }
	else
	  {
	    if(ptr[TYPE] & UNSIGNED)
	      gen(GETb1pu, 0);
	    else
	      gen(GETb1p,  0);
	  }
      } 
    else
      {					/* direct */
      
	if(ptr[IDENT] == POINTER)
	  {
#ifdef INT32
	    gen(GETd1m,  ptr);
#else
	    gen(GETw1m,  ptr);
#endif	    
	  }
	else if (ptr[TYPE] >> 2 == BPD)
	  {
	    gen(GETd1m,  ptr);
	  }
	else if (ptr[TYPE] >> 2 == BPW)
	  {
	    gen(GETw1m,  ptr);
	  }
	else
	  {
	    if(ptr[TYPE] & UNSIGNED)
	      gen(GETb1mu, ptr);
	    else
	      gen(GETb1m,  ptr);
	  }
      }
  }

constant(is)
int is[];
  {
    int offset;
    
    if (is[TC] = number(is + CV))
      {
#ifdef INT32
	gen(GETd1n,  is[CV]);
#else
	gen(GETw1n,  is[CV]);
#endif
      }
    else if(is[TC] = chrcon(is + CV))
      {
#ifdef INT32
	gen(GETd1n,  is[CV]);
#else
	gen(GETw1n,  is[CV]);
#endif
      }
    else if(string(&offset))
      {
	gen(POINT1l, offset);
      }
    else
      {
	return 0;
      }
    return 1;
  }

number(value)  int *value; {
  int k, minus;
  k = minus = 0;
  while(1) {
    if(match("+")) ;
    else if(match("-")) minus = 1;
    else break;
    }
  if(isdigit(ch) == 0) return 0;
  if(ch == '0') {
    while(ch == '0') inbyte();
    if(toupper(ch) == 'X') {
      inbyte();
      while(isxdigit(ch)) {
        if(isdigit(ch))
             k = k*16 + (inbyte() - '0');
        else k = k*16 + 10 + (toupper(inbyte()) - 'A');
        }
      }
    else while (ch >= '0' && ch <= '7')
      k = k*8 + (inbyte() - '0');
    }
  else while (isdigit(ch)) k = k*10 + (inbyte() - '0');
  if(minus) {
    *value = -k;
    return (INT);
    }
  if((*value = k) < 0) return (UINT);
  else                 return (INT);
  }

chrcon(value)
int *value;
  {
    int k;
    
    k = 0;
    if(match("'") == 0)
      return 0;
    while(ch != '\'')
      k = (k << 8) + (litchar() & 255);
    gch();
    *value = k;
    return (INT);
  }

string(offset) int *offset; {
  char c;
  if(match(quote) == 0) return 0;
  *offset = litptr;
  while (ch != '"') {
    if(ch == 0) break;
    stowlit(litchar(), 1);
    }
  gch();
  litq[litptr++] = 0;
  return 1;
  }

stowlit(value, size) int value, size; {
  if((litptr+size) >= LITMAX) {
    error("literal queue overflow");
    exit(ERRCODE);
    }
  putint(value, litq+litptr, size);
  litptr += size;
  }

litchar() {
  int i, oct;
  if(ch != '\\' || nch == 0) return gch();
  gch();
  switch(ch) {
    case 'n': gch(); return NEWLINE;
    case 't': gch(); return  9;  /* HT */
    case 'b': gch(); return  8;  /* BS */
    case 'f': gch(); return 12;  /* FF */
    }
  i = 3;
  oct = 0;
  while((i--) > 0 && ch >= '0' && ch <= '7')
    oct = (oct << 3) + gch() - '0';
  if(i == 2) return gch();
  else       return oct;
  }

/***************** pipeline functions ******************/

/*
** skim over terms adjoining || and && operators
*/
skim(opstr, tcode, dropval, endval, level, is)
  char *opstr;
  int tcode, dropval, endval, (*level)(), is[]; {
  int k, droplab, endlab;
  droplab = 0;
  while(1) {
    k = down1(level, is);
    if(nextop(opstr)) {
      bump(opsize);
      if(droplab == 0) droplab = getlabel();
      dropout(k, tcode, droplab, is);
      }
    else if(droplab) {
      dropout(k, tcode, droplab, is);
#ifdef INT32
      gen(GETd1n, endval);
#else
      gen(GETw1n, endval);
#endif
      gen(JMPm, endlab = getlabel());
      gen(LABm, droplab);
#ifdef INT32
      gen(GETd1n, dropval);
#else
      gen(GETw1n, dropval);
#endif
      gen(LABm, endlab);
      is[TI] = is[TA] = is[TC] = is[CV] = is[SA] = 0;
      return 0;
      }
    else return k;
    }
  }

/*
** test for early dropout from || or && sequences
*/
dropout(k, tcode, exit1, is)
  int k, tcode, exit1, is[]; {
  if(k) fetch(is);
  else if(is[TC])
    {
#ifdef INT32
      gen(GETd1n, is[CV]);
#else
      gen(GETw1n, is[CV]);
#endif
    }
  gen(tcode, exit1);          /* jumps on false */
  } 

/*
** drop to a lower level
*/
down(opstr, opoff, level, is)
  char *opstr;  int opoff, (*level)(), is[]; {
  int k;
  k = down1(level, is);
  if(nextop(opstr) == 0) return k;
  if(k) fetch(is);
  while(1) {
    if(nextop(opstr)) {
      int is2[7];     /* allocate only if needed */

      null_is (is2);
      
      bump(opsize);
      opindex += opoff;
      down2(op[opindex], op2[opindex], level, is, is2);
      }
    else return 0;
    }
  }

/*
** unary drop to a lower level
*/
down1(level, is) int (*level)(), is[]; {
  int k, *before, *start;
  setstage(&before, &start);
  k = (*level)(is);
  if(is[TC]) clearstage(before, 0);  /* load constant later */
  return k;
  }

/*
** binary drop to a lower level
*/
down2(oper, oper2, level, is, is2)
  int oper, oper2, (*level)(), is[], is2[]; {
  int *before, *start;
  char *ptr;
  int value;
  setstage(&before, &start);
  is[SA] = 0;                     /* not "... op 0" syntax */
  if(is[TC]) {                    /* consant op unknown */
    if(down1(level, is2)) fetch(is2);
    if(is[CV] == 0) is[SA] = snext;
    gen(GETw2n, is[CV] << fdouble(oper, is2, is));
    }
  else {                          /* variable op unknown */
    gen(PUSH1, 0);                /* at start in the buffer */
    if(down1(level, is2)) fetch(is2);
    if(is2[TC]) {                 /* variable op constant */
      if(is2[CV] == 0) is[SA] = start;
#ifdef INT32
      csp += BPD;                 /* adjust stack and */
#else
      csp += BPW;                 /* adjust stack and */
#endif
      clearstage(before, 0);      /* discard the PUSH */
      if(oper == ADD12) {         /* commutative */
#ifdef INT32
        gen(GETd2n, is2[CV] << fdouble(oper, is, is2));
#else
        gen(GETw2n, is2[CV] << fdouble(oper, is, is2));
#endif
        }
      else {                      /* non-commutative */
        gen(MOVE21, 0);
#ifdef INT32
        gen(GETd1n, is2[CV] << fdouble(oper, is, is2));
#else
        gen(GETw1n, is2[CV] << fdouble(oper, is, is2));
#endif
        }
      }
    else {                        /* variable op variable */
      gen(POP2, 0);
      if(value = fdouble(oper, is, is2))
	{
	  gen(DBL1, 0);		// index size 2
	  if (value > 1)
	    gen(DBL1, 0);	// .. or even 4
	}
      if(value = fdouble(oper, is2, is))
	{
	  gen(DBL2, 0);
	  if (value > 1)
	    gen(DBL2, 0);
	}
      }
    }
  if(oper) {
    if(nosign(is) || nosign(is2)) oper = oper2;
    if(is[TC] = is[TC] & is2[TC]) {               /* constant result */
      is[CV] = calc(is[CV], oper, is2[CV]);
      clearstage(before, 0);  
      if(is2[TC] == UINT) is[TC] = UINT;
      }
    else {                                        /* variable result */
      gen(oper, 0);
      if(oper == SUB12
      && is [TA] >> 2 == BPW
      && is2[TA] >> 2 == BPW) { /* difference of two word addresses */
        gen(SWAP12, 0);
        gen(GETw1n, 1);
        gen(ASR12, 0);          /* div by 2 */
        }
      is[OP] = oper;            /* identify the operator */
      }
    if(oper == SUB12 || oper == ADD12) {
      if(is[TA] && is2[TA])     /*  addr +/- addr */
        is[TA] = 0;
      else if(is2[TA]) {        /* value +/- addr */
        is[ST] = is2[ST];
        is[TI] = is2[TI];
        is[TA] = is2[TA];
        }
      }
    if(is[ST] == 0 || ((ptr = is2[ST]) && (ptr[TYPE] & UNSIGNED)))
      is[ST] = is2[ST];
    }
  }

/*
** unsigned operand?
*/
nosign(is) int is[]; {
  char *ptr;
  if(is[TA]
  || is[TC] == UINT
  || ((ptr = is[ST]) && (ptr[TYPE] & UNSIGNED))
    ) return 1;
  return 0;
  }

/*
** calcualte signed constant result
*/
calc(left, oper, right) int left, oper, right; {
  switch(oper) {
    case ADD12: return (left  +  right); 
    case SUB12: return (left  -  right);
    case MUL12: return (left  *  right); 
    case DIV12: return (left  /  right);
    case MOD12: return (left  %  right); 
    case EQ12:  return (left  == right); 
    case NE12:  return (left  != right);
    case LE12:  return (left  <= right); 
    case GE12:  return (left  >= right);
    case LT12:  return (left  <  right); 
    case GT12:  return (left  >  right);
    case AND12: return (left  &  right);
    case OR12:  return (left  |  right);
    case XOR12: return (left  ^  right); 
    case ASR12: return (left  >> right); 
    case ASL12: return (left  << right);
    } 
  return (calc2(left, oper, right));
  }

/*
** calcualte unsigned constant result
*/
calc2(left, oper, right) unsigned left, right; int oper; {
  switch(oper) {
    case MUL12u: return (left  *  right); 
    case DIV12u: return (left  /  right);
    case MOD12u: return (left  %  right); 
    case LE12u:  return (left  <= right); 
    case GE12u:  return (left  >= right);
    case LT12u:  return (left  <  right); 
    case GT12u:  return (left  >  right);
    } 
  return (0);
  }

