#include <stdio.h>

size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream) {
	unsigned bytes_read = 0;
	unsigned bytes_count = size * nmemb;

	for (size_t i = 0; i < bytes_count; i++) {
		char c = fgetc(stream);

		if (c == EOF) {
			break;
		}

		*(char*)(ptr+i) = c;

		bytes_read++;
	}

	return bytes_read / size;
}
