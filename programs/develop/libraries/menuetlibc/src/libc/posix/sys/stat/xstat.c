/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * This is file XSTAT.C
 *
 * Internal assist functions which are common to stat() and fstat().
 *
 *
 * Copyright (c) 1994-96 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely as long as the above copyright
 * notice is left intact.  There is no warranty on this software.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <time.h>
#include <errno.h>
#include <dos.h>
#include <libc/farptrgs.h>
#include <libc/dosio.h>
#include <libc/bss.h>

#include "xstat.h"

static int xstat_count = -1;

/* Some fields of struct stat are expensive to compute under DOS,
   because they require multiple disk accesses.  Fortunately, many
   DOS programs don't care about these.  To leave both pedants (like
   me) and performance-oriented guys happy, a variable is provided
   which controls which expensive fields should be computed.  To get
   the fastest stat() for your program, clear the bits for only those
   features you need and set the others.

   This improvement was suggested by Charles Sandmann
   <sandmann@clio.rice.edu> and DJ Delorie <dj@delorie.com>.  */

#define _STAT_INODE         1   /* should we bother getting inode numbers? */
#define _STAT_EXEC_EXT      2   /* get execute bits from file extension? */
#define _STAT_EXEC_MAGIC    4   /* get execute bits from magic signature? */
#define _STAT_DIRSIZE       8   /* compute directory size? */
#define _STAT_ROOT_TIME  0x10   /* try to get root dir time stamp? */
#define _STAT_WRITEBIT   0x20   /* fstat() needs write bit? */

/* Should we bother about executables at all? */
#define _STAT_EXECBIT       (_STAT_EXEC_EXT | _STAT_EXEC_MAGIC)

/* By default, all the bits are reset (including as yet unused ones), so
   people who don't care will transparently have the full version.  */
unsigned short _djstat_flags;

/* As we depend on undocumented DOS features, we could fail in some
   incompatible environment or future DOS versions.  If we do, the
   following variable will have some of its bits set.  Each bit
   describes a single feature which we tried to use and failed.
   The function _djstat_describe_lossage() may be called to print a
   human-readable description of the bits which were set by the last
   call to f?stat().  This should make debugging f?stat() failures
   in an unanticipated environment a lot easier.

   This improvement was suggested by Charles Sandmann
   <sandmann@clio.rice.edu>.  */

unsigned short _djstat_fail_bits;

/* ----------------------------------------------------------------------- */

/* Convert file date and time to time_t value suitable for
   struct stat fields.  */

time_t
_file_time_stamp(unsigned int dos_ftime)
{
  struct tm file_tm;

  memset(&file_tm, 0, sizeof(struct tm));
  file_tm.tm_isdst = -1;    /* let mktime() determine if DST is in effect */

  file_tm.tm_sec  = (dos_ftime & 0x1f) * 2;
  file_tm.tm_min  = (dos_ftime >>  5) & 0x3f;
  file_tm.tm_hour = (dos_ftime >> 11) & 0x1f;
  file_tm.tm_mday = (dos_ftime >> 16) & 0x1f;
  file_tm.tm_mon  = ((dos_ftime >> 21) & 0x0f) - 1; /* 0 = January */
  file_tm.tm_year = (dos_ftime >> 25) + 80;

  return mktime(&file_tm);
}

/* Get time stamp of a DOS file packed as a 32-bit int.
 * This does what Borland's getftime() does, except it doesn't
 * pollute the application namespace and returns an int instead
 * of struct ftime with packed bit-fields.
 */


int
_getftime(int fhandle, unsigned int *dos_ftime)
{
  return -1;
}

/* Invent an inode number for those files which don't have valid DOS
 * cluster number.  These could be: devices like /dev/nul; empty
 * files which were not allocated disk space yet; or files on
 * networked drives, for which the redirector doesn't bring the
 * cluster number.
 *
 * To ensure proper operation of this function, you must call it
 * with a filename in some canonical form.  E.g., with a name
 * returned by truename(), or that returned by _fixpath().  The
 * point here is that the entire program MUST abide by these
 * conventions through its operation, or else you risk getting
 * different inode numbers for the same file.
 *
 * This function is due to Eric Backus and was taken with minor
 * modifications from stat.c, as included in DJGPP 1.11m5.
 * The function now returns 0 instead of -1 if it can't allocate
 * memory for a new name, so that f?stat() won't fail if the inode
 * is unavailable, but return zero inode instead.
 */

