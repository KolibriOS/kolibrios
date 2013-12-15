#include <stdio.h>
#include <stdlib.h>

#include <parserutils/parserutils.h>

#include "input/filter.h"

#include "testutils.h"

static void *myrealloc(void *ptr, size_t len, void *pw)
{
	UNUSED(pw);

	return realloc(ptr, len);
}

int main(int argc, char **argv)
{
	parserutils_filter *input;

	UNUSED(argc);
	UNUSED(argv);

	assert(parserutils__filter_create("UTF-8", myrealloc, NULL, &input) ==
			PARSERUTILS_OK);

	parserutils__filter_destroy(input);

	printf("PASS\n");

	return 0;
}
