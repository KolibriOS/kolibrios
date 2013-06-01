/*
 * Copyright 2003 Phil Mellor <monkeyson@users.sourceforge.net>
 * Copyright 2003 John M Bell <jmb202@ecs.soton.ac.uk>
 * Copyright 2004 James Bursa <bursa@users.sourceforge.net>
 * Copyright 2005 Richard Wilson <info@tinct.net>
 *
 * This file is part of NetSurf, http://www.netsurf-browser.org/
 *
 * NetSurf is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * NetSurf is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 * Option reading and saving (implementation).
 *
 * Options are stored in the format key:value, one per line. For bool options,
 * value is "0" or "1".
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "css/css.h"
#include "desktop/plot_style.h"
#include "utils/log.h"
#include "utils/utils.h"
#include "desktop/options.h"

struct ns_options nsoptions = {
	NSOPTION_MAIN_DEFAULTS,
	NSOPTION_SYS_COLOUR_DEFAULTS,
	NSOPTION_EXTRA_DEFAULTS
};

enum option_type_e {
	OPTION_BOOL,
	OPTION_INTEGER,
	OPTION_STRING,
	OPTION_COLOUR
} ;

struct option_entry_s {
	const char *key;
	enum option_type_e type;
	void *p;
};

struct option_entry_s option_table[] = {
	NSOPTION_MAIN_TABLE,
	NSOPTION_EXTRA_TABLE
};

#define option_table_entries (sizeof option_table / sizeof option_table[0])

/**
 * Set an option value based on a string
 */
static bool 
strtooption(const char *value, struct option_entry_s *option_entry)
{
	bool ret = false;
	colour rgbcolour; /* RRGGBB */

	switch (option_entry->type) {
	case OPTION_BOOL:
		*((bool *)option_entry->p) = value[0] == '1';
		ret = true;
		break;

	case OPTION_INTEGER:
		*((int *)option_entry->p) = atoi(value);
		ret = true;
		break;

	case OPTION_COLOUR:
		sscanf(value, "%x", &rgbcolour);
		*((colour *)option_entry->p) =
			((0x000000FF & rgbcolour) << 16) |
			((0x0000FF00 & rgbcolour) << 0) |
			((0x00FF0000 & rgbcolour) >> 16);
		ret = true;
		break;

	case OPTION_STRING:
		free(*((char **)option_entry->p));
		if (*value == 0) {
			/* do not allow empty strings in text options */
			*((char **)option_entry->p) = NULL;
		} else {
			*((char **)option_entry->p) = strdup(value);
		}
		ret = true;
		break;
	}

	return ret;
}

static void nsoptions_validate(struct ns_options *opts)
{
	if (opts->font_size < 50)
		opts->font_size = 50;

	if (1000 < opts->font_size)
		opts->font_size = 1000;

	if (opts->font_min_size < 10)
		opts->font_min_size = 10;

	if (500 < opts->font_min_size)
		opts->font_min_size = 500;

	if (opts->memory_cache_size < 0)
		opts->memory_cache_size = 0;

}

/* exported interface documented in options.h */
void nsoption_read(const char *path)
{
	char s[100];
	FILE *fp;

	if (path == NULL) {
		LOG(("No options loaded"));
		return;
	}

	fp = fopen(path, "r");
	if (!fp) {
		LOG(("failed to open file '%s'", path));
		return;
	}

	while (fgets(s, 100, fp)) {
		char *colon, *value;
		unsigned int i;

		if (s[0] == 0 || s[0] == '#')
			continue;
		colon = strchr(s, ':');
		if (colon == 0)
			continue;
		s[strlen(s) - 1] = 0;  /* remove \n at end */
		*colon = 0;  /* terminate key */
		value = colon + 1;

		for (i = 0; i != option_table_entries; i++) {
			if (strcasecmp(s, option_table[i].key) != 0)
				continue;

			strtooption(value, option_table + i);
			break;
		}
	}

	fclose(fp);

	nsoptions_validate(&nsoptions);
}


