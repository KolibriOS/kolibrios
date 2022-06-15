/**
 * @file
 * The following is a full featured parser for configuration files using
 * basic format "key = value".
 *
 * Well, it's big, but it can properly manage spaces, empty lines,
 * single and double-quoted strings, hex numbers, comments, semicolons
 * and more. It also happens to be much more robust than the original one.
 *
 * @author zamaz
 */

#include <stddef.h>
#include <assert.h>
#include "ckvp.h"

enum {
	STATE_ERROR = 1,
	STATE_BEGIN,     /**< initial state */
	STATE_COMMENT,   /**< currently in a comment */
	STATE_KEY,       /**< (key) currently in a key */
	STATE_KEYBS,     /**< (key) backslash */
	STATE_KEYBSX1,   /**< (key) first character of a hex value (\\x) */
	STATE_KEYBSX2,   /**< (key) second character of a hex value (\\x) */
	STATE_KEYSQ,     /**< (key) currently in a simple quoted key */
	STATE_KEYDQ,     /**< (key) currently in a double quoted key */
	STATE_KEYDQBS,   /**< (key) backslash while in double quotes */
	STATE_KEYDQBSX1, /**< (key) first value of \\x in double quotes */
	STATE_KEYDQBSX2, /**< (key) second value of \\x in double quotes */
	STATE_BEQ,       /**< before '=' between key and value */
	STATE_AEQ,       /**< after '=' between key and value */
	STATE_VALUE,     /**< (value) same as (key) things above, for values */
	STATE_VALBS,     /**< (value) backslash */
	STATE_VALBSX1,   /**< (value) first character of an hex value (\\x) */
	STATE_VALBSX2,   /**< (value) second character of a hex value (\\x) */
	STATE_VALSQ,     /**< (value) currently in a simple quoted value */
	STATE_VALDQ,     /**< (value) currently in a double quoted value */
	STATE_VALDQBS,   /**< (value) backslash while in double quotes */
	STATE_VALDQBSX1, /**< (value) first value of \\x in double quotes */
	STATE_VALDQBSX2, /**< (value) second values of \\x in double quotes */
	STATE_VALEND,    /**< end of a value, ready to take a new key */
	ACTION_KEY        = 0x0100, /**< key complete */
	ACTION_VALUE      = 0x0200, /**< value complete */
	ACTION_ERROR      = 0x0400, /**< caught an error */
	ACTION_STORE      = 0x1000, /**< character must be stored as is */
	ACTION_STORE_MOD  = 0x2000, /**< store filtered character */
	ACTION_STORE_HEX1 = 0x4000, /**< store first hex digit */
	ACTION_STORE_HEX2 = 0x8000  /**< store second hex digit */
};

#define	HEX_INDICES(st)						\
	['0'] = (st), ['1'] = (st), ['2'] = (st), ['3'] = (st),	\
	['4'] = (st), ['5'] = (st), ['6'] = (st), ['7'] = (st),	\
	['8'] = (st), ['9'] = (st), ['a'] = (st), ['b'] = (st),	\
	['c'] = (st), ['d'] = (st), ['e'] = (st), ['f'] = (st),	\
	['A'] = (st), ['B'] = (st), ['C'] = (st), ['D'] = (st),	\
	['E'] = (st), ['F'] = (st)

/**
 * ckvp_parse() takes the current state (ckvp), a buffer in[size] and returns
 * the number of characters processed.
 *
 * Each time ckvp_parse() returns, ckvp->state must be checked. If no error
 * occured, ckvp_parse() must be called again with the remaining characters
 * if any, otherwise the next input buffer.
 *
 * At the end of input, ckvp_parse() must be called with a zero size.
 *
 * This function doesn't allocate anything.
 *
 * @param[in,out] ckvp Current state.
 * @param size Number of characters in buffer "in".
 * @param in Input buffer to parse.
 * @return Number of characters processed.
 */
