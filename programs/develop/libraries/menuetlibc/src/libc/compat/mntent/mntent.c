/* Copyright (C) 1996 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
 * This is implementation of getmntent() and friends for DJGPP v2.x.
 *
 * Copyright (c) 1995-96 Eli Zaretskii <eliz@is.elta.co.il>
 *
 * This software may be used freely so long as this copyright notice is
 * left intact.  There is no warranty on this software.
 *
 * ---------------------------------------------------------------------
 *
 * The primary motivation for these functions was the GNU df program,
 * which lists all the mounted filesystems with a summary of the disk
 * space available on each one of them.  However, they are also useful
 * on their own right.
 *
 * Unlike Unix, where all mountable filesystems can be found on special
 * file (and thus implementing these function boils down to reading that
 * file), with MS-DOS it's a mess.  Every type of drive has its own
 * interface; there are JOINed and SUBSTed pseudo-drives and RAM disks;
 * different network redirectors hook DOS in a plethora of incompatible
 * ways; a single drive A: can be mapped to either A: or B:, etc.  That
 * is why this implementation uses almost every trick in the book to get
 * at the intimate details of every drive.  Some places where you might
 * find these tricks are: ``Undocumented DOS, 2nd ed.'' by Schulman et al
 * and Ralf Brown's Interrupt List.
 *
 */
#include <libc/stubs.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <mntent.h>
#include <dir.h>
#include <libc/farptrgs.h>
#include <sys/movedata.h>
#include <libc/unconst.h>
#include <assert.h>

/* Macro to convert a segment and an offset to a "far offset" suitable
   for _farxxx() functions of DJGPP.  */
#ifndef MK_FOFF
#define MK_FOFF(s,o) ((int)((((unsigned long)(s)) << 4) + (unsigned short)(o)))
#endif

#define CDS_JOIN     0x2000
#define CDS_VALID    0xc000
#define REMOVABLE    0
#define FIXED        1

/* Static variables.  */

static char          drive_number = -1;
static char          skip_drive_b = 0;
static char          drive_a_mapping = 0;
static char          cds_drives   = 0;
static unsigned long cds_address;
static int           cds_elsize;
static unsigned short dos_mem_base, our_mem_base;
static struct mntent mntent;
static unsigned char drive_string[128];
static char          *mnt_type;
static unsigned char mnt_dir[128];
static unsigned char mnt_fsname[128];
static          char dev_opts[] = "r ,dev=  ";

static char NAME_dblsp[] = "dblsp";
static char NAME_stac[] = "stac";
static char NAME_ram[] = "ram";
static char NAME_cdrom[] = "cdrom";
static char NAME_net[] = "net";
static char NAME_fd[] = "fd";
static char NAME_hd[] = "hd";
static char NAME_subst[] = "subst";
static char NAME_join[] = "join";

int _is_remote_drive(int);

/* Static helper functions.  */

/*
 * Get the entry for this disk in the DOS Current Directory Structure
 * (CDS).  In case of success, return this drive's attribute word; or
 * 0 in case of failure.  Fill the buffer at CURRDIR with the current
 * directory on that drive.
 * The pointer to the CDS array and the size of the array element
 * (which are DOS version-dependent) are computed when setmntent() is
 * called.
 */
static int
get_cds_entry(int drive_num, char *currdir)
{
  unsigned long  cds_entry_address;
  if (!cds_address)
    {
      *currdir = '\0';
      return 0;
    }

  /* The address of the CDS element for this drive.  */
  cds_entry_address = cds_address + (drive_num - 1)*cds_elsize;
  
  /* The current directory: 67-byte ASCIIZ string at the beginning
     of the CDS structure for our drive.  */
  movedata(dos_mem_base, (cds_entry_address & 0xfffff),
           our_mem_base, (unsigned int)currdir, 0x43);

  /* The drive attribute word is at the offset 43h, right after the
     current directory string.  */
  return _farpeekw(dos_mem_base, cds_entry_address + 0x43);
}

/*
 * For a PC with a single floppy drive, that drive can be referenced
 * as both A: and B:.  This function returns the logical drive number
 * which was last used to reference a physical drive, or 0 if the
 * drive has only one logical drive assigned to it (which means there
 * are two floppies in this system).
 */
static int assigned_to(int drive_num)
{
 return drive_num;
}

/*
 * Check if the drive is compressed with DoubleSpace.  If it is,
 * get the host drive on which the Compressed Volume File (CVF)
 * resides, put the name of that CVF into MNT_FSNAME[] and return
 * non-zero.
 */
static int get_doublespace_info(int drive_num)
{
 return 0;
}

static int get_stacker_info(int drive_num)
{
 return 0;
}

/*
 * Get the network name which corresponds to a drive DRIVE_NUM.
 * Ideally, _truename() (Int 21h/AH=60h) should return the same
 * string, but some network redirectors don't put a full UNC
 * name into the CDS, and others bypass the CDS altogether.
 */
static int get_netredir_entry(int drive_num)
{
 return 0;
}

/*
 * Return 1 if this drive is a CD-ROM drive, 0 otherwise.  Works
 * with MSCDEX 2.x, but what about other CD-ROM device drivers?
 */
static int is_cdrom_drive(int drive_num)
{
  return 0;
}

/*
 * Return 1 if a CD-ROM drive DRIVE_NUM is ready, i.e. there is a
 * disk in the drive and that disk is a data (not AUDIO) disk.
 */
static int cdrom_drive_ready(int drive_num)
{
  return 0;
}

/*
 * Detect a RAM disk.  We do this by checking if the number of FAT
 * copies (in the Device Parameter Block) is 1, which is typical of
 * RAM disks.  [This doesn't _have_ to be so, but if it's good
 * enough for Andrew Schulman et al (Undocumented DOS, 2nd edition),
 * we can use this as well.]
 */
static int is_ram_drive(int drive_num)
{
 return -1;
}

/*
 * Check if the media in this disk drive is fixed or removable.
 * Should only be called after we're sure this ain't CD-ROM or
 * RAM disk, since these might fool you with this call.
 */
static int media_type(int drive_num)
{
 return 0;
}

/* Exported library functions.  */

FILE * setmntent(char *filename, char *type)
{
 return NULL;
}

static char NAME_unknown[] = "???";
struct mntent * getmntent(FILE *filep)
{
 mntent.mnt_fsname = "FAT";
 mntent.mnt_dir    = "/";
 mntent.mnt_freq   = -1;
 mntent.mnt_passno = -1;
 mntent.mnt_time   = -1;
}

int addmntent(FILE *filep, struct mntent *mnt)
{
 unimpl();
}

char * hasmntopt(struct mntent *mnt, char *opt)
{
  return strstr(mnt->mnt_opts, opt);
}

int endmntent(FILE *filep)
{
  if (filep != (FILE *)1)
    {
      errno = EBADF;    /* fake errno for invalid handle */
      return NULL;
    }
  drive_number = 0;
  skip_drive_b = 0;
  return 1;
}
