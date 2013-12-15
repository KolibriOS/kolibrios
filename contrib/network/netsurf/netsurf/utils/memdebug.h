/** \file
 * Heap debugging functions (interface).
 *
 * Based on memdebug.h from curl (see below), with the following modifications:
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
 * $Id: memdebug.h,v 1.1 2004/07/28 22:35:02 bursa Exp $
 ***************************************************************************/

#ifndef _MEMDEBUG_H_
#define _MEMDEBUG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define logfile memdebug_debuglogfile

extern FILE *logfile;

/* memory functions */
void *memdebug_malloc(size_t size, int line, const char *source);
void *memdebug_calloc(size_t elements, size_t size, int line, const char *source);
void *memdebug_realloc(void *ptr, size_t size, int line, const char *source);
void memdebug_free(void *ptr, int line, const char *source);
char *memdebug_strdup(const char *str, int line, const char *source);
char *memdebug_strndup(const char *str, size_t size, int line, const char *source);
void memdebug_memdebug(const char *logname);
void memdebug_memlimit(long limit);

/* file descriptor manipulators */
int memdebug_socket(int domain, int type, int protocol, int line , const char *);
int memdebug_sclose(int sockfd, int, const char *source);
int memdebug_accept(int s, void *addr, void *addrlen,
                int line, const char *source);

/* FILE functions */
FILE *memdebug_fopen(const char *file, const char *mode, int line,
                 const char *source);
int memdebug_fclose(FILE *file, int line, const char *source);

#ifndef MEMDEBUG_NODEFINES

#undef strdup
#define strdup(ptr) memdebug_strdup(ptr, __LINE__, __FILE__)
#define strndup(ptr,size) memdebug_strndup(ptr, size, __LINE__, __FILE__)
#define malloc(size) memdebug_malloc(size, __LINE__, __FILE__)
#define calloc(nbelem,size) memdebug_calloc(nbelem, size, __LINE__, __FILE__)
#define realloc(ptr,size) memdebug_realloc(ptr, size, __LINE__, __FILE__)
#define free(ptr) memdebug_free(ptr, __LINE__, __FILE__)

#define socket(domain,type,protocol)\
 memdebug_socket(domain,type,protocol,__LINE__,__FILE__)
#undef accept /* for those with accept as a macro */
#define accept(sock,addr,len)\
 memdebug_accept(sock,addr,len,__LINE__,__FILE__)

#define getaddrinfo(host,serv,hint,res) \
  memdebug_getaddrinfo(host,serv,hint,res,__LINE__,__FILE__)
#define getnameinfo(sa,salen,host,hostlen,serv,servlen,flags) \
  memdebug_getnameinfo(sa,salen,host,hostlen,serv,servlen,flags, __LINE__, \
  __FILE__)
#define freeaddrinfo(data) \
  memdebug_freeaddrinfo(data,__LINE__,__FILE__)

/* sclose is probably already defined, redefine it! */
#undef sclose
#define sclose(sockfd) memdebug_sclose(sockfd,__LINE__,__FILE__)
/* ares-adjusted define: */
#undef closesocket
#define closesocket(sockfd) memdebug_sclose(sockfd,__LINE__,__FILE__)

#undef fopen
#define fopen(file,mode) memdebug_fopen(file,mode,__LINE__,__FILE__)
#define fclose(file) memdebug_fclose(file,__LINE__,__FILE__)

#endif /* MEMDEBUG_NODEFINES */

#endif
