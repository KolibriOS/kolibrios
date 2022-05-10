/*
 * Copyright (c) 2017 rxi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <sys/ksys.h>

#include "microtar.h"

typedef struct {
  char name[100];
  char mode[8];
  char owner[8];
  char group[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char type;
  char linkname[100];
  char _padding[255];
} mtar_raw_header_t;

static void * mtar_memset( void * s, int c, size_t n ){
  unsigned char * p = ( unsigned char * ) s;
  while ( n-- ){
      *p++ = ( unsigned char ) c;
  }
  return s;
}

#ifdef MTAR_OBJ
// All pointers was changed for compatible to latest version tcc and the libc.obj headers
size_t (*_fread)(void *restrict, size_t size, size_t count, FILE *restrict)=NULL;
size_t (*_fwrite)(const void *restrict, size_t size, size_t count, FILE *restrict)=NULL;
int    (*_fclose)(FILE *)=NULL;
FILE*  (*_fopen)(const char *restrict, const char *restrict)=NULL;
int    (*_fseek)(FILE *, long, int)=NULL;
long   (*_ftell)(FILE *)=NULL;
int    (*_sprintf)(char* buffer, const char* format, ...)=NULL;
int    (*_sscanf)(const char*, const char *restrict, ...)=NULL;
int    (*_strcmp)(const char * s1, const char* s2)=NULL;
char*  (*_strchr)(const char* s, int c)=NULL;
char*  (*_strcpy)(char*  s1, const char* s2)=NULL;

#endif

static unsigned round_up(unsigned n, unsigned incr) {
  return n + (incr - n % incr) % incr;
}


static unsigned checksum(const mtar_raw_header_t* rh) {
  unsigned i;
  unsigned char *p = (unsigned char*) rh;
  unsigned res = 256;
  for (i = 0; i < offsetof(mtar_raw_header_t, checksum); i++) {
    res += p[i];
  }
  for (i = offsetof(mtar_raw_header_t, type); i < sizeof(*rh); i++) {
    res += p[i];
  }
  return res;
}


static int tread(mtar_t *tar, void *data, unsigned size) {
  int err = tar->read(tar, data, size);
  tar->pos += size;
  return err;
}


static int twrite(mtar_t *tar, const void *data, unsigned size) {

  int err = tar->write(tar, data, size);
  tar->pos += size;
  return err;
}


static int write_null_bytes(mtar_t *tar, int n) {
  int i, err;
  char nul = '\0';
  for (i = 0; i < n; i++) {
    err = twrite(tar, &nul, 1);
    if (err) {
      return err;
    }
  }
  return MTAR_ESUCCESS;
}


static int raw_to_header(mtar_header_t *h, const mtar_raw_header_t *rh) {
  unsigned chksum1, chksum2;

  /* If the checksum starts with a null byte we assume the record is NULL */
  if (*rh->checksum == '\0') {
    return MTAR_ENULLRECORD;
  }

  /* Build and compare checksum */
  chksum1 = checksum(rh);
  _sscanf(rh->checksum, "%o", &chksum2);
  if (chksum1 != chksum2) {
    return MTAR_EBADCHKSUM;
  }

  /* Load raw header into header */
  _sscanf(rh->mode, "%o", &h->mode);
  _sscanf(rh->owner, "%o", &h->owner);
  _sscanf(rh->size, "%o", &h->size);
  _sscanf(rh->mtime, "%o", &h->mtime);
  h->type = rh->type;
  _strcpy(h->name, rh->name);
  _strcpy(h->linkname, rh->linkname);

  return MTAR_ESUCCESS;
}


static int header_to_raw(mtar_raw_header_t *rh, const mtar_header_t *h) {
  unsigned chksum;

  /* Load header into raw header */
  mtar_memset(rh, 0, sizeof(*rh));
  _sprintf(rh->mode, "%o", h->mode);
  _sprintf(rh->owner, "%o", h->owner);
  _sprintf(rh->size, "%o", h->size);
  _sprintf(rh->mtime, "%o", h->mtime);
  rh->type = h->type ? h->type : MTAR_TREG;
  _strcpy(rh->name, h->name);
  _strcpy(rh->linkname, h->linkname);

  /* Calculate and write checksum */
  chksum = checksum(rh);
  _sprintf(rh->checksum, "%06o", chksum);
  rh->checksum[7] = ' ';

  return MTAR_ESUCCESS;
}


