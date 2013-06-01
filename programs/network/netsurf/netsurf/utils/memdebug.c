/** \file
 * Heap debugging functions (implementation).
 *
 * Based on memdebug.c from curl (see below), with the following modifications:
 *
 * - renamed functions from curl_ to memdebug_
 * - added memdebug_strndup
 * - added guard bytes before and after each block to help detect overflows
 * - if a guard byte is corrupted during free, dumps the DA to file
 */

/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2004, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * $Id: memdebug.c,v 1.1 2004/07/28 22:35:02 bursa Exp $
 ***************************************************************************/

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#ifdef riscos
#include <unixlib/local.h>

#include "oslib/os.h"
#include "oslib/osfile.h"
#endif

#include "memdebug.h"

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define MAGIC 0x34343434
#define GUARD 0x34

#if defined(riscos) && !defined(__ELF__)
extern int __dynamic_num;
#endif

struct memdebug {
  size_t size;
  unsigned int magic;
  double mem[1];
  /* I'm hoping this is the thing with the strictest alignment
   * requirements.  That also means we waste some space :-( */
};

/*
 * Note that these debug functions are very simple and they are meant to
 * remain so. For advanced analysis, record a log file and write perl scripts
 * to analyze them!
 *
 * Don't use these with multithreaded test programs!
 */

#define logfile memdebug_debuglogfile
FILE *memdebug_debuglogfile;
static bool memlimit; /* enable memory limit */
static long memsize;  /* set number of mallocs allowed */

/* this sets the log file name */
void memdebug_memdebug(const char *logname)
{
  if(logname)
    logfile = fopen(logname, "w");
  else
    logfile = stderr;
}

/* This function sets the number of malloc() calls that should return
   successfully! */
void memdebug_memlimit(long limit)
{
  memlimit = true;
  memsize = limit;
}

/* returns true if this isn't allowed! */
static bool countcheck(const char *func, int line, const char *source)
{
  /* if source is NULL, then the call is made internally and this check
     should not be made */
  if(memlimit && source) {
    if(!memsize) {
      if(logfile && source)
        fprintf(logfile, "LIMIT %s:%d %s reached memlimit\n",
                source, line, func);
      if(source)
        fprintf(stderr, "LIMIT %s:%d %s reached memlimit\n",
                source, line, func);
      return true; /* RETURN ERROR! */
    }
    else
      memsize--; /* countdown */

    /* log the countdown */
    if(logfile && source)
      fprintf(logfile, "LIMIT %s:%d %ld ALLOCS left\n",
              source, line, memsize);

  }

  return false; /* allow this */
}

void *memdebug_malloc(size_t wantedsize, int line, const char *source)
{
  struct memdebug *mem;
  size_t size;

  if(countcheck("malloc", line, source))
    return NULL;

  /* alloc at least 64 bytes */
  size = sizeof(struct memdebug)+wantedsize + 8;

  mem=(struct memdebug *)(malloc)(size);
  if(mem) {
    unsigned int i;
    /* fill memory with junk */
    memset(mem->mem, 0xA5, wantedsize);
    mem->size = wantedsize;
    mem->magic = MAGIC;
    for (i = 0; i != 8; i++)
      ((char *) mem->mem)[wantedsize + i] = GUARD;
  }

  if(logfile && source)
    fprintf(logfile, "MEM %s:%d malloc(%zu) = %p\n",
            source, line, wantedsize, mem ? mem->mem : 0);
  return (mem ? mem->mem : NULL);
}

void *memdebug_calloc(size_t wanted_elements, size_t wanted_size,
                    int line, const char *source)
{
  struct memdebug *mem;
  size_t size, user_size;

  if(countcheck("calloc", line, source))
    return NULL;

  /* alloc at least 64 bytes */
  user_size = wanted_size * wanted_elements;
  size = sizeof(struct memdebug) + user_size + 8;

  mem = (struct memdebug *)(malloc)(size);
  if(mem) {
    unsigned int i;
    /* fill memory with zeroes */
    memset(mem->mem, 0, user_size);
    mem->size = user_size;
    mem->magic = MAGIC;
    for (i = 0; i != 8; i++)
      ((char *) mem->mem)[mem->size + i] = GUARD;
  }

  if(logfile && source)
    fprintf(logfile, "MEM %s:%d calloc(%zu,%zu) = %p\n",
            source, line, wanted_elements, wanted_size, mem ? mem->mem : 0);
  return (mem ? mem->mem : NULL);
}

char *memdebug_strdup(const char *str, int line, const char *source)
{
  char *mem;
  size_t len;

  assert(str != NULL);

  if(countcheck("strdup", line, source))
    return NULL;

  len=strlen(str)+1;

  mem=memdebug_malloc(len, 0, NULL); /* NULL prevents logging */
  if (mem)
  memcpy(mem, str, len);

  if(logfile)
    fprintf(logfile, "MEM %s:%d strdup(%p) (%zu) = %p\n",
            source, line, str, len, mem);

  return mem;
}

