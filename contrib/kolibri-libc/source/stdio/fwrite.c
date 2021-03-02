#include <stdio.h>

size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream) {
	unsigned bytes_written = 0;
	unsigned bytes_count = size * nmemb;

	for (size_t i = 0; i < bytes_count; i++) {
		char c = *(char*)(ptr+i);
		if (fputc(c, stream) != c) {
			break;
		}

		bytes_written++;
	}

	return bytes_written / size;
}
