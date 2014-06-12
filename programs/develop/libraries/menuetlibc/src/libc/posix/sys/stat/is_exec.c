/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/* IS_EXEC.C
 *
 * Given a filename or a file handle, and the extension of the file,
 * determine if the file is executable.
 * First, the file extension is checked in case it uniquely identifies
 * the file as either an executable or not.  Failing this, the first
 * two bytes of the file are tested for known signatures of executable
 * files.
 *
 * Copyright (c) 1994 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 */

#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libc/farptrgs.h>
#include <libc/dosio.h>

extern unsigned short _djstat_flags;
unsigned short        _get_magic(const char *, int);
int                   _is_executable(const char *, int, const char *);

/*
 * Read a MAGIC NUMBER from a given file.  These are the first
 * two bytes of the file, if we look at them as an unsigned short. */

#define _STAT_EXEC_EXT      2   /* get execute bits from file extension? */
#define _STAT_EXEC_MAGIC    4   /* get execute bits from magic signature? */

unsigned short _get_magic(const char *s, int fh)
{
}

/* A list of extensions which designate executable files.  These
   are NOT tested for the magic number.  */
static char executables[] = "|EXE|COM|BAT|BTM|DLL|VXD|";

/* A list of extensions which belong to files known to NEVER be
   executables.  These exist to minimize read()'ing files while
   detecting executables by magic number.  You are welcome to
   add to this list, but remember: only extensions which could
   NEVER be present in executables should go here.  */
static char non_executables[] = "\
|A|A01|A02|A03|A04|A05|ADL|ARC|ARJ|ASC|ASM|AUX|AWK\
|BAS|BIB|BGI|BMP\
|C|CC|CFG|CGZ|CH3|CHR|CI|CLP|CMF|CPI|CPP|CXX\
|DAT|DBF|DIZ|DOC|DVI\
|E|EL|ELC\
|F77|FN3\
|GIF|GZ\
|H|HLP|HPP|HXX\
|ICO|IN|INC|INF|INI\
|JPG\
|L|LEX|LF|LIB|LOG|LST|LZH\
|M|MAK|MAP|MF|MID|MPG\
|O|OBJ\
|PAK|PAS|PBM|PCD|PCX|PDS|PIC|PIF|PN3|PRJ|PS\
|RAS|RGB|RLE\
|S|SND|SY3\
|TAR|TAZ|TEX|TGA|TGZ|TIF|TXH|TXI|TXT\
|VOC\
|WAV|WK1|WK3|WKB|WQ1|WQ3|WQ4|WQ5|WQ6|WQ!\
|XBM\
|Y\
|ZIP|ZOO|";

int _is_executable(const char *filename, int fhandle, const char *extension)
{
  if (!extension && filename)
  {
    const char *cp, *ep=0;
    for (cp=filename; *cp; cp++)
    {
      if (*cp == '.')
	ep = cp;
      if (*cp == '/' || *cp == '\\' || *cp == ':')
	ep = 0;
    }
    extension = ep;
  }
  if ((_djstat_flags & _STAT_EXEC_EXT) == 0
      && extension
      && *extension
      && strlen(extension) <= ((extension[0]=='.') ? 4 : 3))
    {
      /* Search the list of extensions in executables[]. */
      char tmp_buf[6];

      tmp_buf[0] = '|';
      strcpy(tmp_buf+1, *extension == '.' ? extension + 1 : extension);
      strcat(tmp_buf, "|");
      if (strstr(non_executables, tmp_buf))
        return 0;
      else if (strstr(executables, tmp_buf))
        return 1;
    }

  /* No extension, or extension doesn't define execute
     bits unambiguously.  We are in for some dirty work.
     Read the first two bytes of the file and see if they
     are any of the known magic numbers which designate
     executable files.
     Unix-like shells, which have executable shell scripts
     without extensions and DON'T have "#!" as their FIRST
     TWO CHARACTERS, lose here.  Sorry, folks.  */
  if ( (_djstat_flags & _STAT_EXEC_MAGIC) == 0 )
    {
      switch (_get_magic(filename, fhandle))
        {
          case 0x5a4d:      /* "MZ" */
          case 0x010b:
          case 0x014c:
          case 0x2123:      /* "#!" */
              return 1;
        }
    }

  return 0;
}