const char* mtar_strerror(int err) {
  switch (err) {
    case MTAR_ESUCCESS     : return "success";
    case MTAR_EFAILURE     : return "failure";
    case MTAR_EOPENFAIL    : return "could not open";
    case MTAR_EREADFAIL    : return "could not read";
    case MTAR_EWRITEFAIL   : return "could not write";
    case MTAR_ESEEKFAIL    : return "could not seek";
    case MTAR_EBADCHKSUM   : return "bad checksum";
    case MTAR_ENULLRECORD  : return "null record";
    case MTAR_ENOTFOUND    : return "file not found";
  }
  return "unknown error";
}


static int file_write(mtar_t *tar, const void *data, unsigned size) {
  unsigned res = _fwrite(data, 1, size, tar->stream);
  return (res == size) ? MTAR_ESUCCESS : MTAR_EWRITEFAIL;
}

static int file_read(mtar_t *tar, void *data, unsigned size) {
  unsigned res = _fread(data, 1, size, tar->stream);
  return (res == size) ? MTAR_ESUCCESS : MTAR_EREADFAIL;
}

static int file_seek(mtar_t *tar, unsigned offset) {
    int res = _fseek(tar->stream, offset, SEEK_SET);
  return (res == 0) ? MTAR_ESUCCESS : MTAR_ESEEKFAIL;
}

static int file_close(mtar_t *tar) {
  _fclose(tar->stream);
  return MTAR_ESUCCESS;
}


int mtar_open(mtar_t *tar, const char *filename, const char *mode) {
  int err;
  mtar_header_t h;

  /* Init tar struct and functions */
  mtar_memset(tar, 0, sizeof(*tar));
  tar->write = file_write;
  tar->read = file_read;
  tar->seek = file_seek;
  tar->close = file_close;

  /* Assure mode is always binary */
  if ( _strchr(mode, 'r') ) mode = "rb";
  if ( _strchr(mode, 'w') ) mode = "wb";
  if ( _strchr(mode, 'a') ) mode = "ab";
  /* Open file */
  tar->stream = _fopen(filename, mode);
  if (!tar->stream) {
    return MTAR_EOPENFAIL;
  }
  /* Read first header to check it is valid if mode is `r` */
  if (*mode == 'r') {
    err = mtar_read_header(tar, &h);
    if (err != MTAR_ESUCCESS) {
      mtar_close(tar);
      return err;
    }
  }

  /* Return ok */
  return MTAR_ESUCCESS;
}


int mtar_close(mtar_t *tar) {
  return tar->close(tar);
}


int mtar_seek(mtar_t *tar, unsigned pos) {
  int err = tar->seek(tar, pos);
  tar->pos = pos;
  return err;
}


int mtar_rewind(mtar_t *tar) {
  tar->remaining_data = 0;
  tar->last_header = 0;
  return mtar_seek(tar, 0);
}


int mtar_next(mtar_t *tar) {
  int err, n;
  mtar_header_t h;
  /* Load header */
  err = mtar_read_header(tar, &h);
  if (err) {
    return err;
  }
  /* Seek to next record */
  n = round_up(h.size, 512) + sizeof(mtar_raw_header_t);
  return mtar_seek(tar, tar->pos + n);
}


int mtar_find(mtar_t *tar, const char *name, mtar_header_t *h) {
  int err;
  mtar_header_t header;
  /* Start at beginning */
  err = mtar_rewind(tar);
  if (err) {
    return err;
  }
  /* Iterate all files until we hit an error or find the file */
  while ( (err = mtar_read_header(tar, &header)) == MTAR_ESUCCESS ) {
    if ( !_strcmp(header.name, name) ) {
      if (h) {
        *h = header;
      }
      return MTAR_ESUCCESS;
    }
    mtar_next(tar);
  }
  /* Return error */
  if (err == MTAR_ENULLRECORD) {
    err = MTAR_ENOTFOUND;
  }
  return err;
}


int mtar_read_header(mtar_t *tar, mtar_header_t *h) {
  int err;
  mtar_raw_header_t rh;
  /* Save header position */
  tar->last_header = tar->pos;
  /* Read raw header */
  err = tread(tar, &rh, sizeof(rh));
  if (err) {
    return err;
  }
  /* Seek back to start of header */
  err = mtar_seek(tar, tar->last_header);
  if (err) {
    return err;
  }
  /* Load raw header into header struct and return */
  return raw_to_header(h, &rh);
}


