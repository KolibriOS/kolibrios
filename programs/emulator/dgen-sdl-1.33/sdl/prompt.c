#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prompt.h"

enum state_type {
	TYPE_SPACE, /* between words */
	TYPE_WORD, /* on a word */
	TYPE_SQUOTE, /* on a word, between simple quotes */
	TYPE_DQUOTE, /* on a word, between double quotes */
	TYPE_BSLASH, /* after a backslash */
	TYPE_BSLHEX1, /* \x, on the 1st hex digit */
	TYPE_BSLHEX2, /* \x, on the 2nd hex digit */
	TYPE_BSLOCT2, /* \[0-7], on the 2nd octal digit */
	TYPE_BSLOCT3, /* \[0-7], on the 3rd octal digit */
	TYPE_DQBSLASH, /* between ", after a backslash */
	TYPE_DQBSLHEX1, /* between ", \x, on the 1st hex digit */
	TYPE_DQBSLHEX2, /* between ", \x, on the 2nd hex digit */
	TYPE_DQBSLOCT2, /* between ", \[0-7], on the 2nd octal digit */
	TYPE_DQBSLOCT3, /* between ", \[0-7], on the 3rd octal digit */
	TYPE_MAX /* number of elements in this enum */
};

enum state_match {
	MATCH_SPACE, /* a space */
	MATCH_SQUOTE, /* a simple quote */
	MATCH_DQUOTE, /* a double quote */
	MATCH_BSLASH, /* a backslash */
	MATCH_HEX, /* a hex digit */
	MATCH_OCT, /* an octal digit */
	MATCH_x, /* an 'x' */
	MATCH_ANY, /* anything else */
	MATCH_MAX /* number of elements in this enum */
};

enum state_action {
	ACTION_NONE, /* ignore input */
	ACTION_CHAR, /* store character */
	ACTION_DIGIT /* store hex or octal digit */
};

struct state {
	enum state_type type; /* current state type */
	struct {
		enum state_action action; /* before going into next state */
		enum state_type type; /* next state */
	} next[MATCH_MAX];
};

