#include "stdio.h"
#include "clib.h"

/*
** Write string to standard output. 
*/
puts(string) char *string; 
{fputs(string,stdout);
 OS_fputc('\n',stdout);
}

/*
** reverse string in place 
*/
reverse(s) char *s; 
{char *j; int c;

 j=s+strlen(s)-1;
 while(s<j) 
 {c=*s;
  *s++ =*j;
  *j-- =c;
 }
}

/*
** copy t to s 
*/
strcpy(s,t) char *s,*t;
{char *d;

 d=s;
 while(*s++ =*t++);
 return (d);
}

/*
** return length of string s (fast version)
*/
strlen(s) char *s;
{char *ptr;

 ptr=s;
 while(*ptr)
 {++ptr;
 }

  return (ptr-s);
  
#ifdef _INASM  
  #asm
  xor al,al        ; set search value to zero
  mov cx,65535     ; set huge maximum
  mov di,[bp+4]    ; get address of s
  cld              ; set direction flag forward
  repne scasb      ; scan for zero
  mov ax,65534
  sub ax,cx        ; calc and return length
  #endasm
#endif
}

/*
** return upper-case of c if it is lower-case, else c
*/
toupper(c) int c;
{if(c<='z' && c>='a') return (c-32);
 return (c);
}

/*
** atoi(s) - convert s to integer.
*/
atoi(s) char *s;
{int sign,n;

 while(isspace(*s)) ++s;
 sign = 1;
 switch(*s) 
 {case '-': sign=-1;
  case '+': ++s;
 }
 n=0;
 while(isdigit(*s)) n=10*n+*s++ -'0';
 return (sign*n);
}

/*
** atoib(s,b) - Convert s to "unsigned" integer in base b.
**              NOTE: This is a non-standard function.
*/
atoib(s,b) char *s; int b;
{int n, digit;

 n=0;
 while(isspace(*s)) ++s;
 while((digit=(127 & *s++))>='0')
 {     if(digit>='a') digit-=87;
  else if(digit>='A') digit-=55;
  else                digit -= '0';
  if(digit>=b) break;
  n=b*n+digit;
 }
 return (n);
}

/*
** Gets an entire string (including its newline
** terminator) or size-1 characters, whichever comes
** first. The input is terminated by a null character.
** Entry: str  = Pointer to destination buffer.
**        size = Size of the destination buffer.
**        fd   = File descriptor of pertinent file.
** Returns str on success, else NULL.
*/
fgets(str,size,fd) char *str; unsigned size,fd;
{return (_gets(str,size,fd,1));
}

/*
** Gets an entire string from stdin (excluding its newline
** terminator) or size-1 characters, whichever comes
** first. The input is terminated by a null character.
** The user buffer must be large enough to hold the data.
** Entry: str  = Pointer to destination buffer.
** Returns str on success, else NULL.
*/
gets(str) char *str;
{return (_gets(str,32767,stdin,0));
}

_gets(str,size,fd,nl) char *str; unsigned size,fd,nl;
{int backup; char *next;

 next=str;
 while(--size>0)
 {switch (*next=fgetc(fd)) 
  {case  EOF:
    *next=NULL;
    if(next==str) return (NULL);
    return (str);

   case '\n':
    *(next+nl)=NULL;
    return (str);

   case  RUB: /* \b */
    if(next>str) backup=1;
    else         backup=0;
    goto backout;

   case WIPE: /* \r */
    backup=next-str;
backout: 
    if(0/*iscons(fd)*/) 
    {++size;
     while(backup--) 
     {fputs("\b \b",stderr);
      --next;++size;
     }
     continue;
    }

   default: 
    ++next;
  }
 }
 *next = NULL;
 return (str);
}

/*
** fprintf(fd, ctlstring, arg, arg, ...) - Formatted print.
** Operates as described by Kernighan & Ritchie.
** b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
*/
fprintf(argc) int argc;
{int *nxtarg;

 nxtarg=CCARGC()+&argc;
 return(_print(*(--nxtarg),--nxtarg));
}

