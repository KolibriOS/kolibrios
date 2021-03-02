#include <stdio.h>

size_t fwrite(const void * restrict ptr,size_t size, size_t nmemb,FILE * restrict stream) {
	unsigned bytes_written = 0;
	unsigned bytes_count = size * nmemb;
	_ksys_file_write_file(stream->name, stream->position, bytes_count, ptr, &bytes_written);
	stream->position += bytes_written;
	ksys_bdfe_t info;
	// TODO: Handle _ksys_file_get_info error somehow
	if (!_ksys_file_get_info(stream->name, &info)) {
		if (stream->position >= info.size) {
			stream->eof = 1;
		}
	}
	return bytes_written / size;
}