static const struct state prompt_parse_state[TYPE_MAX] = {
	{
		TYPE_SPACE, /* between words */
		{
			{ ACTION_NONE, TYPE_SPACE }, /* MATCH_SPACE */
			{ ACTION_NONE, TYPE_SQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_DQUOTE }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_BSLASH }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_HEX */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_WORD } /* MATCH_ANY */
		}
	},
	{
		TYPE_WORD, /* on a word */
		{
			{ ACTION_NONE, TYPE_SPACE }, /* MATCH_SPACE */
			{ ACTION_NONE, TYPE_SQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_DQUOTE }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_BSLASH }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_HEX */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_WORD } /* MATCH_ANY */
		}
	},
	{
		TYPE_SQUOTE, /* on a word, between simple quotes */
		{
			{ ACTION_CHAR, TYPE_SQUOTE }, /* MATCH_SPACE */
			{ ACTION_NONE, TYPE_WORD }, /* MATCH_SQUOTE */
			{ ACTION_CHAR, TYPE_SQUOTE }, /* MATCH_DQUOTE */
			{ ACTION_CHAR, TYPE_SQUOTE }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_SQUOTE }, /* MATCH_HEX */
			{ ACTION_CHAR, TYPE_SQUOTE }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_SQUOTE }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_SQUOTE } /* MATCH_ANY */
		}
	},
	{
		TYPE_DQUOTE, /* on a word, between double quotes */
		{
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SPACE */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_WORD }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_DQBSLASH }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_HEX */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_DQUOTE } /* MATCH_ANY */
		}
	},
	{
		TYPE_BSLASH, /* after a backslash */
		{
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_SPACE */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_SQUOTE */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_DQUOTE */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_BSLOCT2 }, /* MATCH_OCT */
			{ ACTION_NONE, TYPE_BSLHEX1 }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_WORD } /* MATCH_ANY */
		}
	},
	{
		TYPE_BSLHEX1, /* \x, on the first hex digit */
		{
			{ ACTION_NONE, TYPE_SPACE }, /* MATCH_SPACE */
			{ ACTION_NONE, TYPE_SQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_DQUOTE }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_BSLASH }, /* MATCH_BSLASH */
			{ ACTION_DIGIT, TYPE_BSLHEX2 }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_BSLHEX2 }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_WORD } /* MATCH_ANY */
		}
	},
	{
		TYPE_BSLHEX2, /* \x, on the second hex digit */
		{
			{ ACTION_NONE, TYPE_SPACE }, /* MATCH_SPACE */
			{ ACTION_NONE, TYPE_SQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_DQUOTE }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_BSLASH }, /* MATCH_BSLASH */
			{ ACTION_DIGIT, TYPE_WORD }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_WORD }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_WORD } /* MATCH_ANY */
		}
	},
	{
		TYPE_BSLOCT2, /* \, on the second octal digit */
		{
			{ ACTION_NONE, TYPE_SPACE }, /* MATCH_SPACE */
			{ ACTION_NONE, TYPE_SQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_DQUOTE }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_BSLASH }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_BSLOCT3 }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_WORD } /* MATCH_ANY */
		}
	},
	{
		TYPE_BSLOCT3, /* \, on the third octal digit */
		{
			{ ACTION_NONE, TYPE_SPACE }, /* MATCH_SPACE */
			{ ACTION_NONE, TYPE_SQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_DQUOTE }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_BSLASH }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_WORD }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_WORD }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_WORD } /* MATCH_ANY */
		}
	},
	{
		TYPE_DQBSLASH, /* between ", after a backslash */
		{
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SPACE */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_DQUOTE */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_DQBSLOCT2 }, /* MATCH_OCT */
			{ ACTION_NONE, TYPE_DQBSLHEX1 }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_DQUOTE } /* MATCH_ANY */
		}
	},
	{
		TYPE_DQBSLHEX1, /* between ", \x, on the 1st hex digit */
		{
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SPACE */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_WORD }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_DQBSLASH }, /* MATCH_BSLASH */
			{ ACTION_DIGIT, TYPE_DQBSLHEX2 }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_DQBSLHEX2 }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_DQUOTE } /* MATCH_ANY */
		}
	},
	{
		TYPE_DQBSLHEX2, /* between ", \x, on the 2nd hex digit */
		{
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SPACE */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_WORD }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_DQBSLASH }, /* MATCH_BSLASH */
			{ ACTION_DIGIT, TYPE_DQUOTE }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_DQUOTE }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_DQUOTE } /* MATCH_ANY */
		}
	},
	{
		TYPE_DQBSLOCT2, /* between ", \[0-7], on the 2nd octal digit */
		{
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SPACE */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_WORD }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_DQBSLASH }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_BSLOCT3 }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_DQUOTE } /* MATCH_ANY */
		}
	},
	{
		TYPE_DQBSLOCT3, /* between ", \[0-7], on the 3rd octal digit */
		{
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SPACE */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_SQUOTE */
			{ ACTION_NONE, TYPE_WORD }, /* MATCH_DQUOTE */
			{ ACTION_NONE, TYPE_DQBSLASH }, /* MATCH_BSLASH */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_HEX */
			{ ACTION_DIGIT, TYPE_DQUOTE }, /* MATCH_OCT */
			{ ACTION_CHAR, TYPE_DQUOTE }, /* MATCH_x */
			{ ACTION_CHAR, TYPE_DQUOTE } /* MATCH_ANY */
		}
	}
};

void prompt_parse_clean(struct prompt_parse *pp)
{
	if (pp->argv != NULL) {
		unsigned int i;

		for (i = 0; (pp->argv[i] != NULL); ++i)
			free(pp->argv[i]);
		free(pp->argv);
	}
	free(pp->args);
	free(pp->argo);
	memset(pp, 0, sizeof(*pp));
}

