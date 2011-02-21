/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fnmatch.h>
#include <dir.h>
#include <glob.h>

typedef struct Save {
  struct Save *prev;
  char *entry;
} Save;

static Save *save_list;
static int save_count;
static int flags;
static int (*errfunc)(const char *epath, int eerno);
static char *pathbuf;
static int wildcard_nesting;
static char use_lower_case;
static char slash;

static int glob2(const char *pattern, char *epathbuf);
static int add(const char *path);
static int str_compare(const void *va, const void *vb);

static int
add(const char *path)
{
  Save *sp;
  for (sp=save_list; sp; sp=sp->prev)
    if (strcmp(sp->entry, path) == 0)
      return 0;
  sp = (Save *)malloc(sizeof(Save));
  if (sp == 0)
    return 1;
  sp->entry = (char *)malloc(strlen(path)+1);
  if (sp->entry == 0)
  {
    free(sp);
    return 1;
  }
/*  printf("add: `%s'\n", path); */
  strcpy(sp->entry, path);
  sp->prev = save_list;
  save_list = sp;
  save_count++;
  return 0;
}

static int
glob_dirs(const char *rest, char *epathbuf, int first) /* rest is ptr to null or ptr after slash, bp after slash */
{
  struct ffblk ff;
  int done;

/*  printf("glob_dirs[%d]: rest=`%s' %c epathbuf=`%s' %c pathbuf=`%s'\n",
	 wildcard_nesting, rest, *rest, epathbuf, *epathbuf, pathbuf); */

  if (first)
  {
    if (*rest)
    {
      glob2(rest, epathbuf);
    }
    else
    {
      char sl = epathbuf[-1];
      *epathbuf = 0;
/*      printf("end, checking `%s'\n", pathbuf); */
      if (epathbuf == pathbuf)
      {
	epathbuf[0] = '.';
	epathbuf[1] = 0;
      }
      else
	epathbuf[-1] = 0;
      if (__file_exists(pathbuf))
	add(pathbuf);
      epathbuf[-1] = sl;
    }
  }

  strcpy(epathbuf, "*.*");
  done = findfirst(pathbuf, &ff, FA_DIREC);
  while (!done)
  {
    if ((ff.ff_name[0] != '.') && (ff.ff_attrib & FA_DIREC))
    {
      int i;
      char *tp;
      if (use_lower_case)
	for (i=0; ff.ff_name[i] && i<13; i++)
	  ff.ff_name[i] = tolower(ff.ff_name[i]);

/*      printf("found `%s' `%s'\n", pathbuf, ff.ff_name); */

      strcpy(epathbuf, ff.ff_name);
      tp = epathbuf + strlen(epathbuf);
      *tp++ = slash;
      *tp = 0;

      wildcard_nesting++;
      if (*rest)
      {
	glob2(rest, tp);
      }
      else
      {
	if (!(flags & GLOB_MARK))
	  tp[-1] = 0;
	add(pathbuf);
	tp[-1] = slash;
      }
      *tp = 0;
      glob_dirs(rest, tp, 0);
      wildcard_nesting--;
    }
    done = findnext(&ff);
  }
  return 0;
}