/* exported interface documented in options.h */
void nsoption_write(const char *path)
{
	unsigned int entry;
	FILE *fp;
	colour rgbcolour; /* RRGGBB */

	fp = fopen(path, "w");
	if (!fp) {
		LOG(("failed to open file '%s' for writing", path));
		return;
	}

	for (entry = 0; entry != option_table_entries; entry++) {
		switch (option_table[entry].type) {
		case OPTION_BOOL:
			fprintf(fp, "%s:%c\n", option_table[entry].key, 
				*((bool *) option_table[entry].p) ? '1' : '0');
			break;

		case OPTION_INTEGER:
			fprintf(fp, "%s:%i\n", option_table[entry].key, 
				*((int *) option_table[entry].p));
			break;

		case OPTION_COLOUR:
			rgbcolour = ((0x000000FF & *((colour *)
					option_table[entry].p)) << 16) |
				((0x0000FF00 & *((colour *)
					option_table[entry].p)) << 0) |
				((0x00FF0000 & *((colour *)
					option_table[entry].p)) >> 16);
			fprintf(fp, "%s:%06x\n", option_table[entry].key, 
				rgbcolour);
			break;

		case OPTION_STRING:
			if (((*((char **) option_table[entry].p)) != NULL) && 
			    (*(*((char **) option_table[entry].p)) != 0)) {
				fprintf(fp, "%s:%s\n", option_table[entry].key,
					*((char **) option_table[entry].p));
			}
			break;
		}
	}

	fclose(fp);
}


/**
 * Output an option value into a string, in HTML format.
 *
 * \param option  The option to output the value of.
 * \param size    The size of the string buffer.
 * \param pos     The current position in string
 * \param string  The string in which to output the value.
 * \return The number of bytes written to string or -1 on error
 */
static size_t 
nsoption_output_value_html(struct option_entry_s *option,
		size_t size, size_t pos, char *string)
{
	size_t slen = 0; /* length added to string */
	colour rgbcolour; /* RRGGBB */

	switch (option->type) {
	case OPTION_BOOL:
		slen = snprintf(string + pos, size - pos, "%s",
				*((bool *)option->p) ? "true" : "false");
		break;

	case OPTION_INTEGER:
		slen = snprintf(string + pos, size - pos, "%i",
				*((int *)option->p));
		break;

	case OPTION_COLOUR:
		rgbcolour = ((0x000000FF & *((colour *) option->p)) << 16) |
				((0x0000FF00 & *((colour *) option->p)) << 0) |
				((0x00FF0000 & *((colour *) option->p)) >> 16);
		slen = snprintf(string + pos, size - pos,
				"<span style=\"background-color: #%06x; "
				"color: #%06x;\">#%06x</span>", rgbcolour,
				(~rgbcolour) & 0xffffff, rgbcolour);
		break;

	case OPTION_STRING:
		if (*((char **)option->p) != NULL) {
			slen = snprintf(string + pos, size - pos, "%s",
					*((char **)option->p));
		} else {
			slen = snprintf(string + pos, size - pos,
					"<span class=\"null-content\">NULL"
					"</span>");
		}
		break;
	}

	return slen;
}


/**
 * Output an option value into a string, in plain text format.
 *
 * \param option  The option to output the value of.
 * \param size    The size of the string buffer.
 * \param pos     The current position in string
 * \param string  The string in which to output the value.
 * \return The number of bytes written to string or -1 on error
 */
static size_t 
nsoption_output_value_text(struct option_entry_s *option,
		size_t size, size_t pos, char *string)
{
	size_t slen = 0; /* length added to string */
	colour rgbcolour; /* RRGGBB */

	switch (option->type) {
	case OPTION_BOOL:
		slen = snprintf(string + pos, size - pos, "%c",
				*((bool *)option->p) ? '1' : '0');
		break;

	case OPTION_INTEGER:
		slen = snprintf(string + pos, size - pos, "%i",
				*((int *)option->p));
		break;

	case OPTION_COLOUR:
		rgbcolour = ((0x000000FF & *((colour *) option->p)) << 16) |
				((0x0000FF00 & *((colour *) option->p)) << 0) |
				((0x00FF0000 & *((colour *) option->p)) >> 16);
		slen = snprintf(string + pos, size - pos, "%06x", rgbcolour);
		break;

	case OPTION_STRING:
		if (*((char **)option->p) != NULL) {
			slen = snprintf(string + pos, size - pos, "%s",
					*((char **)option->p));
		}
		break;
	}

	return slen;
}

