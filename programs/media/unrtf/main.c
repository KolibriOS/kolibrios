/*=============================================================================
   GNU UnRTF, a command-line program to convert RTF documents to other formats.
   Copyright (C) 2000,2001 Zachary Thayer Smith

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   The author is reachable by electronic mail at tuorfa@yahoo.com.
=============================================================================*/


/*----------------------------------------------------------------------
 * Module name:    main.c
 * Author name:    Zach Smith
 * Create date:    01 Sep 00
 * Purpose:        main() routine with file open/close.
 *----------------------------------------------------------------------
 * Changes:
 * 14 Oct 00, tuorfa@yahoo.com: added -nopict option
 * 15 Oct 00, tuorfa@yahoo.com: added verify_file_type() 
 * 08 Apr 01, tuorfa@yahoo.com: more GNU-like switches implemented
 * 24 Jul 01, tuorfa@yahoo.com: removed verify_file_type()
 * 03 Aug 01, tuorfa@yahoo.com: added --inline switch
 * 08 Sep 01, tuorfa@yahoo.com: added use of PROGRAM_NAME
 * 19 Sep 01, tuorfa@yahoo.com: addition of output personalities
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks 
 * 23 Sep 01, tuorfa@yahoo.com: added wpml switch
 *--------------------------------------------------------------------*/



#include <stdio.h>
#include <string.h>

#include "defs.h"
#include "error.h"
#include "word.h"
#include "convert.h"
#include "parse.h"
#include "hash.h"
#include "malloc.h"

#include "output.h"
#include "html.h"
#include "text.h"
#include "vt.h"
#include "ps.h"
#include "latex.h"
#include "wpml.h"



int nopict_mode; /* TRUE => do not write \pict's to files */
int dump_mode;   /* TRUE => output a dump of the word tree */
int debug_mode;  /* TRUE => output comments within HTML */
int lineno;      /* used for error reporting */
int simple_mode; /* TRUE => output HTML without SPAN/DIV tags */
int inline_mode; /* TRUE => output HTML without HTML/BODY/HEAD */


OutputPersonality *op;
enum {
	OP_HTML, OP_TEXT, OP_LATEX, OP_PS, OP_VT, OP_WPML
};



/*========================================================================
 * Name:	main
 * Purpose:	Main control function.
 * Args:	Args.
 * Returns:	Exit code.
 *=======================================================================*/

int
main (int argc, char **argv)
{
	FILE *f;
	Word * word;
	char *path=NULL;
	int i;
	int output_format = OP_HTML;

	nopict_mode = debug_mode = dump_mode = inline_mode = FALSE;

	if (argc<2 || argc>7) usage();

	for (i=1; i<argc; i++) {
		if (!strcmp("--dump",argv[i])) dump_mode=TRUE;
		else if (!strcmp("-d",argv[i])) dump_mode=TRUE;
		else if (!strcmp("--debug",argv[i])) debug_mode=TRUE;
		else if (!strcmp("--simple",argv[i])) simple_mode=TRUE;
		else if (!strcmp("--html",argv[i])) output_format=OP_HTML;
		else if (!strcmp("--text",argv[i])) output_format=OP_TEXT;
		else if (!strcmp("--vt",argv[i])) output_format=OP_VT;
		else if (!strcmp("--ps",argv[i])) output_format=OP_PS;
		else if (!strcmp("--latex",argv[i])) output_format=OP_LATEX;
		else if (!strcmp("--wpml",argv[i])) output_format=OP_WPML;
		else if (!strcmp("-t",argv[i])) {
			if ((i+1)<argc && *argv[i+1]!='-') {
				i++;
				if (!strcmp ("html", argv[i]))
					output_format=OP_HTML;
				else if (!strcmp ("vt", argv[i]))
					output_format=OP_VT;
				else if (!strcmp ("text", argv[i]))
					output_format=OP_TEXT;
				else if (!strcmp ("ps", argv[i]))
					output_format=OP_PS;
				else if (!strcmp ("latex", argv[i]))
					output_format=OP_LATEX;
				else if (!strcmp ("wpml", argv[i]))
					output_format=OP_WPML;
			}
		} 
		else if (!strcmp("--inline",argv[i])) inline_mode=TRUE;
		else if (!strcmp("--help",argv[i]))  {
			usage();
			exit (0);
		}
		else if (!strcmp("--version",argv[i]))  {
			fprintf (stderr, "%s\n", PROGRAM_VERSION);
			exit (0);
		}
		else if (!strcmp("--nopict",argv[i])) nopict_mode=TRUE;
		else if (!strcmp("-n",argv[i])) nopict_mode=TRUE;
		else {
			if (*argv[i]=='-') usage();

			if(path) 
				usage();
			else 	
				path=argv[i];
		}
	}
	
	if (!path) usage();

	switch (output_format) {
	case OP_TEXT:
		op = text_init();
		break;
	case OP_VT:
		op = vt_init();
		break;
	case OP_HTML:
		op = html_init();
		break;
	case OP_PS:
		op = ps_init();
		break;
	case OP_LATEX:
		op = latex_init();
		break;
	case OP_WPML:
		op = wpml_init();
		break;
	default:
		error_handler ("unknown output format");
	}

	hash_init();

	fprintf (stderr, "This is %s, ", PROGRAM_NAME);
	fprintf (stderr, "version %s\n", PROGRAM_VERSION);
	fprintf (stderr, "By Zach T. Smith\n");

	if (debug_mode) fprintf (stderr, "Debug mode.\n");
	if (dump_mode) fprintf (stderr, "Dump mode.\n");

	f = fopen (path, "r");
	if (!f) {
		char path2[200];
		strcpy(path2,path); strcat(path2,".rtf");
		f = fopen(path2, "r");
		if(!f)
			error_handler ("cannot open input file");
	}

	fprintf(stderr,"Processing %s...\n", path);

	lineno=0;

	word = word_read (f);

	if (dump_mode) {
		word_dump (word);
		printf ("\n");
	} else {
		word_print (word);
	}

	fclose(f);

	fprintf(stderr,"Done.\n");

	hash_stats();

	if (debug_mode) {
		fprintf (stderr, "Total memory allocated %ld bytes.\n",
			total_malloced());
	}

	/* May as well */
	word_free (word);

	return 0;
}