static int
glob2(const char *pattern, char *epathbuf) /* both point *after* the slash */
{
  const char *pp, *pslash;
  char *bp;
  struct ffblk ff;
  char *my_pattern;
  int done;

  if (strcmp(pattern, "...") == 0)
  {
    return glob_dirs(pattern+3, epathbuf, 1);
  }
  if (strncmp(pattern, "...", 3) == 0 && (pattern[3] == '\\' || pattern[3] == '/'))
  {
    slash = pattern[3];
    return glob_dirs(pattern+4, epathbuf, 1);
  }

  *epathbuf = 0;
  /* copy as many non-wildcard segments as possible */
  pp = pattern;
  bp = epathbuf;
  pslash = bp-1;
  while (1)
  {
    if (*pp == ':' || *pp == '\\' || *pp == '/')
    {
      pslash = bp;
      if (strcmp(pp+1, "...") == 0
	  || (strncmp(pp+1, "...", 3) == 0 && (pp[4] == '/' || pp[4] == '\\')))
      {
	if (*pp != ':')
	  slash = *pp;
/*	printf("glob2: dots at `%s'\n", pp); */
	*bp++ = *pp++;
	break;
      }
    }

    else if (*pp == '*' || *pp == '?' || *pp == '[')
    {
      if (pslash > pathbuf)
	strncpy(epathbuf, pattern, pslash - pathbuf);
      pp = pattern + (pslash - epathbuf) + 1;
      bp = epathbuf + (pslash - epathbuf) + 1;
      break;
    }

    else if (*pp == 0)
    {
      break;
    }

    else if (islower(*pp))
      use_lower_case = 1;
    else if (isupper(*pp))
      use_lower_case = 0;

    *bp++ = *pp++;
  }
  *bp = 0;

  if (*pp == 0) /* end of pattern? */
  {
    if (wildcard_nesting==0 || __file_exists(pathbuf))
      add(pathbuf);
    return 0;
  }
/*  printf("glob2: initial segment is `%s'\n", pathbuf); */
  if (wildcard_nesting)
  {
    char s = bp[-1];
    bp[-1] = 0;
    if (!__file_exists(pathbuf))
      return 0;
    bp[-1] = s;
  }

  for (pslash = pp; *pslash && *pslash != '\\' && *pslash != '/';  pslash++)
  {
    if (islower(*pslash))
      use_lower_case = 1;
    else if (isupper(*pslash))
      use_lower_case = 0;
  }
  if (*pslash)
    slash = *pslash;
  my_pattern = (char *)alloca(pslash - pp + 1);
  if (my_pattern == 0)
    return 0;
  strncpy(my_pattern, pp, pslash - pp);
  my_pattern[pslash-pp] = 0;

/*  printf("glob2: `%s' `%s'\n", pathbuf, my_pattern); */

  if (strcmp(my_pattern, "...") == 0)
  {
    glob_dirs(*pslash ? pslash+1 : pslash, bp, 1);
    return 0;
  }

  strcpy(bp, "*.*");

  done = findfirst(pathbuf, &ff, FA_RDONLY|FA_SYSTEM|FA_DIREC|FA_ARCH);
  while (!done)
  {
    int i;
    if (ff.ff_name[0] != '.')
    {
      if (use_lower_case)
	for (i=0; ff.ff_name[i] && i<13; i++)
	  ff.ff_name[i] = tolower(ff.ff_name[i]);
      if (fnmatch(my_pattern, ff.ff_name, FNM_NOESCAPE|FNM_PATHNAME|FNM_NOCASE) == 0)
      {
	strcpy(bp, ff.ff_name);
	if (*pslash)
	{
	  char *tp = bp + strlen(bp);
	  *tp++ = *pslash;
	  *tp = 0;
/*	  printf("nest: `%s' `%s'\n", pslash+1, pathbuf); */
	  wildcard_nesting++;
	  glob2(pslash+1, tp);
	  wildcard_nesting--;
	}
	else
	{
/*	  printf("ffmatch: `%s' matching `%s', add `%s'\n",
		 ff.ff_name, my_pattern, pathbuf); */
	  if (ff.ff_attrib & FA_DIREC & (flags & GLOB_MARK))
	  {
	    bp[strlen(bp)] = slash;
	    bp[strlen(bp)+1] = 0;
	  }
	  add(pathbuf);
	}
      }
    }
    done = findnext(&ff);
  } 

  return 0;
}

static int
str_compare(const void *va, const void *vb)
{
  return strcmp(*(char * const *)va, *(char * const *)vb);
}

int
glob(const char *_pattern, int _flags, int (*_errfunc)(const char *_epath, int _eerrno), glob_t *_pglob)
{
  char path_buffer[2000];
  int l_ofs, l_ptr;

  pathbuf = path_buffer+1;
  flags = _flags;
  errfunc = _errfunc;
  wildcard_nesting = 0;
  save_count = 0;
  save_list = 0;
  use_lower_case = 1;
  slash = '/';

  glob2(_pattern, pathbuf);

  if (save_count == 0)
  {
    if (flags & GLOB_NOCHECK)
      add(_pattern);
    else
      return GLOB_NOMATCH;
  }

  if (flags & GLOB_DOOFFS)
    l_ofs = _pglob->gl_offs;
  else
    l_ofs = 0;

  if (flags & GLOB_APPEND)
  {
    _pglob->gl_pathv = (char **)realloc(_pglob->gl_pathv, (l_ofs + _pglob->gl_pathc + save_count + 1) * sizeof(char *));
    if (_pglob->gl_pathv == 0)
      return GLOB_ERR;
    l_ptr = l_ofs + _pglob->gl_pathc;
  }
  else
  {
    _pglob->gl_pathv = (char* *)malloc((l_ofs + save_count + 1) * sizeof(char *));
    if (_pglob->gl_pathv == 0)
      return GLOB_ERR;
    l_ptr = l_ofs;
    if (l_ofs)
      memset(_pglob->gl_pathv, 0, l_ofs * sizeof(char *));
  }

  l_ptr += save_count;
  _pglob->gl_pathv[l_ptr] = 0;
  while (save_list)
  {
    Save *s = save_list;
    l_ptr --;
    _pglob->gl_pathv[l_ptr] = save_list->entry;
    save_list = save_list->prev;
    free(s);
  }
  if (!(flags & GLOB_NOSORT))
    qsort(_pglob->gl_pathv + l_ptr, save_count, sizeof(char *), str_compare);

  _pglob->gl_pathc = l_ptr + save_count;

  return 0;
}