/*
  (c) Copyright 1992 Eric Backus

  This software may be used freely so long as this copyright notice is
  left intact.  There is no warranty on this software.
*/

struct name_list
{
  struct name_list *next;
  char             *name;
  unsigned          mtime;
  unsigned long     size;
  long              inode;
};

ino_t
_invent_inode(const char *name, unsigned time_stamp, unsigned long fsize)
{
  static struct name_list  *name_list[256];

  /* If the st_inode is wider than a short int, we will count up
   * from USHRT_MAX+1 and thus ensure there will be no clashes with
   * actual cluster numbers.
   * Otherwise, we must count downward from USHRT_MAX, which could
   * yield two files with identical inode numbers: one from actual
   * DOS cluster number, and another from this function.  In the
   * latter case, we still have at least 80 inode numbers before
   * we step into potentially used numbers, because some FAT entries
   * are reserved to mean EOF, unused entry and other special codes,
   * and the FAT itself uses up some clusters which aren't counted.
   */
  static int         dir = (sizeof(ino_t) > 2 ? 1 : -1);

  /* INODE_COUNT is declared LONG and not ino_t, because some DOS-based
   * compilers use short or unsigned short for ino_t.
   */
  static long        inode_count = (sizeof(ino_t) > 2
                                    ? (long)USHRT_MAX + 1L
                                    : USHRT_MAX);

  struct name_list  *name_ptr, *prev_ptr;
  const char        *p;
  int                hash;

  /* Force initialization in restarted programs (emacs).  */
  if (xstat_count != __bss_count)
    {
      xstat_count = __bss_count;
      inode_count = (sizeof(ino_t) > 2 ? (long)USHRT_MAX + 1L : USHRT_MAX);
      memset (name_list, 0, sizeof name_list);
    }

  if (!name)
    return 0;

  /* Skip over device and leading slash.  This will allow for less
   * inode numbers to be used, because there is nothing bad in generating
   * identical inode for two files which belong to different drives.
   */
  if (*name && name[1] == ':' && (name[2] == '\\' || name[2] == '/'))
  {
    /* If this is a root directory, return inode = 1.  This is compatible
       with the code on stat.c which deals with root directories. */
    if (name[3] == 0)
      return (ino_t)1;

    name += 3;
  }

  /* If the passed name is empty, invent a new inode unconditionally.
   * This is for those unfortunate circumstances where we couldn't
   * get a name (e.g., fstat() under Novell).  For these we want at
   * least to ensure that no two calls will get the same inode number.
   * The lossage here is that you get different inodes even if you call
   * twice with the same file.  Sigh...
   */
  if (!*name)
    {
      ino_t retval = inode_count;

      inode_count += dir;
      return retval;
    }

  /* We could probably use a better hash than this */
  p = name;
  hash = 0;
  while (*p != '\0')
    hash += *p++;
  hash &= 0xff;

  /* Have we seen this string? */
  name_ptr = name_list[hash];
  prev_ptr = name_ptr;
  while (name_ptr)
    {
      if (strcmp(name, name_ptr->name) == 0 &&
          name_ptr->mtime == time_stamp &&
          name_ptr->size  == fsize)
        break;
      prev_ptr = name_ptr;
      name_ptr = name_ptr->next;
    }

  if (name_ptr)
    /* Same string, time stamp, and size, so same inode */
    return name_ptr->inode;
  else
    {
      ino_t retval;
      
      /* New string with same hash code */
      name_ptr = (struct name_list *)malloc(sizeof *name_ptr);
      if (name_ptr == 0)
        return 0;
      name_ptr->next = (struct name_list *)0;
      name_ptr->name = (char *)malloc(strlen(name)+1);
      if (name_ptr->name == 0)
      {
	free(name_ptr);
	return 0;
      }
      strcpy(name_ptr->name, name);
      name_ptr->mtime = time_stamp;
      name_ptr->size = fsize;
      name_ptr->inode = inode_count;
      if (prev_ptr)
        prev_ptr->next = name_ptr;
      else
        name_list[hash] = name_ptr;
      retval = inode_count;
      inode_count += dir; /* increment or decrement as appropriate */

      return retval;
    }
}
