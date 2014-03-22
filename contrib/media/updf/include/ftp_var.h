/*
 * Copyright (c) 1985, 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ftp_var.h	8.4 (Berkeley) 10/9/94
 */

/*
 * FTP global variables.
 */

#include <sys/param.h>
#include <setjmp.h>

#include "extern.h"

#ifndef FTP_EXTERN
#define FTP_EXTERN extern
#endif
/*
 * Options and other state info.
 */
FTP_EXTERN int	trace;		/* trace packets exchanged */
FTP_EXTERN int	hash;		/* print # for each buffer transferred */
FTP_EXTERN int	hashbytes;	/* number of bytes per # printed */
FTP_EXTERN int	sendport;	/* use PORT cmd for each data connection */
FTP_EXTERN int	verbose;	/* print messages coming back from server */
FTP_EXTERN int	connected;	/* connected to server */
FTP_EXTERN int	fromatty;	/* input is from a terminal */
FTP_EXTERN int	interactive;	/* interactively prompt on m* cmds */
FTP_EXTERN int	debug;		/* debugging level */
FTP_EXTERN int	bell;		/* ring bell on cmd completion */
FTP_EXTERN int	doglob;		/* glob local file names */
FTP_EXTERN int	autologin;	/* establish user account on connection */
FTP_EXTERN int	proxy;		/* proxy server connection active */
FTP_EXTERN int	proxflag;	/* proxy connection exists */
FTP_EXTERN int	sunique;	/* store files on server with unique name */
FTP_EXTERN int	runique;	/* store local files with unique name */
FTP_EXTERN int	mcase;		/* map upper to lower case for mget names */
FTP_EXTERN int	ntflag;		/* use ntin ntout tables for name translation */
FTP_EXTERN int	mapflag;	/* use mapin mapout templates on file names */
FTP_EXTERN int	code;		/* return/reply code for ftp command */
FTP_EXTERN int	crflag;		/* if 1, strip car. rets. on ascii gets */
FTP_EXTERN char	pasv[64];	/* passive port for proxy data connection */
FTP_EXTERN int	passivemode;	/* passive mode enabled */
FTP_EXTERN char	*altarg;	/* argv[1] with no shell-like preprocessing  */
FTP_EXTERN char	ntin[17];	/* input translation table */
FTP_EXTERN char	ntout[17];	/* output translation table */
extern	char *mapin;		/* input map template */
extern	char *mapout;		/* output map template */
FTP_EXTERN char	typename[32];	/* name of file transfer type */
FTP_EXTERN int	type;		/* requested file transfer type */
FTP_EXTERN int	curtype;	/* current file transfer type */
FTP_EXTERN char	structname[32];	/* name of file transfer structure */
FTP_EXTERN int	stru;		/* file transfer structure */
FTP_EXTERN char	formname[32];	/* name of file transfer format */
FTP_EXTERN int	form;		/* file transfer format */
FTP_EXTERN char	modename[32];	/* name of file transfer mode */
FTP_EXTERN int	mode;		/* file transfer mode */
FTP_EXTERN char	bytename[32];	/* local byte size in ascii */
FTP_EXTERN int	bytesize;	/* local byte size in binary */

FTP_EXTERN char	*hostname;	/* name of host connected to */
FTP_EXTERN int	unix_server;	/* server is unix, can use binary for ascii */
FTP_EXTERN int	unix_proxy;	/* proxy is unix, can use binary for ascii */

FTP_EXTERN struct servent *sp;	/* service spec for tcp/ftp */

FTP_EXTERN jmp_buf toplevel;	/* non-local goto stuff for cmd scanner */

#if HAVE_LIBREADLINE
FTP_EXTERN char *line;
#else
FTP_EXTERN char	line[200];	/* input line buffer */
#endif

FTP_EXTERN char	*stringbase;	/* current scan point in line buffer */
FTP_EXTERN char	argbuf[200];	/* argument storage buffer */
FTP_EXTERN char	*argbase;	/* current storage point in arg buffer */
FTP_EXTERN int	margc;		/* count of arguments on input line */
FTP_EXTERN char	*margv[20];	/* args parsed from input line */
FTP_EXTERN int	cpend;		/* flag: if != 0, then pending server reply */
FTP_EXTERN int	mflag;		/* flag: if != 0, then active multi command */

FTP_EXTERN int	options;	/* used during socket creation */

/*
 * Format of command table.
 */
struct cmd {
	char	*c_name;	/* name of command */
	char	*c_help;	/* help string */
	char	c_bell;		/* give bell when command completes */
	char	c_conn;		/* must be connected to use command */
	char	c_proxy;	/* proxy server may execute */
	void	(*c_handler) __P((int, char **)); /* function to call */
};

struct macel {
	char mac_name[9];	/* macro name */
	char *mac_start;	/* start of macro in macbuf */
	char *mac_end;		/* end of macro in macbuf */
};

FTP_EXTERN int macnum;			/* number of defined macros */
FTP_EXTERN struct macel macros[16];
FTP_EXTERN char macbuf[4096];