size_t ckvp_parse(ckvp_t *ckvp, size_t size, const char in[])
{
	/**
	 * State machine definition:
	 *
	 * st[current_state][current_character] = next state | action
	 *
	 * Special indices for current_character are:
	 *
	 * - 0x100 for action on characters not in the list
	 * - 0x101 for action when encountering end of input while in the
	 *         current state (often ACTION_ERROR)
	 */
	static const unsigned int st[][0x102] = {
		[STATE_ERROR] = {
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_BEGIN] = {
			[' '] = STATE_BEGIN,
			['\f'] = STATE_BEGIN,
			['\n'] = STATE_BEGIN,
			['\r'] = STATE_BEGIN,
			['\t'] = STATE_BEGIN,
			['\v'] = STATE_BEGIN,
			[';'] = (STATE_ERROR | ACTION_ERROR),
			['#'] = STATE_COMMENT,
			['\''] = STATE_KEYSQ,
			['"'] = STATE_KEYDQ,
			['\\'] = STATE_KEYBS,
			['='] = (STATE_ERROR | ACTION_ERROR),
			[0x100] = (STATE_KEY | ACTION_STORE),
			[0x101] = 0
		},
		[STATE_COMMENT] = {
			['\n'] = STATE_BEGIN,
			[0x100] = STATE_COMMENT,
			[0x101] = 0
		},
		[STATE_KEY] = {
			[' '] = (STATE_BEQ | ACTION_KEY),
			['\f'] = (STATE_BEQ | ACTION_KEY),
			['\n'] = (STATE_BEQ | ACTION_KEY),
			['\r'] = (STATE_BEQ | ACTION_KEY),
			['\t'] = (STATE_BEQ | ACTION_KEY),
			['\v'] = (STATE_BEQ | ACTION_KEY),
			['\''] = STATE_KEYSQ,
			['\"'] = STATE_KEYDQ,
			[';'] = (STATE_ERROR | ACTION_ERROR),
			['='] = (STATE_AEQ | ACTION_KEY),
			['#'] = (STATE_ERROR | ACTION_ERROR),
			['\\'] = STATE_KEYBS,
			[0x100] = (STATE_KEY | ACTION_STORE),
			[0x101] = ACTION_ERROR
		},
		[STATE_KEYBS] = {
			['f'] = (STATE_KEY | ACTION_STORE_MOD),
			['n'] = (STATE_KEY | ACTION_STORE_MOD),
			['r'] = (STATE_KEY | ACTION_STORE_MOD),
			['t'] = (STATE_KEY | ACTION_STORE_MOD),
			['v'] = (STATE_KEY | ACTION_STORE_MOD),
			['x'] = STATE_KEYBSX1,
			['\n'] = STATE_KEY,
			[0x100] = (STATE_KEY | ACTION_STORE),
			[0x101] = ACTION_ERROR
		},
		[STATE_KEYBSX1] = {
			HEX_INDICES(STATE_KEYBSX2 | ACTION_STORE_HEX1),
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_KEYBSX2] = {
			HEX_INDICES(STATE_KEY | ACTION_STORE_HEX2),
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_KEYSQ] = {
			['\''] = STATE_KEY,
			[0x100] = (STATE_KEYSQ | ACTION_STORE),
			[0x101] = ACTION_ERROR
		},
		[STATE_KEYDQ] = {
			['"'] = STATE_KEY,
			['\\'] = STATE_KEYDQBS,
			[0x100] = (STATE_KEYDQ | ACTION_STORE),
			[0x101] = ACTION_ERROR
		},
		[STATE_KEYDQBS] = {
			['f'] = (STATE_KEYDQ | ACTION_STORE_MOD),
			['n'] = (STATE_KEYDQ | ACTION_STORE_MOD),
			['r'] = (STATE_KEYDQ | ACTION_STORE_MOD),
			['t'] = (STATE_KEYDQ | ACTION_STORE_MOD),
			['v'] = (STATE_KEYDQ | ACTION_STORE_MOD),
			['x'] = STATE_KEYDQBSX1,
			['\n'] = STATE_KEYDQ,
			[0x100] = (STATE_KEYDQ | ACTION_STORE),
			[0x101] = ACTION_ERROR
		},
		[STATE_KEYDQBSX1] = {
			HEX_INDICES(STATE_KEYDQBSX2 | ACTION_STORE_HEX1),
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_KEYDQBSX2] = {
			HEX_INDICES(STATE_KEYDQ | ACTION_STORE_HEX2),
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_BEQ] = {
			[' '] = STATE_BEQ,
			['\f'] = STATE_BEQ,
			['\n'] = STATE_BEQ,
			['\r'] = STATE_BEQ,
			['\t'] = STATE_BEQ,
			['\v'] = STATE_BEQ,
			['='] = STATE_AEQ,
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_AEQ] = {
			[' '] = STATE_AEQ,
			['\f'] = STATE_AEQ,
			['\n'] = STATE_AEQ,
			['\r'] = STATE_AEQ,
			['\t'] = STATE_AEQ,
			['\v'] = STATE_AEQ,
			['\''] = STATE_VALSQ,
			['\"'] = STATE_VALDQ,
			['\\'] = STATE_VALBS,
			['='] = (STATE_ERROR | ACTION_ERROR),
			['#'] = (STATE_COMMENT | ACTION_VALUE),
			[';'] = (STATE_BEGIN | ACTION_VALUE),
			[0x100] = (STATE_VALUE | ACTION_STORE),
			[0x101] = ACTION_VALUE
		},
		[STATE_VALUE] = {
			[' '] = (STATE_VALEND | ACTION_VALUE),
			['\f'] = (STATE_VALEND | ACTION_VALUE),
			['\n'] = (STATE_BEGIN | ACTION_VALUE),
			['\r'] = (STATE_VALEND | ACTION_VALUE),
			['\t'] = (STATE_VALEND | ACTION_VALUE),
			['\v'] = (STATE_VALEND | ACTION_VALUE),
			['\''] = STATE_VALSQ,
			['\"'] = STATE_VALDQ,
			[';'] = (STATE_BEGIN | ACTION_VALUE),
			['='] = (STATE_ERROR | ACTION_ERROR),
			['#'] = (STATE_COMMENT | ACTION_VALUE),
			['\\'] = STATE_VALBS,
			[0x100] = (STATE_VALUE | ACTION_STORE),
			[0x101] = ACTION_VALUE
		},
		[STATE_VALBS] = {
			['f'] = (STATE_VALUE | ACTION_STORE_MOD),
			['n'] = (STATE_VALUE | ACTION_STORE_MOD),
			['r'] = (STATE_VALUE | ACTION_STORE_MOD),
			['t'] = (STATE_VALUE | ACTION_STORE_MOD),
			['v'] = (STATE_VALUE | ACTION_STORE_MOD),
			['x'] = STATE_VALBSX1,
			['\n'] = STATE_VALUE,
			[0x100] = (STATE_VALUE | ACTION_STORE),
			[0x101] = ACTION_ERROR
		},
		[STATE_VALBSX1] = {
			HEX_INDICES(STATE_VALBSX2 | ACTION_STORE_HEX1),
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_VALBSX2] = {
			HEX_INDICES(STATE_VALUE | ACTION_STORE_HEX2),
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_VALSQ] = {
			['\''] = STATE_VALUE,
			[0x100] = (STATE_VALSQ | ACTION_STORE),
			[0x101] = ACTION_ERROR
		},
		[STATE_VALDQ] = {
			['"'] = STATE_VALUE,
			['\\'] = STATE_VALDQBS,
			[0x100] = (STATE_VALDQ | ACTION_STORE),
			[0x101] = ACTION_ERROR
		},
		[STATE_VALDQBS] = {
			['f'] = (STATE_VALDQ | ACTION_STORE_MOD),
			['n'] = (STATE_VALDQ | ACTION_STORE_MOD),
			['r'] = (STATE_VALDQ | ACTION_STORE_MOD),
			['t'] = (STATE_VALDQ | ACTION_STORE_MOD),
			['v'] = (STATE_VALDQ | ACTION_STORE_MOD),
			['x'] = STATE_VALDQBSX1,
			['\n'] = STATE_VALDQ,
			[0x100] = (STATE_VALDQ | ACTION_STORE),
			[0x101] = ACTION_ERROR
		},
		[STATE_VALDQBSX1] = {
			HEX_INDICES(STATE_VALDQBSX2 | ACTION_STORE_HEX1),
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_VALDQBSX2] = {
			HEX_INDICES(STATE_VALDQ | ACTION_STORE_HEX2),
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = ACTION_ERROR
		},
		[STATE_VALEND] = {
			[' '] = STATE_VALEND,
			['\f'] = STATE_VALEND,
			['\n'] = STATE_BEGIN,
			['\r'] = STATE_VALEND,
			['\t'] = STATE_VALEND,
			['\v'] = STATE_VALEND,
			[';'] = STATE_BEGIN,
			['#'] = STATE_COMMENT,
			[0x100] = (STATE_ERROR | ACTION_ERROR),
			[0x101] = 0
		}
	};
	static const unsigned char cv[] = {
		['f'] = '\f', ['n'] = '\n', ['r'] = '\r',
		['t'] = '\t', ['v'] = '\v'
	};
	static const unsigned char hb[] = {
		['0'] = 0x0, ['1'] = 0x1, ['2'] = 0x2, ['3'] = 0x3,
		['4'] = 0x4, ['5'] = 0x5, ['6'] = 0x6, ['7'] = 0x7,
		['8'] = 0x8, ['9'] = 0x9, ['a'] = 0xa, ['b'] = 0xb,
		['c'] = 0xc, ['d'] = 0xd, ['e'] = 0xe, ['f'] = 0xf,
		['A'] = 0xa, ['B'] = 0xb, ['C'] = 0xc, ['D'] = 0xd,
		['E'] = 0xe, ['F'] = 0xf
	};
	size_t i;

	assert(sizeof(unsigned int) >= 4);
	assert(ckvp != NULL);
	assert(in != NULL);
	if (ckvp->state != CKVP_NONE) {
		ckvp->out_size = 0;
		ckvp->state = CKVP_NONE;
	}
	if (ckvp->internal & 0x00010000) {
		++(ckvp->line);
		ckvp->column = 1;
	}
	else if (ckvp->internal & 0x00020000)
		++(ckvp->column);
	ckvp->internal &= ~(0x00030000);
	if (size == 0) {
		assert((ckvp->internal & 0x00ff) != 0x00);
		assert((ckvp->internal & 0x00ff) <= STATE_VALEND);
		if (st[(ckvp->internal & 0x00ff)][0x101] & ACTION_ERROR)
			ckvp->state = CKVP_ERROR;
		else if (st[(ckvp->internal & 0x00ff)][0x101] & ACTION_VALUE)
			ckvp->state = CKVP_OUT_VALUE;
		return 0;
	}
	for (i = 0; (i < size); ++i) {
		unsigned char c = in[i];
		unsigned int newst;

		assert((ckvp->internal & 0x00ff) != 0x00);
		assert((ckvp->internal & 0x00ff) <= STATE_VALEND);
		if ((newst = st[(ckvp->internal & 0x00ff)][(c & 0xff)]) == 0)
			newst = st[(ckvp->internal & 0x00ff)][0x100];
		ckvp->internal = ((ckvp->internal & 0xffff0000) | newst);
		assert(newst != 0);
		if (newst & 0x0f00) {
			if (newst & ACTION_ERROR)
				ckvp->state = CKVP_ERROR;
			else if (newst & ACTION_KEY)
				ckvp->state = CKVP_OUT_KEY;
			else if (newst & ACTION_VALUE)
				ckvp->state = CKVP_OUT_VALUE;
			goto endnl;
		}
		if (newst & 0xf000) {
			if (newst & ACTION_STORE_HEX1) {
				ckvp->internal &= ~(0x00f00000);
				ckvp->internal |= (hb[c] << 20);
				continue;
			}
			else if (newst & ACTION_STORE_HEX2)
				c = (((ckvp->internal >> 16) & 0xf0) | hb[c]);
			else if (newst & ACTION_STORE_MOD)
				c = cv[c];
			if (ckvp->out_size == CKVP_OUT_SIZE) {
				ckvp->out[0] = c;
				ckvp->out_size = 1;
			}
			else
				ckvp->out[((ckvp->out_size)++)] = c;
			if (ckvp->out_size == CKVP_OUT_SIZE) {
				ckvp->state = CKVP_OUT_FULL;
				goto endnl;
			}
		}
		if (c == '\n') {
			++(ckvp->line);
			ckvp->column = 1;
		}
		else
			++(ckvp->column);
		continue;
	endnl:
		if (c == '\n')
			ckvp->internal |= 0x00010000;
		else
			ckvp->internal |= 0x00020000;
		return ++i;
	}
	return size;
}