/*
** printf(ctlstring, arg, arg, ...) - Formatted print.
** Operates as described by Kernighan & Ritchie.
** b, c, d, o, s, u, and x specifications are supported.
** Note: b (binary) is a non-standard extension.
*/
printf(argc) int argc;
{return(_print(stdout,CCARGC()+&argc-1));
}

/*
** _print(fd, ctlstring, arg, arg, ...)
** Called by fprintf() and printf().
*/
_print(fd,nxtarg) int fd,*nxtarg; 
{int arg,left,pad,cc,len,maxchr,width;
 char *ctl,*sptr,str[17];

 cc=0;                                         
 ctl=*nxtarg--;                          
 while(*ctl) 
 {if(*ctl!='%') {OS_fputc(*ctl++,fd); ++cc; continue;}
  else ++ctl;
  if(*ctl=='%') {OS_fputc(*ctl++,fd); ++cc; continue;}
  if(*ctl=='-') {left=1; ++ctl;} else left=0;       
  if(*ctl=='0') pad='0';
  else pad=' ';
  if(isdigit(*ctl))
  {width=atoi(ctl++);
   while(isdigit(*ctl)) ++ctl;
  }else width=0;
  if(*ctl=='.') 
  {maxchr=atoi(++ctl);
   while(isdigit(*ctl)) ++ctl;
  }else maxchr=0;
  arg=*nxtarg--;
  sptr=str;
  switch(*ctl++)
  {case 'c': str[0]=arg; str[1]=NULL; break;
   case 's': sptr=arg;          break;
   case 'd': itoa(arg,str);     break;
   case 'b': itoab(arg,str,2);  break;
   case 'o': itoab(arg,str,8);  break;
   case 'u': itoab(arg,str,10); break;
   case 'x': itoab(arg,str,16); break;
   default: return (cc);
  }
  len=strlen(sptr);
  if(maxchr && maxchr<len) len=maxchr;
  if(width>len) width=width-len; else width=0; 
  if(!left) while(width--) {OS_fputc(pad,fd); ++cc;}
  while(len--) {OS_fputc(*sptr++,fd); ++cc;}
  if(left) while(width--) {OS_fputc(pad,fd); ++cc;}  
 }
 return (cc);
}

/*
** Write a string to fd. 
** Entry: string = Pointer to null-terminated string.
**        fd     = File descriptor of pertinent file.
*/
fputs(string,fd) char *string; int fd;
{while(*string)
     OS_fputc(*string++,fd);
}

/*
** All character classification functions except isascii().
** Integer argument (c) must be in ASCII range (0-127) for
** dependable answers.
*/

#define ALNUM     1
#define ALPHA     2
#define CNTRL     4
#define DIGIT     8
#define GRAPH    16
#define LOWER    32
#define PRINT    64
#define PUNCT   128
#define BLANK   256
#define UPPER   512
#define XDIGIT 1024

int _is[128] =
{0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
 0x004, 0x104, 0x104, 0x104, 0x104, 0x104, 0x004, 0x004,
 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
 0x140, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0,
 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0,
 0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459,
 0x459, 0x459, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0,
 0x0D0, 0x653, 0x653, 0x653, 0x653, 0x653, 0x653, 0x253,
 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
 0x253, 0x253, 0x253, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x0D0,
 0x0D0, 0x473, 0x473, 0x473, 0x473, 0x473, 0x473, 0x073,
 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
 0x073, 0x073, 0x073, 0x0D0, 0x0D0, 0x0D0, 0x0D0, 0x004
};

isalnum (c) int c; {return (_is[c] & ALNUM );} /* 'a'-'z', 'A'-'Z', '0'-'9' */
isalpha (c) int c; {return (_is[c] & ALPHA );} /* 'a'-'z', 'A'-'Z' */
iscntrl (c) int c; {return (_is[c] & CNTRL );} /* 0-31, 127 */
isdigit (c) int c; {return (_is[c] & DIGIT );} /* '0'-'9' */
isgraph (c) int c; {return (_is[c] & GRAPH );} /* '!'-'~' */
islower (c) int c; {return (_is[c] & LOWER );} /* 'a'-'z' */
isprint (c) int c; {return (_is[c] & PRINT );} /* ' '-'~' */
ispunct (c) int c; {return (_is[c] & PUNCT );} /* !alnum && !cntrl && !space */
isspace (c) int c; {return (_is[c] & BLANK );} /* HT, LF, VT, FF, CR, ' ' */
isupper (c) int c; {return (_is[c] & UPPER );} /* 'A'-'Z' */
isxdigit(c) int c; {return (_is[c] & XDIGIT);} /* '0'-'9', 'a'-'f', 'A'-'F' */