char *memdebug_strndup(const char *str, size_t size, int line, const char *source)
{
  char *mem;
  size_t len;

  assert(str != NULL);

  if(countcheck("strndup", line, source))
    return NULL;

  len=strlen(str)+1;
  if (size < len - 1)
    len = size + 1;

  mem=memdebug_malloc(len, 0, NULL); /* NULL prevents logging */
  if (mem) {
  memcpy(mem, str, len);
  mem[len - 1] = 0;
  }

  if(logfile)
    fprintf(logfile, "MEM %s:%d strndup(%p, %zd) (%zu) = %p\n",
            source, line, str, size, len, mem);

  return mem;
}

/* We provide a realloc() that accepts a NULL as pointer, which then
   performs a malloc(). In order to work with ares. */
void *memdebug_realloc(void *ptr, size_t wantedsize,
                     int line, const char *source)
{
  unsigned int i;
  struct memdebug *mem=NULL;

  size_t size = sizeof(struct memdebug)+wantedsize+8;

  if(countcheck("realloc", line, source))
    return NULL;

  if(ptr) {
    mem = (struct memdebug *)(void *)
		((char *)ptr - offsetof(struct memdebug, mem));
  }

  if(logfile) {
    if (mem && mem->magic != MAGIC)
      fprintf(logfile, "MAGIC match failed!\n");
    for (i = 0; mem && i != 8; i++)
      if (((char *) mem->mem)[mem->size + i] != GUARD)
        fprintf(logfile, "GUARD %u match failed!\n", i);
    fprintf(logfile, "MEM %s:%d realloc(%p, %zu) = ",
            source, line, ptr, wantedsize);
    fflush(logfile);
  }

  mem=(struct memdebug *)(realloc)(mem, size);
  if(logfile)
    fprintf(logfile, "%p\n", mem?mem->mem:NULL);

  if(mem) {
    mem->size = wantedsize;
    mem->magic = MAGIC;
    for (i = 0; i != 8; i++)
      ((char *) mem->mem)[wantedsize + i] = GUARD;
    return mem->mem;
  }

  return NULL;
}

void memdebug_free(void *ptr, int line, const char *source)
{
  unsigned int i;
  struct memdebug *mem;

  if (!ptr)
    return;

  assert(ptr != NULL);

  mem = (struct memdebug *)(void *)
		((char *)ptr - offsetof(struct memdebug, mem));
  if(logfile) {
    fprintf(logfile, "MEM %s:%d free(%p)\n", source, line, ptr);
    if (mem->magic != MAGIC) {
      fprintf(logfile, "MAGIC match failed!\n");
#ifdef riscos
  #ifndef __ELF__
      if (__dynamic_num != -1) {
        int size;
        byte *base_address;
        xosdynamicarea_read(__dynamic_num, &size, &base_address,
            0, 0, 0, 0, 0);
        fprintf(logfile, "saving DA %i %p %x\n", __dynamic_num, base_address,
            size);
        xosfile_save("core", (bits) base_address, 0, base_address,
            base_address + size);
      }
  #else
      __unixlib_write_coredump(NULL);
  #endif
#endif
    }
    fflush(logfile);
    for (i = 0; i != 8; i++)
      if (((char *) mem->mem)[mem->size + i] != GUARD)
        fprintf(logfile, "GUARD %u match failed!\n", i);
    fflush(logfile);
  }

  /* destroy  */
  memset(mem->mem, 0x13, mem->size);
  mem->magic = 0x13131313;
  for (i = 0; i != 8; i++)
    ((char *) mem->mem)[mem->size + i] = 0x13;

  /* free for real */
  (free)(mem);
}

int memdebug_socket(int domain, int type, int protocol, int line,
                const char *source)
{
  int sockfd=(socket)(domain, type, protocol);
  if(logfile && (sockfd!=-1))
    fprintf(logfile, "FD %s:%d socket() = %d\n",
            source, line, sockfd);
  return sockfd;
}

int memdebug_accept(int s, void *saddr, void *saddrlen,
                int line, const char *source)
{
  struct sockaddr *addr = (struct sockaddr *)saddr;
  socklen_t *addrlen = (socklen_t *)saddrlen;
  int sockfd=(accept)(s, addr, addrlen);
  if(logfile)
    fprintf(logfile, "FD %s:%d accept() = %d\n",
            source, line, sockfd);
  return sockfd;
}

/* this is our own defined way to close sockets on *ALL* platforms */
int memdebug_sclose(int sockfd, int line, const char *source)
{
  int res=sclose(sockfd);
  if(logfile)
    fprintf(logfile, "FD %s:%d sclose(%d)\n",
            source, line, sockfd);
  return res;
}

FILE *memdebug_fopen(const char *file, const char *mode,
                 int line, const char *source)
{
  FILE *res=(fopen)(file, mode);
  if(logfile)
    fprintf(logfile, "FILE %s:%d fopen(\"%s\",\"%s\") = %p\n",
            source, line, file, mode, res);
  return res;
}

int memdebug_fclose(FILE *file, int line, const char *source)
{
  int res;

  assert(file != NULL);

  res=(fclose)(file);
  if(logfile)
    fprintf(logfile, "FILE %s:%d fclose(%p)\n",
            source, line, file);
  return res;
}