int mtar_read_data(mtar_t *tar, void *ptr, unsigned size) {
  int err;
  /* If we have no remaining data then this is the first read, we get the size,
   * set the remaining data and seek to the beginning of the data */
  if (tar->remaining_data == 0) {
    mtar_header_t h;
    /* Read header */
    err = mtar_read_header(tar, &h);
    if (err) {
      return err;
    }
    /* Seek past header and init remaining data */
    err = mtar_seek(tar, tar->pos + sizeof(mtar_raw_header_t));
    if (err) {
      return err;
    }
    tar->remaining_data = h.size;
  }
  /* Read data */
  err = tread(tar, ptr, size);
  if (err) {
    return err;
  }
  tar->remaining_data -= size;
  /* If there is no remaining data we've finished reading and seek back to the
   * header */
  if (tar->remaining_data == 0) {
    return mtar_seek(tar, tar->last_header);
  }
  return MTAR_ESUCCESS;
}


int mtar_write_header(mtar_t *tar, const mtar_header_t *h) {
  mtar_raw_header_t rh;
  /* Build raw header and write */
  header_to_raw(&rh, h);
  tar->remaining_data = h->size;
  return twrite(tar, &rh, sizeof(rh));
}

int mtar_write_file_header(mtar_t *tar, const char *name, unsigned size) {
  mtar_header_t h;
  /* Build header */
  mtar_memset(&h, 0, sizeof(h));
  _strcpy(h.name, name);
  h.size = size;
  h.type = MTAR_TREG;
  h.mode = 0664;
  /* Write header */
  return mtar_write_header(tar, &h);
}


int mtar_write_dir_header(mtar_t *tar, const char *name) {
  mtar_header_t h;
  /* Build header */
  mtar_memset(&h, 0, sizeof(h));
  _strcpy(h.name, name);
  h.type = MTAR_TDIR;
  h.mode = 0775;
  /* Write header */
  return mtar_write_header(tar, &h);
}


int mtar_write_data(mtar_t *tar, const void *data, unsigned size) {
  int err;
  /* Write data */
  err = twrite(tar, data, size);
  if (err) {
    return err;
  }
  tar->remaining_data -= size;
  /* Write padding if we've written all the data for this file */
  if (tar->remaining_data == 0) {
    return write_null_bytes(tar, round_up(tar->pos, 512) - tar->pos);
  }
  return MTAR_ESUCCESS;
}


int mtar_finalize(mtar_t *tar) {
  /* Write two NULL records */
  return write_null_bytes(tar, sizeof(mtar_raw_header_t) * 2);
}

/* Load libc.obj */

#ifdef MTAR_OBJ

#include <sys/ksys.h>

int mtar_init(){
  ksys_dll_t *libc = _ksys_dlopen("/sys/lib/libc.obj");
  if(!libc){
    _ksys_debug_puts("mtar.obj: libc.obj not loaded!");
    return 1;
  }

  _fread  = _ksys_dlsym(libc, "fread");
  _fwrite = _ksys_dlsym(libc, "fwrite");
  _fclose = _ksys_dlsym(libc, "fclose");
  _fopen  = _ksys_dlsym(libc, "fopen");
  _fseek  = _ksys_dlsym(libc, "fseek");
  _ftell  = _ksys_dlsym(libc, "ftell");
  _sprintf= _ksys_dlsym(libc, "sprintf");
  _sscanf = _ksys_dlsym(libc, "sscanf");
  _strcmp = _ksys_dlsym(libc, "strcmp");
  _strchr = _ksys_dlsym(libc, "strchr");
  _strcpy = _ksys_dlsym(libc, "strcpy");
  return 0;
}


ksys_dll_t EXPORTS[] = {
  {"mtar_init", mtar_init},
  {"mtar_open", mtar_open},
  {"mtar_close", mtar_close},
  {"mtar_seek", mtar_seek},
  {"mtar_rewind", mtar_rewind},
  {"mtar_next", mtar_next},
  {"mtar_find", mtar_find},
  {"mtar_read_header", mtar_read_header},
  {"mtar_read_data", mtar_read_data},
  {"mtar_write_header", mtar_write_header},
  {"mtar_write_file_header", mtar_write_file_header},
  {"mtar_write_dir_header",  mtar_write_dir_header},
  {"mtar_write_data",mtar_write_data},
  {"mtar_finalize", mtar_finalize},
  {"mtar_strerror", mtar_strerror},
  NULL
};

#endif