struct prompt_parse *prompt_parse(struct prompt *p, struct prompt_parse *pp)
{
	struct prompt_history *ph = &p->history[p->current];
	enum state_type type;
	struct {
		unsigned int length;
		uint8_t tmp;
		unsigned int word: 1;
		unsigned int num: 1;
		unsigned int last: 1;
	} parsing;
	uint8_t *s;
	unsigned int i;
	unsigned int j;
	unsigned int k;
	unsigned int argc;
	unsigned int index;
	unsigned int cursor;
	enum state_action act;
	uint8_t **argv = NULL;
	unsigned int *args = NULL;
	struct prompt_parse_argo *argo = NULL;
	uint8_t *temp = NULL;
	size_t size_max = 0;
	uint8_t c = '\0';
	enum state_type cur = TYPE_SPACE;
	enum state_match ma = MATCH_SPACE;

	assert(ph->length <= sizeof(ph->line));
	assert(p->cursor <= ph->length);
argv_ok:
	argc = 0;
	index = 0;
	cursor = ~0u;
	memset(&parsing, 0, sizeof(parsing));
	type = TYPE_SPACE;
	for (s = ph->line, i = 0, j = 0, k = 0; (i != ph->length); ++i) {
		cur = type;
		c = s[i];
		if ((cur == TYPE_BSLASH) || (cur == TYPE_DQBSLASH)) {
			switch (c) {
			case 'a':
				c = '\a';
				break;
			case 'b':
				c = '\b';
				break;
			case 'f':
				c = '\f';
				break;
			case 'n':
				c = '\n';
				break;
			case 'r':
				c = '\r';
				break;
			case 't':
				c = '\t';
				break;
			case 'v':
				c = '\v';
				break;
			}
		}
		if (c == '\'')
			ma = MATCH_SQUOTE;
		else if (c == '"')
			ma = MATCH_DQUOTE;
		else if (c == '\\')
			ma = MATCH_BSLASH;
		else if (c == 'x')
			ma = MATCH_x;
		else if (isspace(c))
			ma = MATCH_SPACE;
		else if ((c >= '0') && (c <= '7') &&
			 ((cur == TYPE_BSLASH) ||
			  (cur == TYPE_BSLOCT2) ||
			  (cur == TYPE_BSLOCT3) ||
			  (cur == TYPE_DQBSLASH) ||
			  (cur == TYPE_DQBSLOCT2) ||
			  (cur == TYPE_DQBSLOCT3))) {
			ma = MATCH_OCT;
			parsing.tmp *= 8;
			parsing.tmp += (c - '0');
		}
		else if ((isxdigit(c)) &&
			 ((cur == TYPE_BSLHEX1) ||
			  (cur == TYPE_BSLHEX2) ||
			  (cur == TYPE_DQBSLHEX1) ||
			  (cur == TYPE_DQBSLHEX2))) {
			uint8_t x = c;

			ma = MATCH_HEX;
			parsing.tmp *= 16;
			x &= 0xcf;
			x -= (((c >> 6) & 0x01) * 7);
			x &= 0x0f;
			parsing.tmp += x;
		}
		else
			ma = MATCH_ANY;
		if (type == TYPE_SPACE) {
		last:
			if (i == p->cursor) {
				index = argc;
				if (type == TYPE_SPACE)
					cursor = ~0u;
				else
					cursor = parsing.length;
			}
			if (parsing.num) {
				if (temp != NULL)
					temp[parsing.length] = parsing.tmp;
				++(parsing.length);
				parsing.num = 0;
				parsing.tmp = 0;
			}
			if (parsing.word) {
				if (parsing.length > size_max)
					size_max = parsing.length;
				if (temp != NULL) {
					argv[argc] = malloc
						(sizeof(*argv[argc]) *
						 (parsing.length + 1));
					if (argv[argc] == NULL)
						goto fail;
					memcpy(argv[argc], temp,
					       parsing.length);
					argv[argc][parsing.length] = '\0';
					args[argc] = parsing.length;
					argo[argc].pos = k;
					if (type == TYPE_SPACE)
						argo[argc].len = ((i - 1) - k);
					else
						argo[argc].len = (i - k);
				}
				++argc;
				parsing.length = 0;
				parsing.word = 0;
				parsing.num = 0;
				parsing.tmp = 0;
			}
			if ((i == p->cursor) && (type == TYPE_SPACE))
				index = argc;
			if (parsing.last) {
				if (temp != NULL) {
					argo[argc].pos = p->cursor;
					argo[argc].len = 0;
				}
				goto end;
			}
		}
		act = prompt_parse_state[cur].next[ma].action;
		type = prompt_parse_state[cur].next[ma].type;
		if (i == p->cursor) {
			index = argc;
			if ((cur == TYPE_SPACE) && (type == TYPE_SPACE))
				cursor = ~0u;
			else
				cursor = parsing.length;
		}
		switch (act) {
		case ACTION_NONE:
			if (parsing.num) {
				if (temp != NULL)
					temp[parsing.length] = parsing.tmp;
				++(parsing.length);
				++j;
				parsing.num = 0;
				parsing.tmp = 0;
			}
			break;
		case ACTION_CHAR:
			if (parsing.num) {
				if (temp != NULL)
					temp[parsing.length] = parsing.tmp;
				++(parsing.length);
				++j;
				parsing.num = 0;
				parsing.tmp = 0;
			}
			if (temp != NULL)
				temp[parsing.length] = c;
			++(parsing.length);
			++j;
			break;
		case ACTION_DIGIT:
			parsing.num = 1;
			break;
		}
		if ((cur == TYPE_SPACE) && (type != TYPE_SPACE)) {
			k = i;
			parsing.word = 1;
		}
	}
	parsing.last = 1;
	goto last;
end:
	if (argv == NULL) {
		if (((argv = malloc(sizeof(*argv) * (argc + 1))) == NULL) ||
		    ((args = malloc(sizeof(*args) * (argc + 1))) == NULL) ||
		    ((argo = malloc(sizeof(*argo) * (argc + 1))) == NULL) ||
		    ((size_max != 0) &&
		     ((temp = malloc(sizeof(*temp) * size_max)) == NULL))) {
			argc = 0;
			goto fail;
		}
		argv[argc] = NULL;
		args[argc] = 0;
		argo[argc].pos = 0;
		argo[argc].len = 0;
		goto argv_ok;
	}
	free(temp);
	pp->index = index;
	pp->cursor = cursor;
	pp->argc = argc;
	pp->argv = argv;
	pp->args = args;
	pp->argo = argo;
	return pp;
fail:
	free(temp);
	if (argv != NULL) {
		for (i = 0; (i < argc); ++i)
			free(argv[i]);
		free(argv);
	}
	free(args);
	free(argo);
	return NULL;
}

