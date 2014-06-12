/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
/*
   This is popen() and pclose() for MSDOS.  They were developed using
   Microsoft C, but should port easily to DOS C any compiler.
   
   Original author: pacetti@fl-ngnet.army.mil

   These routines are hacks, that is they SIMULATE thier UNIX
   counterparts.  Since MSDOS and won't allow concurrent process spawning,
   you can't really pipe.  I came up with this nearly stupid way around
   piping because I wanted to have some portability between UNIX and MSDOS.
   I'm probably not the first person to have this idea or implement it, but
   I think I'm the first to give it away for free (no points?!).

   The functions simulate popen() and pclose() by redirecting stdin or
   stdout, then spawning a child processes via system().

   If you popen() for read, the stdout is redirected to a temporary
   file, and the child is spawned.  The stdout is reopened via dup2(), the
   temporary file is opened for read and a file pointer to it is returned.

   If you popen() for write, a temporary file is opened for write, and
   a file pointer to it is returned.  When you pclose(), the stdin is
   redirected to the temporary file and the child is spawned.

   In both cases, pclose() closes and unlinks the temporary file.

   A static linked list of C structures is built to store necessary
   info for all popen()ed files so you can popen() more than one file at
   a time.

   The popen() function will return NULL on an error, or a valid FILE
   *pointer on a successful open.  The pclose() function will return
   negative one (-1) on an error or zero (0) on a successful close.

   The function prototypes are:

   FILE *popen(command, mode)
        char *command, char *mode;

   int pclose(pp)
       FILE *pp;

   Where command is a character string equivilant to a MSDOS command
   line, mode is "r" for read or "w" for write, and pp is a pointer to a
   file opened through popen().

   A main() function has been included for testing purposes, to compile
   it define the preprocessor token TEST at compile time.
 */

#include <libc/stubs.h>
#include <io.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libc/file.h>

/* hold file pointer, descriptor, command, mode, temporary file name */
struct pipe_list {
  FILE *fp;
  int fd;
  char *command, mode[10], temp_name[FILENAME_MAX];
  struct pipe_list *next;
};

/* static, global list pointer */
static struct pipe_list *pl = NULL;

FILE *
popen (const char *cm, const char *md) /* program name, pipe mode */
{
  struct pipe_list *l1, *l2;
  static char *tn = NULL;       /* temporary file basename */

  if (!tn)
    if ((tn = tmpnam(0)) == NULL)
      return NULL;

  /* make new node */
  if ((l1 = (struct pipe_list *) malloc (sizeof (struct pipe_list))) == NULL)
    return NULL;

  /* zero out elements to we'll get here */
  l1->fd = 0;
  l1->fp = NULL;
  l1->next = NULL;

  /* if empty list - just grab new node */
  if (!pl)
    pl = l1;
  else
  {
    /* otherwise, find last node in list */
    ++(l1->fd);
    l2 = pl;
    while (l2->next)
    {
      ++(l1->fd);
      l2 = l2->next;
    };
    /* add new node to list */
    l2->next = l1;
  }

  /* stick in elements we know already */
  strcpy (l1->mode, md);
  sprintf (l1->temp_name, "%s.%d", tn, l1->fd);

  /* if can save the program name, build temp file */
  if ((l1->command = malloc(strlen(cm)+1)))
  {
    strcpy(l1->command, cm);
    /* if caller wants to read */
    if (l1->mode[0] == 'r')
    {
      /* dup stdout */
      if ((l1->fd = dup (fileno (stdout))) == EOF)
	l1->fp = NULL;
      else if (!(l1->fp = freopen (l1->temp_name, "wb", stdout)))
	l1->fp = NULL;
      else
	/* exec cmd */
	if (system (cm) == EOF)
	  l1->fp = NULL;
      /* reopen real stdout */
      if (dup2 (l1->fd, fileno (stdout)) == EOF)
      {
	l1->fp = NULL;
	close(l1->fd);
      }
      else
	/* open file for reader */
	l1->fp = fopen (l1->temp_name, l1->mode);
    }
    else
      /* if caller wants to write */
      if (l1->mode[0] == 'w')
        /* open temp file */
        l1->fp = fopen (l1->temp_name, l1->mode);
      else
        /* unknown mode */
        l1->fp = NULL;
  }
  return l1->fp;              /* return == NULL ? ERROR : OK */
}

int
pclose (FILE *pp)
{
  struct pipe_list *l1, *l2;    /* list pointers */
  int retval=0;			/* function return value */

  /* if pointer is first node */
  if (pl->fp == pp)
  {
    /* save node and take it out the list */
    l1 = pl;
    pl = l1->next;
  }
  else
    /* if more than one node in list */
    if (pl->next)
    {
      /* find right node */
      for (l2 = pl, l1 = pl->next; l1; l2 = l1, l1 = l2->next)
        if (l1->fp == pp)
          break;

      /* take node out of list */
      l2->next = l1->next;
    }
    else
      return -1;

  /* if FILE not in list - return error */
  if (l1->fp == pp)
  {
    /* close the (hopefully) popen()ed file */
    fclose (l1->fp);

    /* if pipe was opened to write */
    if (l1->mode[0] == 'w')
    {
      /* dup stdin */
      if ((l1->fd = dup (fileno (stdin))) == EOF)
	retval = -1;
      else
	/* open temp stdin */
	if (!(l1->fp = freopen (l1->temp_name, "rb", stdin)))
	  retval = -1;
	else
	  /* exec cmd */
          if (system (l1->command) == EOF)
            retval = -1;
          else
	  {
            /* reopen stdin */
	    if (dup2 (l1->fd, fileno (stdin)) == EOF)
	      retval = -1;
	    close(l1->fd);
	  }
    }
    else
      /* if pipe was opened to read */
      if (l1->mode[0] == 'r')
        retval = 0;
      else
        /* invalid mode */
        retval = -1;
  }
  remove (l1->temp_name);       /* remove temporary file */
  free (l1->command);           /* dealloc memory */
  free (l1);                    /* dealloc memory */
  l1 = NULL;                    /* make pointer bogus */

  return retval;              /* retval==0 ? OK : ERROR */
}
