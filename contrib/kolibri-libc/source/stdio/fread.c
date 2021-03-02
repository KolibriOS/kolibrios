#include <stdio.h>

size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream) {
	unsigned bytes_read = 0;
	unsigned bytes_count = size * nmemb;
	_ksys_file_read_file(stream->name, stream->position, bytes_count, ptr, &bytes_read);
	stream->position += bytes_read;
	ksys_bdfe_t info;
	// TODO: Handle _ksys_file_get_info error somehow
	if (!_ksys_file_get_info(stream->name, &info)) {
		if (stream->position >= info.size) {
			stream->eof = 1;
		}
	}
	return bytes_read / size;
}