void prompt_init(struct prompt *p)
{
	memset(p, 0, sizeof(*p));
	p->entries = 1;
}

void prompt_push(struct prompt *p)
{
	struct prompt_history *ph = &p->history[p->current];

	assert(ph->length <= sizeof(ph->line));
	assert(p->cursor <= ph->length);
	assert(p->entries <= (sizeof(p->history) / sizeof(p->history[0])));
	assert(p->current < p->entries);
	/* don't keep empty strings */
	if (ph->length == 0)
		return;
	if (p->entries != (sizeof(p->history) / sizeof(p->history[0])))
		++(p->entries);
	if (p->current != 0)
		p->history[0] = p->history[p->current];
	memmove(&p->history[1], &p->history[0],
		(sizeof(p->history[0]) * (p->entries - 1)));
	memset(&p->history[0], 0, sizeof(p->history[0]));
	p->cursor = 0;
	p->current = 0;
}

void prompt_older(struct prompt *p)
{
	assert(p->entries <= (sizeof(p->history) / sizeof(p->history[0])));
	assert(p->current < p->entries);
	if (p->current == (p->entries - 1))
		return;
	++(p->current);
	p->cursor = p->history[p->current].length;
	assert(p->cursor <= sizeof(p->history[p->current].line));
}

void prompt_newer(struct prompt *p)
{
	assert(p->entries <= (sizeof(p->history) / sizeof(p->history[0])));
	assert(p->current < p->entries);
	if (p->current == 0)
		return;
	--(p->current);
	p->cursor = p->history[p->current].length;
	assert(p->cursor <= sizeof(p->history[p->current].line));
}

