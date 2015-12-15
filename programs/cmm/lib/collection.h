#ifndef INCLUDE_COLLECTION_H
#define INCLUDE_COLLECTION_H
#print "[include <collection.h>]\n"

struct collection
{
	int realloc_size, count;
	dword data_start;
	dword data_cur_pos;
	dword data_size;
	dword element_offset[4090];
	int add();
	dword get();
	void drop();
	void increase_data_size();

};

void collection::increase_data_size() {
	int filled_size;
	if (realloc_size<4096) realloc_size = 4096;
	if (!data_size) {
		data_size = realloc_size;
		data_start = malloc(realloc_size);		
	}
	else {
		data_size = data_size + realloc_size;
		data_start = realloc(data_start, data_size);
	}
}

int collection::add(dword in) {
	if (count >= 4090) return 0;
	if (data_cur_pos+strlen(in)+2 > data_size) {
		increase_data_size();
		add(in);
		return;
	}
	strcpy(data_start+data_cur_pos, in);
	element_offset[count] = data_cur_pos;
	data_cur_pos += strlen(in) + 1;
	count++;
	return 1;
}

dword collection::get(dword pos) {
	return data_start + element_offset[pos];
}

void collection::drop() {
	if (data_start) free(data_start);
	data_size = data_start = data_cur_pos = count = 0;
}

#endif