/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <fnmatch.h>

#define	EOS	'\0'

static const char *rangematch(const char *pattern, char test, int nocase);

#define isslash(c) ((c) == '\\' || (c) == '/')

static const char *
find_slash(const char *s)
{
  while (*s)
  {
    if (isslash(*s))
      return s;
    s++;
  }
  return 0;
}

static const char *
rangematch(const char *pattern, char test, int nocase)
{
  char c, c2;
  int negate, ok;

  if ((negate = (*pattern == '!')))
    ++pattern;

  for (ok = 0; (c = *pattern++) != ']';)
  {
    if (c == 0)
      return 0;			/* illegal pattern */
    if (*pattern == '-' && (c2 = pattern[1]) != 0 && c2 != ']')
    {
      if (c <= test && test <= c2)
	ok = 1;
      if (nocase && toupper(c) <= toupper(test) && toupper(test) <= toupper(c2))
	ok = 1;
      pattern += 2;
    }
    else if (c == test)
      ok = 1;
    else if (nocase && (toupper(c) == toupper(test)))
      ok = 1;
  }
  return ok == negate ? NULL : pattern;
}

int
fnmatch(const char *pattern, const char *string, int flags)
{
  char c;
  char test;

  for (;;)
    switch ((c = *pattern++))
    {
    case 0:
      return *string == 0 ? 0 : FNM_NOMATCH;
      
    case '?':
      if ((test = *string++) == 0 ||
	  (isslash(test) && (flags & FNM_PATHNAME)))
	return(FNM_NOMATCH);
      break;
      
    case '*':
      c = *pattern;
      /* collapse multiple stars */
      while (c == '*')
	c = *++pattern;

      /* optimize for pattern with * at end or before / */
      if (c == 0)
	if (flags & FNM_PATHNAME)
	  return find_slash(string) ? FNM_NOMATCH : 0;
	else
	  return 0;
      else if (isslash(c) && flags & FNM_PATHNAME)
      {
	if ((string = find_slash(string)) == NULL)
	  return FNM_NOMATCH;
	break;
      }

      /* general case, use recursion */
      while ((test = *string) != 0)
      {
	if (fnmatch(pattern, string, flags) == 0)
	  return(0);
	if (isslash(test) && flags & FNM_PATHNAME)
	  break;
	++string;
      }
      return FNM_NOMATCH;
      
    case '[':
      if ((test = *string++) == 0 ||
	  (isslash(test) && flags & FNM_PATHNAME))
	return FNM_NOMATCH;
      if ((pattern = rangematch(pattern, test, flags & FNM_NOCASE)) == NULL)
	return FNM_NOMATCH;
      break;
      
    case '\\':
      if (!(flags & FNM_NOESCAPE) && pattern[1] && strchr("*?[\\", pattern[1]))
      {
	if ((c = *pattern++) == 0)
	{
	  c = '\\';
	  --pattern;
	}
	if (c != *string++)
	  return FNM_NOMATCH;
	break;
      }
      /* FALLTHROUGH */
      
    default:
      if (isslash(c) && isslash(*string))
      {
	string++;
	break;
      }
      if (flags & FNM_NOCASE)
      {
	if (toupper(c) != toupper(*string++))
	  return FNM_NOMATCH;
      }
      else
      {
	if (c != *string++)
	  return FNM_NOMATCH;
      }
      break;
    }
}
