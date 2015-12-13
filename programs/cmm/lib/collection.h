#ifndef INCLUDE_COLLECTION_H
#define INCLUDE_COLLECTION_H
#print "[include <collection.h>]\n"

struct collection
{
	int count;
	dword element_offset[4096];
	dword data_size;
	dword string_data_start;
	dword string_data_cur_pos;
	void add();
	dword get();
	void drop();
	void init();

};

void collection::init(dword size) {
	if (data_size) drop();
	data_size = data_size + size;
	string_data_cur_pos = string_data_start = malloc(data_size);
	count = 0;
}

void collection::add(dword in) {
	strcpy(string_data_cur_pos, in);
	element_offset[count] = string_data_cur_pos;
	string_data_cur_pos += strlen(in) + 1;
	count++;
}

dword collection::get(dword pos) {
	return element_offset[pos];
}

void collection::drop() {
	if (string_data_start) free(string_data_start);
	data_size =
	string_data_start =
	string_data_cur_pos =
	count = 0;
}

#endif