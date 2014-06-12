#include<menuet/os.h>
#include<unistd.h>

void _exit(int code)
{
 __asm__ __volatile__("int $0x40"::"a"(-1));
 for(;;);
}

#include <libc/stubs.h>
#include <io.h>
#include <unistd.h>
#include <stdlib.h>
#include <crt0.h>
#include <libc/farptrgs.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>

#define ds _my_ds()


extern char __menuet__app_param_area[];
extern char __menuet__app_path_area[];

static void * c1xmalloc(size_t s)
{
 void *q = malloc(s);
 if (q == 0)
 {
#define err(x)
  err("No memory to gather arguments\r\n");
  _exit(1);
 }
 return q;
}

static int atohex(char *s)
{
  int rv = 0;
  while (*s)
  {
    int v = *s - '0';
    if (v > 9)
      v -= 7;
    v &= 15; /* in case it's lower case */
    rv = rv*16 + v;
    s++;
  }
  return rv;
}

typedef struct Arg {
  char *arg;
  char **arg_globbed;
  struct ArgList *arg_file;
  struct Arg *next;
  int was_quoted;
} Arg;

typedef struct ArgList {
  int argc;
  Arg **argv;
} ArgList;

static Arg *new_arg(void)
{
  Arg *a = (Arg *)c1xmalloc(sizeof(Arg));
  memset(a, 0, sizeof(Arg));
  return a;
}

static void delete_arglist(ArgList *al);

static void delete_arg(Arg *a)
{
  if (a->arg) free(a->arg);
  if (a->arg_globbed)
  {
    int i;
    for (i=0; a->arg_globbed[i]; i++)
      free(a->arg_globbed[i]);
    free(a->arg_globbed);
  }
  if (a->arg_file)
    delete_arglist(a->arg_file);
  free(a);
}

static ArgList * new_arglist(int count)
{
  ArgList *al = (ArgList *)c1xmalloc(sizeof(ArgList));
  al->argc = count;
  al->argv = (Arg **)c1xmalloc((count+1)*sizeof(Arg *));
  memset(al->argv, 0, (count+1)*sizeof(Arg *));
  return al;
}

static void delete_arglist(ArgList *al)
{
  int i;
  for (i=0; i<al->argc; i++)
    delete_arg(al->argv[i]);
  free(al->argv);
  free(al);
}

static char * parse_arg(char *bp, char *last, int unquote, size_t *len, int *was_quoted)
{
  char *ep = bp, *epp = bp;
  int quote=0;

  while ((quote || !isspace(*(unsigned char *)ep)) && ep < last)
  {
    if (quote && *ep == quote)
    {
      quote = 0;
      if (!unquote)
	*epp++ = *ep;
      ep++;
    }
    else if (!quote && (*ep == '\'' || *ep == '"'))
    {
      quote = *ep++;
      if (!unquote)
	*epp++ = quote;
    }
    else if (*ep == '\\' && strchr("'\"", ep[1]) && ep < last-1)
    {
      if (!unquote)
	*epp++ = *ep;
      ep++;
      *epp++ = *ep++;
      /* *was_quoted = 1;  - This makes no sense. */
    }
    else
    {
      if ((quote && (strchr("[?*", *ep) || strncmp(ep, "...", 3) == 0))
	  && unquote)
	*was_quoted = 1;
      *epp++ = *ep++;
    }
  }

  *len = epp - bp;
  return ep;
}

static ArgList * parse_bytes(char *bytes, int length, int unquote)
{
  int largc, i;
  Arg *a, **anext, *afirst;
  ArgList *al;
  char *bp=bytes, *ep, *last=bytes+length;

  anext = &afirst;
  largc = 0;
  while (bp<last)
  {
    size_t arg_len;
    while (isspace(*(unsigned char *)bp) && bp < last)
      bp++;
    if (bp == last)
      break;
    *anext = a = new_arg();
    ep = parse_arg(bp, last, unquote, &arg_len, &(a->was_quoted));
    anext = &(a->next);
    largc++;
    a->arg = (char *)c1xmalloc(arg_len+1);
    memcpy(a->arg, bp, arg_len);
    a->arg[arg_len] = 0;
    bp = ep+1;
  }
  al = new_arglist(largc);
  for (i=0, a=afirst; i<largc; i++, a=a->next)
    al->argv[i] = a;
  return al;
}

static ArgList * parse_print0(char *bytes, int length)
{
  int largc, i;
  Arg *a, **anext, *afirst;
  ArgList *al;
  char *bp=bytes, *ep, *last=bytes+length;

  anext = &afirst;
  largc = 0;
  while (bp<last)
  {
    size_t arg_len = strlen(bp);
    ep = bp;
    bp += arg_len + 1;
    *anext = a = new_arg();
    a->was_quoted = 1;
    anext = &(a->next);
    largc++;
    a->arg = (char *)c1xmalloc(arg_len+1);
    memcpy(a->arg, ep, arg_len);
    a->arg[arg_len] = 0;
  }
  al = new_arglist(largc);
  for (i=0, a=afirst; i<largc; i++, a=a->next)
    al->argv[i] = a;
  return al;
}

static int count_args(ArgList *al)
{
  int i, r=0;
  for (i=0; i<al->argc; i++)
  {
    int j;
    if (al->argv[i]->arg_globbed)
    {
      for (j=0; al->argv[i]->arg_globbed[j]; j++);
      r += j;
    }
    else if (al->argv[i]->arg_file)
    {
      r += count_args(al->argv[i]->arg_file);
    }
    else
    {
      r++;
    }
  }
  return r;
}

