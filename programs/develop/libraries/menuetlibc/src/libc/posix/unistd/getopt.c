/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libc/unconst.h>

int opterr = 1,	optind = 1, optopt = 0;
char *optarg = 0;

#define	BADCH	(int)'?'
#define	EMSG	""

int
getopt(int nargc, char *const nargv[], const char *ostr)
{
  static const char *place = EMSG;	/* option letter processing */
  char *oli;			/* option letter list index */
  char *p;

  if (!*place)
  {
    if (optind >= nargc || *(place = nargv[optind]) != '-')
    {
      place = EMSG;
      return(EOF);
    }
    if (place[1] && *++place == '-')
    {
      ++optind;
      place = EMSG;
      return(EOF);
    }
  }

  if ((optopt = (int)*place++) == (int)':'
      || !(oli = strchr(ostr, optopt)))
  {
    /*
     * if the user didn't specify '-' as an option,
     * assume it means EOF.
     */
    if (optopt == (int)'-')
      return EOF;
    if (!*place)
      ++optind;
    if (opterr)
    {
      if (!(p = strrchr(*nargv, '/')))
	p = *nargv;
      else
	++p;
      fprintf(stderr, "%s: illegal option -- %c\n", p, optopt);
    }
    return BADCH;
  }
  if (*++oli != ':')
  {		/* don't need argument */
    optarg = NULL;
    if (!*place)
      ++optind;
  }
  else
  {				/* need an argument */
    if (*place)			/* no white space */
      optarg = unconst(place, char *);
    else if (nargc <= ++optind)
    { /* no arg */
      place = EMSG;
      if (!(p = strrchr(*nargv, '/')))
	p = *nargv;
      else
	++p;
      if (opterr)
	fprintf(stderr, "%s: option requires an argument -- %c\n",
		p, optopt);
      return BADCH;
    }
    else			/* white space */
      optarg = nargv[optind];
    place = EMSG;
    ++optind;
  }
  return optopt;		/* dump back option letter */
}
