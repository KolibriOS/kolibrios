#ifndef PROMPT_H_
#define PROMPT_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
#define PROMPT_DECL_BEGIN__ extern "C" {
#define PROMPT_DECL_END__ }
#else
#define PROMPT_DECL_BEGIN__
#define PROMPT_DECL_END__
#endif

#define PROMPT_HIST_SIZE 64
#define PROMPT_LINE_SIZE 512

PROMPT_DECL_BEGIN__

struct prompt {
	unsigned int cursor; /* cursor position for current line */
	unsigned int current; /* current history[] entry */
	unsigned int entries; /* number of entries in history[] */
	struct prompt_history {
		unsigned int length; /* line length */
		uint8_t line[PROMPT_LINE_SIZE]; /* line data */
	} history[PROMPT_HIST_SIZE];
};

struct prompt_parse {
	unsigned int index; /* current index in argv */
	unsigned int cursor; /* cursor position for current index or ~0u */
	unsigned int argc; /* number of elements in argv */
	uint8_t **argv; /* NULL-terminated array of pointers to each word */
	unsigned int *args; /* size of each argv[] element */
	struct prompt_parse_argo {
		unsigned int pos;
		unsigned int len;
	} *argo; /* position and length of each element in original string */
};

extern void prompt_init(struct prompt *p);
extern void prompt_push(struct prompt *p);
extern void prompt_older(struct prompt *p);
extern void prompt_newer(struct prompt *p);
extern void prompt_left(struct prompt *p);
extern void prompt_right(struct prompt *p);
extern void prompt_begin(struct prompt *p);
extern void prompt_end(struct prompt *p);
extern void prompt_clear(struct prompt *p);
extern void prompt_put(struct prompt *p, uint8_t c);
extern void prompt_delete(struct prompt *p);
extern void prompt_backspace(struct prompt *p);
extern void prompt_replace(struct prompt *p, unsigned int pos,
			   unsigned int len, const uint8_t *with,
			   unsigned int with_len);
extern struct prompt_parse *prompt_parse(struct prompt *p,
					 struct prompt_parse *pp);
extern void prompt_parse_clean(struct prompt_parse *pp);

PROMPT_DECL_END__

#endif /* PROMPT_H_ */