/* exported interface documented in options.h */
void 
nsoption_commandline(int *pargc, char **argv)
{
	char *arg;
	char *val;
	int arglen;
	int idx = 1;
	int mv_loop;

	unsigned int entry_loop;

	while (idx < *pargc) {
		arg = argv[idx];
		arglen = strlen(arg);

		/* check we have an option */
		/* option must start -- and be as long as the shortest option*/
		if ((arglen < (2+5) ) || (arg[0] != '-') || (arg[1] != '-'))
			break;

		arg += 2; /* skip -- */

		val = strchr(arg, '=');
		if (val == NULL) {
			/* no equals sign - next parameter is val */
			idx++;
			if (idx >= *pargc)
				break;
			val = argv[idx];
		} else {
			/* equals sign */
			arglen = val - arg ;
			val++;
		}

		/* arg+arglen is the option to set, val is the value */

		LOG(("%.*s = %s",arglen,arg,val));

		for (entry_loop = 0; 
		     entry_loop < option_table_entries; 
		     entry_loop++) {
			if (strncmp(arg, option_table[entry_loop].key, 
				    arglen) == 0) { 
				strtooption(val, option_table + entry_loop);
				break;
			}			
		}

		idx++;
	}

	/* remove processed options from argv */
	for (mv_loop=0;mv_loop < (*pargc - idx); mv_loop++) {
		argv[mv_loop + 1] = argv[mv_loop + idx];
	}
	*pargc -= (idx - 1);
}

/* exported interface documented in options.h */
int 
nsoption_snoptionf(char *string, size_t size, unsigned int option, const char *fmt)
{
	size_t slen = 0; /* current output string length */
	int fmtc = 0; /* current index into format string */
	struct option_entry_s *option_entry;

	if (option >= option_table_entries)
		return -1;

	option_entry = option_table + option;

	while((slen < size) && (fmt[fmtc] != 0)) {
		if (fmt[fmtc] == '%') {
			fmtc++;
			switch (fmt[fmtc]) {
			case 'k':
				slen += snprintf(string + slen, size - slen,
						"%s", option_entry->key);
				break;

			case 't':
				switch (option_entry->type) {
				case OPTION_BOOL:
					slen += snprintf(string + slen,
							 size - slen,
							 "boolean");
					break;

				case OPTION_INTEGER:
					slen += snprintf(string + slen,
							 size - slen,
							 "integer");
					break;

				case OPTION_COLOUR:
					slen += snprintf(string + slen,
							 size - slen,
							 "colour");
					break;

				case OPTION_STRING:
					slen += snprintf(string + slen,
							 size - slen,
							 "string");
					break;

				}
				break;


			case 'V':
				slen += nsoption_output_value_html(option_entry,
						size, slen, string);
				break;
			case 'v':
				slen += nsoption_output_value_text(option_entry,
						size, slen, string);
				break;
			}
			fmtc++;
		} else {
			string[slen] = fmt[fmtc];
			slen++;
			fmtc++;
		}
	}

	/* Ensure that we NUL-terminate the output */
	string[min(slen, size - 1)] = '\0';

	return slen;
}

/* exported interface documented in options.h */
void 
nsoption_dump(FILE *outf)
{
	char buffer[256];
	int opt_loop = 0;
	int res;

	do {
		res = nsoption_snoptionf(buffer, sizeof buffer, opt_loop,
				"%k:%v\n");
		if (res > 0) {
			fprintf(outf, "%s", buffer);
		}
		opt_loop++;
	} while (res > 0);
}

