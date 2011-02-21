/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * This is file ST_LOSS.C
 *
 * Function to print a human-readable description of _DJSTAT_FAIL_BITS
 * variable, for debugging purposes.  A string which contains description
 * of every fail bit set is printed to the stream given by FP, or to
 * stderr if FP is NULL.
 *
 * Copyright (c) 1994 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely as long as the above copyright
 * notice is left intact.  There is no warranty on this software.
 *
 */

#include <stdio.h>
#include "xstat.h"

/* List of messages describing possible failures.  There is a message
   for every bit in the _DJSTAT_FAIL_BITS variable, plus a success
   message, which is printed if no bits are set.
*/
static const char *stfail_message[] = {
  "Everything checks out OK",
  "Get DOS SDA call (INT 21h/AX=5D06h/0Dh) failed",
  "Unsupported DOS version: less than 3.10 (for stat), or 1.x (for fstat)",
  "Cannot find SDA entry which corresponds to pathname (bad SDA pointer?)",
  "Get TrueName call (INT 21h/AX=6000h) failed",
  "Failed to get starting cluster number; inode defaults to hashing\n"
  "(if no other messages were printed, then this is either an empty\n"
  "file on a local disk drive, or a file on a networked drive, or\n"
  "you run under some kind of DOS clone)",
  "Root directory has no volume label required to get its time fields",
  "Get SDA call returned preposterously large or negative number of SDAs",
  "Write access bit required, but cannot be figured out",
  "Failed to get drive letter for handle (Novell NetWare 3.x?)",
  "SFT entry found, but is inconsistent with file size and time stamp",
  "Negative index into SFT: might be bad handle table in PSP",
  "File entry not found in SFT array (bad SFT index, or Novell 3.x)"
};

/* Number of used bits we know about in _djstat_fail_bits. */
static const int used_bits = sizeof(stfail_message)/sizeof(stfail_message[0]);

void
_djstat_describe_lossage(FILE *fp)
{
  int i              = 0;
  unsigned long bits = _djstat_fail_bits;  /* so we don't have side effects */

  if (fp == 0)
    fp = stderr;                    /* default: print to stderr */

  while (bits && i < used_bits)
    {
      i++;
      if (bits & 1)                 /* print message for this bit, if set */
        fprintf(fp, "%s\n", stfail_message[i]);

      bits >>= 1;                   /* get next bit */
    }

  /* Did we see any bit set?  */
  if (i == 0)
    fprintf(fp, "%s\n", stfail_message[0]);

}