static char ** fill_args(char **largv, ArgList *al)
{
  int i;
  for (i=0; i<al->argc; i++)
  {
    int j;
    if (al->argv[i]->arg_globbed)
    {
      for (j=0; al->argv[i]->arg_globbed[j]; j++)
      {
        *largv++ = al->argv[i]->arg_globbed[j];
        al->argv[i]->arg_globbed[j] = 0;
      }
    }
    else if (al->argv[i]->arg_file)
    {
      largv = fill_args(largv, al->argv[i]->arg_file);
    }
    else
    {
      *largv++ = al->argv[i]->arg;
      al->argv[i]->arg = 0;
    }
  }
  return largv;
}

static void expand_response_files(ArgList *al)
{
  int i, f;
  for (i=0; i<al->argc; i++)
  {
    if (! al->argv[i]->was_quoted && al->argv[i]->arg[0] == '@')
      if ((f = _open(al->argv[i]->arg+1, O_RDONLY)) >= 0)
      {
	char *bytes;
	int len, st_size;
	st_size = lseek(f, 0L, SEEK_END);
	lseek(f, 0L, SEEK_SET);
        if (st_size < 0)
	  st_size = 0;
        bytes = (char *)c1xmalloc(st_size+1);
        len = _read(f, bytes, st_size);
        if (len < 0)
	  len = 0;
        _close(f);
        /* if the last character is ^Z, remove it */
        if (len > 0 && bytes[len-1] == 0x1a)
          len--;
	/* assume 'find -print0' if the last char is a '\0' */
	if (len > 0 && bytes[len-1] == '\0')
          al->argv[i]->arg_file = parse_print0(bytes, len);
	else
          al->argv[i]->arg_file = parse_bytes(bytes, len, (_crt0_startup_flags & _CRT0_FLAG_KEEP_QUOTES) == 0);
        expand_response_files(al->argv[i]->arg_file);
	free(bytes);
      }
  }
}

static void expand_wildcards(ArgList *al)
{
  int i;
  for (i=0; i<al->argc; i++)
  {
    if (al->argv[i]->arg_file)
      expand_wildcards(al->argv[i]->arg_file);
    else if (!(al->argv[i]->was_quoted))
    {
      al->argv[i]->arg_globbed = __crt0_glob_function(al->argv[i]->arg);
    }
  }
}

static void add_arg(const char* arg, const char* end)
{
	char* res;
	__crt0_argv = realloc(__crt0_argv, 4*(++__crt0_argc));
	res = malloc(end-arg+1);
	if (!__crt0_argv || !res) _exit(1);
	__crt0_argv[__crt0_argc-1] = res;
	while (arg < end)
	{
		if (arg[0] == '"' && arg[1] == '"') ++arg;
		*res++ = *arg++;
	}
	*res = 0;
}

void __crt0_setup_arguments(void)
{
#if 0
// не будем страдать фигнёй...
  ArgList *arglist;
  char *argv0;
  int prepend_argv0 = 1;
  int should_expand_wildcards = 1;
  char *proxy_v = 0;
  argv0="menuet.app";
  /*
  ** Next, scan dos's command line.
  */
  {
    char doscmd[128];
    memcpy(doscmd+1,__menuet__app_param_area,128);
    arglist = parse_bytes(doscmd+1, doscmd[0] & 0x7f,
			  (_crt0_startup_flags & _CRT0_FLAG_KEEP_QUOTES) == 0);
  }

  /*
  **  Now, expand response files
  */
  if (!(_crt0_startup_flags & _CRT0_FLAG_DISALLOW_RESPONSE_FILES))
    expand_response_files(arglist);

  /*
  **  Now, expand wildcards
  */

  if (should_expand_wildcards)
    expand_wildcards(arglist);

  __crt0_argc = prepend_argv0 + count_args(arglist);
  __crt0_argv = (char **)c1xmalloc((__crt0_argc+1) * sizeof(char *));
  if (prepend_argv0)
    __crt0_argv[0] = argv0;
  *fill_args(__crt0_argv+prepend_argv0, arglist) = 0;
#else
// ...а просто разберём командную строку. - diamond
	char* ptr;
	char* cur_arg=NULL;
	int bInQuote=0;
	add_arg(__menuet__app_path_area,
		__menuet__app_path_area + strlen(__menuet__app_path_area));
	for (ptr=__menuet__app_param_area;*ptr && ptr<__menuet__app_param_area+256;ptr++)
	{
		if (*ptr == ' ' || *ptr == '\t')
		{
			if (cur_arg && !bInQuote)
			{
				add_arg(cur_arg,ptr);
				cur_arg = NULL;
			}
			continue;
		}
		if (*ptr == '"')
		{
			if (ptr[1] == '"')
			{if (!cur_arg) cur_arg=ptr;ptr++;}
			else
			{
				if (cur_arg)
				{
					add_arg(cur_arg,ptr);
					if (bInQuote)
					{
						bInQuote = 0;
						cur_arg = NULL;
						continue;
					}
				}
				bInQuote = 1;
				cur_arg = ptr+1;
			}
			continue;
		}
		if (!cur_arg) cur_arg = ptr;
	}
	if (cur_arg) add_arg(cur_arg,ptr);
#endif
}
