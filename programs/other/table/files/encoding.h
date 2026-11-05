#ifndef FILES_ENCODING_H
#define FILES_ENCODING_H

// cp866, of the cells and of the titles the kernel and @notify show, and
// UTF-8: of cp866 the Cyrillic letters, as the kernel converts them
// (fs/parse_fn.inc); a character the other has not becomes '?'

// a UTF-8 text in cp866 into out of size bytes, ended by a zero; out may
// be s: it is never longer
void utf8_to_cp866(const char *s, char *out, int size);

// a cp866 character in UTF-8 into out, 1 or 2 bytes: how many
int cp866_to_utf8(unsigned char c, char *out);

// the text from s on is UTF-8 of characters cp866 has: ASCII is too. A
// text with others is taken for cp866 and keeps its bytes as they are:
// short words of cp866 such as "так" are UTF-8 of other characters
int is_utf8(const char *s);

#endif
