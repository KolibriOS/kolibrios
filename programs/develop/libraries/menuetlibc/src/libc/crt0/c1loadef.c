/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <crt0.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

void __crt0_load_environment_file(char *app_name)
{
  int djgpp_env;
  char *djgpp_var = getenv("DJGPP");

  if (djgpp_var)
  {
    djgpp_env = _open(djgpp_var, O_RDONLY);
    if (djgpp_env >= 0)
    {
      char *file;
      char base[120], *bp, *a0p, *tb;
      int this_prog = 1;
      int fsize = lseek(djgpp_env, 0L, SEEK_END);

      file = (char *)malloc(fsize+2);
      if (file == 0)
	return;
      lseek(djgpp_env, 0L, 0);
      _read(djgpp_env, file, fsize);
      _close(djgpp_env);
      if (file[fsize-1] == '\n')
      {
	file[fsize] = 0;
      }
      else
      {
	file[fsize] = '\n';
	file[fsize+1] = 0;
      }
      tb = file;

      base[0] = '[';
      bp = app_name;
      for (a0p = bp; *a0p; a0p++)
	if (strchr("\\/:", *a0p))
	  bp = a0p+1;
      for (a0p=base+1; *bp && *bp != '.';)
	*a0p++ = tolower(*bp++);
      *a0p++ = ']';
      *a0p++ = 0;

      bp = tb;
      while (1)
      {
	tb = bp;
	while (*tb && (*tb == '\n' || *tb == '\r'))
	  tb++;
	bp = tb;
	while (*bp && *bp != '\n' && *bp != '\r')
	  bp++;
	if (*bp == 0)
	  break;
	*bp++ = 0;
	if (tb[0] == 0 || tb[0] == '#')
	  continue;
	if (tb[0] == '[')
	{
	  if (strcmp(tb, base) == 0)
	    this_prog = 1;
	  else
	    this_prog = 0;
	}
	else
	{
	  if (this_prog)
	  {
	    char *buf = alloca(fsize);
	    char *tb2 = buf;
	    char *sp=tb, *dp=tb2;
	    while (*sp != '=')
	      *dp++ = *sp++;
	    if (*tb2 == '+')	/* non-overriding */
	    {
	      *dp = 0;
	      tb2++;
	      if (getenv(tb2))
		continue;	/* while scanning bytes */
	    }
	    *dp++ = *sp++;	/* copy the '=' */
	    while (*sp)
	    {
	      if (*sp == '%')
	      {
		char *pp;
		if (sp[1] == '%')
		{
		  *dp++ = '%';
		  sp += 2;
		}
		else
		{
		  char ps, *e, *dirend;
		  int dirpart=0, apsemi=0;
		  int mapup=0, maplow=0, mapfs=0, mapbs=0;
		  while (strchr(":;/\\<>", sp[1]))
		  {
		    switch (sp[1])
		    {
		    case ':':  dirpart=1; break;
		    case ';':  apsemi=1;  break;
		    case '/':  mapfs=1;   break;
		    case '\\': mapbs=1;   break;
		    case '<':  mapup=1;   break;
		    case '>':  maplow=1;  break;
		    }
		    sp++;
		  }
		  for (pp=sp+1; *pp && *pp != '%'; pp++);
		  ps = *pp;
		  *pp = 0;
		  e = getenv(sp+1);
		  dirend = dp;
		  if (e)
		  {
		    while (*e)
		    {
		      char ec = *e++;
		      if (strchr("\\/:", ec))
			dirend=dp;
		      if (mapup) ec = toupper(ec);
		      if (maplow) ec = tolower(ec);
		      if (mapfs && ec == '\\') ec = '/';
		      if (mapbs && ec == '/') ec = '\\';
		      *dp++ = ec;
		    }
		  }
		  if (dirpart)
		    dp = dirend;
		  if (apsemi && e)
		    *dp++ = ';';
		  if (ps == 0)
		    break;
		  sp = pp+1;
		}
	      }
	      else
		*dp++ = *sp++;
	    }
	    *dp++ = 0;
	    putenv(tb2);
	  }
	}
      }
      free(file);
    }
  }
}
