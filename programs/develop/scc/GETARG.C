#include <stdio.h>
/*
** Get command line argument. 
** Entry: n    = Number of the argument.
**        s    = Destination string pointer.
**        size = Size of destination string.
**        argc = Argument count from main().
**        argv = Argument vector(s) from main().
** Returns number of characters moved on success,
** else EOF.
*/
getarg(n,s,size,argc,argv)
 int n; char *s; int size,argc,argv[];
{char *str;
 int i;
 
 if(n<0 | n>=argc)
 {*s=NULL;
  return EOF;
 }
 i=0;
 str=argv[n];
 while(i<size)
 {if((s[i]=str[i])==NULL) break;
  ++i;
 }
 s[i]=NULL;
 return i;
}
