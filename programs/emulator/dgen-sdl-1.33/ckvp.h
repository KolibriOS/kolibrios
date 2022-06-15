/**
 * @file
 * Simple parser for "key = value" text files, by zamaz.
 *
 * Can also manage the following weird cases:
 *
 * @code
 *   # comment
 *   # empty lines are ignored
 *   foo = bar # comment
 *   b\
 *   lah = blah ; rah=meh
 *   'with spaces' = "with\ncr"
 *
 *   \x2a = 'do not eat \x2a in simple quotes,'" here we can \x2a"
 *
 *   # EOF
 * @endcode
 */

#ifndef CKVP_H_
#define CKVP_H_

#include <stddef.h>

#ifdef __cplusplus
#define CKVP_DECL_BEGIN__	extern "C" {
#define CKVP_DECL_END__		}
#else
#define CKVP_DECL_BEGIN__
#define CKVP_DECL_END__
#endif

CKVP_DECL_BEGIN__

#define CKVP_OUT_SIZE		511
#define CKVP_OUT_SIZE_STR	"511"
#define CKVP_INIT		{ CKVP_NONE, 1, 1, 0x0002, 0, "" }

/**
 * ckvp_t stores the state, it must be initialized with CKVP_INIT before
 * being fed to ckvp_parse().
 *
 * When "state" reaches CKVP_OUT_FULL, "out_size" is equal to CKVP_OUT_SIZE
 * ("out" is full), and ckvp_parse() won't process any more characters until
 * "out_size" is reset.
 */
typedef struct {
	unsigned int state;    /**< Parsing status */
	unsigned int line;     /**< Current line */
	unsigned int column;   /**< Current column */
	unsigned int internal; /**< Internal state */
	size_t out_size;       /**< Number of characters in "out" */
	/**
	 * Output buffer. Be careful, this buffer not NUL-terminated.
	 */
	char out[(CKVP_OUT_SIZE + 1)]; /* hack: this +1 saves room for \0 */
} ckvp_t;

/**
 *  Possible states (previously defined in an enum, but C++ didn't like it)
 */
#define CKVP_NONE      0 /**< Nothing to report */
#define CKVP_OUT_FULL  1 /**< Buffer "out" is full, must be flushed */
#define CKVP_OUT_KEY   2 /**< Everything stored until now is a key */
#define CKVP_OUT_VALUE 3 /**< Everything stored until now is a value */
#define CKVP_ERROR     4 /**< Error status, check line/column */

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
extern size_t ckvp_parse(ckvp_t *ckvp, size_t size, const char in[]);

CKVP_DECL_END__

#endif /* CKVP_H_ */
