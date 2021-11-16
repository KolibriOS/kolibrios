/*
 * Here are the routines of man2html that output a HREF string.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>		/* tolower() */
#include <string.h>		/* strlen() */
#include "defs.h"

/*
 * The default is to use cgibase. With relative html style
 * we generate URLs of the form "../manX/page.html".
 */
//static int relat_html_style = 0;

/*
 * The default is to use cgibase. With current html style
 * we generate URLs of the form "./page.html".
 */
static int current_html_style = 1;

/*
 * Either the user is non-local (or local, but using httpd),
 * in which case we use http:/cgi-bin, or the user is local
 * and uses lynx, and we use lynxcgi:/home/httpd/cgi-bin.
 */

#if 0

static char *man2htmlpath = "/cgi-bin/man/man2html"; 	/* default */
static char *cgibase_format = "http://%s"; 		/* host.domain:port */
static char *cgibase_ll_format = "lynxcgi:%s"; 		/* directory */
static char *cgibase = "http://localhost";		/* default */

#endif

/*
 * Separator between URL and argument string.
 *
 * With http:<path to script>/a/b?c+d+e the script is called
 * with PATH_INFO=/a/b and QUERY_STRING=c+d+e and args $1=c, $2=d, $3=e.
 * With lynxcgi:<full path to script>?c+d+e no PATH_INFO is possible.
 */
static char sep = '?';					/* or '/' */

/* What shall we say in case of relat_html_style? */
static char *signature = "<HR>\n"
"This document was created by\n"
"<A HREF=\"http://github.com/hamano/man2html/\">man2html</A>,\n"
"using the manual pages.<BR>\n"
"%s\n";

#define TIMEFORMAT "%T GMT, %B %d, %Y"
#define TIMEBUFSZ	500

void print_sig()
{
    char timebuf[TIMEBUFSZ];
    struct tm *timetm;
    time_t clock;

    timebuf[0] = 0;
#ifdef TIMEFORMAT
    sprintf(timebuf, "Time: ");
    clock=time(NULL);
    timetm=localtime(&clock);
    snprintf(timebuf, TIMEBUFSZ, "%s%s", timebuf, asctime(timetm));
    timebuf[TIMEBUFSZ-1] = 0;
#endif
    fprintf(out, signature, timebuf);
}

void
include_file_html(char *g) {
     fprintf(out, "<A HREF=\"file:/usr/include/%s\">%s</A>&gt;", g,g);
}

void
man_page_html(char *sec, char *h) {
	if (current_html_style) {
		if (!h)
			fprintf(out, "<A HREF=\"./\">"
			       "Return to Main Contents</A>");
		else
			fprintf(out, "<A HREF=\"./%s.html\">%s</A>",
			       h, h);
#if 0
	} else if (relat_html_style) {
		if (!h)
			fprintf(out, "<A HREF=\"../index.html\">"
			       "Return to Main Contents</A>");
		else
			fprintf(out, "<A HREF=\"../man%s/%s.%s.html\">%s</A>",
			       sec, h, sec, h);
	} else {
		if (!h)
			fprintf(out, "<A HREF=\"%s%s\">Return to Main Contents</A>",
			       cgibase, man2htmlpath);
		else if (!sec)
			fprintf(out, "<A HREF=\"%s%s%c%s\">%s</A>",
			       cgibase, man2htmlpath, sep, h, h);
		else
			fprintf(out, "<A HREF=\"%s%s%c%s+%s\">%s</A>",
			       cgibase, man2htmlpath, sep, sec, h, h);
#endif
	}
}


void
ftp_html(char *f) {
     fprintf(out, "<A HREF=\"ftp://%s\">%s</A>", f, f);
}

void
www_html(char *f) {
     fprintf(out, "<A HREF=\"http://%s\">%s</A>", f, f);
}

void
mailto_html(char *g) {
     fprintf(out, "<A HREF=\"mailto:%s\">%s</A>", g, g);
}

void
url_html(char *g) {
     fprintf(out, "<A HREF=\"%s\">%s</A>", g, g);
}
