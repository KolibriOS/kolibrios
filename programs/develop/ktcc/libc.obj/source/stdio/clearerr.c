#include <stdio.h>

void clearerr(FILE* stream)
{
	stream->flags.val &= ~(_FILEFLAG_ERR | _FILEFLAG_EOF);
}