/*
** itoa(n,s) - Convert n to characters in s 
*/
itoa(n,s) char *s; int n;
{int sign;
 char *ptr;

 ptr=s;
 if((sign=n)<0) n=-n;
 do
 {*ptr++ =n%10+'0';
 }while((n=n/10)>0);
 if(sign<0) *ptr++='-';
 *ptr='\0';
 reverse(s);
}

/*
** itoab(n,s,b) - Convert "unsigned" n to characters in s using base b.
**                NOTE: This is a non-standard function.
*/
itoab(n,s,b) int n; char *s; int b;
{char *ptr;
 int lowbit;
 ptr=s;
 b >>= 1;
 do
 {lowbit=n&1;
  n=(n>>1)&32767;
  *ptr=((n%b)<<1)+lowbit;
  if(*ptr<10) *ptr+='0'; 
  else        *ptr+=55;
  ++ptr;
 }while(n/=b);
 *ptr=0;
 reverse(s);
}

/*
** itod -- convert nbr to signed decimal string of width sz
**         right adjusted, blank filled; returns str
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
itod(nbr,str,sz) int nbr; char str[]; int sz;
{char sgn;

 if(nbr<0) {nbr=-nbr; sgn='-';}
 else sgn=' ';
      if(sz>0) str[--sz]=NULL;
 else if(sz<0) sz=-sz;
 else while(str[sz]!=NULL) ++sz;
 while(sz)
 {str[--sz]=(nbr%10+'0');
  if((nbr=nbr/10)==0) break;
 }
 if(sz) str[--sz]=sgn;
 while(sz>0) str[--sz]=' ';
 return str;
}

/*
** itoo -- converts nbr to octal string of length sz
**         right adjusted and blank filled, returns str
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
itoo(nbr,str,sz) int nbr; char str[]; int sz;
{int digit;

      if(sz>0) str[--sz]=0;
 else if(sz<0) sz=-sz;
 else while(str[sz]!=0) ++sz;
 while(sz)
 {digit=nbr&7;nbr=(nbr>>3)&8191;
  str[--sz]=digit+48;
  if(nbr==0) break;
 }
 while(sz) str[--sz]=' ';
 return str;
}

/*
** itou -- convert nbr to unsigned decimal string of width sz
**         right adjusted, blank filled; returns str
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/
itou(nbr,str,sz) int nbr; char str[]; int sz;
{int lowbit;

      if(sz>0) str[--sz]=NULL;
 else if(sz<0) sz=-sz;
 else while(str[sz]!=NULL) ++sz;
 while(sz)
 {lowbit=nbr&1;
  nbr=(nbr>>1)&32767;  /* divide by 2 */
  str[--sz]=((nbr%5)<<1)+lowbit+'0';
  if((nbr=nbr/5)==0) break;
 }
 while(sz) str[--sz]=' ';
 return str;
}

/*
** itox -- converts nbr to hex string of length sz
**         right adjusted and blank filled, returns str
**
**        if sz > 0 terminate with null byte
**        if sz = 0 find end of string
**        if sz < 0 use last byte for data
*/

itox(nbr,str,sz) int nbr; char str[]; int sz;
{int digit,offset;

      if(sz>0) str[--sz]=0;
 else if(sz<0) sz=-sz;
 else while(str[sz]!=0)	++sz;
 while(sz)
 {digit=nbr&15;
  nbr=nbr/16;
/*
  nbr=(nbr>>4)&4095; // 268435455; // 0xFFFFFFF
*/
  if(digit<10) offset=48;
  else         offset=55;
  str[--sz]=digit+offset;
  if(nbr==0) break;
 }
 while(sz) str[--sz]=' ';
 return str;
}