void prompt_put(struct prompt *p, uint8_t c)
{
	struct prompt_history *ph = &p->history[p->current];

	assert(ph->length <= sizeof(ph->line));
	assert(p->cursor <= ph->length);
	if (p->cursor == ph->length) {
		/* append character */
		if (ph->length == sizeof(ph->line))
			return;
		ph->line[(ph->length++)] = c;
		++(p->cursor);
		return;
	}
	/* insert character */
	if (ph->length == sizeof(ph->line))
		--(ph->length);
	memmove(&ph->line[(p->cursor + 1)], &ph->line[p->cursor],
		(ph->length - p->cursor));
	++(ph->length);
	ph->line[(p->cursor++)] = c;
}

void prompt_delete(struct prompt *p)
{
	struct prompt_history *ph = &p->history[p->current];

	assert(ph->length <= sizeof(ph->line));
	assert(p->cursor <= ph->length);
	if (p->cursor == ph->length)
		return;
	/* delete character at cursor */
	memmove(&ph->line[p->cursor], &ph->line[(p->cursor + 1)],
		(ph->length - p->cursor));
	--(ph->length);
}

void prompt_replace(struct prompt *p, unsigned int pos,
		    unsigned int len, const uint8_t *with,
		    unsigned int with_len)
{
	struct prompt_history *ph = &p->history[p->current];
	uint8_t *dest;
	unsigned int dest_len;
	unsigned int dest_maxlen;

	assert(ph->length <= sizeof(ph->line));
	assert(p->cursor <= ph->length);
	if (pos > ph->length)
		return;
	dest = &ph->line[pos];
	dest_len = (ph->length - pos);
	dest_maxlen = (sizeof(ph->line) - pos);
	if (len > dest_len)
		len = dest_len;
	if (with_len > dest_maxlen)
		with_len = dest_maxlen;
	if (with_len > len) {
		unsigned int inc = (with_len - len);

		if ((dest_len + inc) > dest_maxlen)
			dest_len = (dest_maxlen - inc);
		ph->length += inc;
		memmove(&dest[with_len], &dest[len], dest_len);
	}
	else if (with_len < len) {
		unsigned int dec = (len - with_len);

		ph->length -= dec;
		memmove(&dest[with_len], &dest[len], dest_len);
	}
	memcpy(dest, with, with_len);
	if (p->cursor > ph->length)
		p->cursor = ph->length;
	assert(ph->length <= sizeof(ph->line));
	assert(p->cursor <= ph->length);
}

void prompt_left(struct prompt *p)
{
	/* move left */
	if (p->cursor != 0)
		--(p->cursor);
}

void prompt_right(struct prompt *p)
{
	struct prompt_history *ph = &p->history[p->current];

	assert(ph->length <= sizeof(ph->line));
	assert(p->cursor <= ph->length);
	/* move right */
	if (p->cursor != ph->length)
		++(p->cursor);
}

void prompt_begin(struct prompt *p)
{
	p->cursor = 0;
}

void prompt_end(struct prompt *p)
{
	struct prompt_history *ph = &p->history[p->current];

	assert(ph->length <= sizeof(ph->line));
	assert(p->cursor <= ph->length);
	p->cursor = ph->length;
}

void prompt_clear(struct prompt *p)
{
	struct prompt_history *ph = &p->history[p->current];

	assert(ph->length <= sizeof(ph->line));
	assert(p->cursor <= ph->length);
	ph->length = 0;
	p->cursor = 0;
}

void prompt_backspace(struct prompt *p)
{
	/* delete previous character */
	if (p->cursor != 0) {
		prompt_left(p);
		prompt_delete(p);
	}
}
