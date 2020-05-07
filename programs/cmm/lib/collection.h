#ifndef INCLUDE_COLLECTION_H
#define INCLUDE_COLLECTION_H
#print "[include <collection.h>]\n"

/*========================================================
=                                                        =
=                       String                           =
=                                                        =
========================================================*/

struct collection
{
	int realloc_size, count;
	dword data_start;
	dword data_size;
	dword element_offset[4000];
	int add();
	int addn();
	dword get(); //get_name_by_pos
	dword get_pos_by_name();
	void drop();
	void increase_data_size();
	dword get_last();
	bool pop();
};

:void collection::increase_data_size() {
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

:int collection::add(dword in) {
	return addn(in, strlen(in));
}

:int collection::addn(dword in, len) {
	if (count >= 4000) {
		debugln("collection: more than 4000 elements!");
		return 0;
	}
	if (element_offset[count]+len+2 > data_size) {
		increase_data_size();
		addn(in, len);
		return 1;
	}
	strncpy(data_start+element_offset[count], in, len);
	count++;
	element_offset[count] = element_offset[count-1] + len + 1;
	return 1;
}

:dword collection::get(dword pos) {
	if (pos<0) || (pos>=count) return 0;
	return data_start + element_offset[pos];
}

:dword collection::get_last() {
	return get(count-1);
}

:dword collection::get_pos_by_name(dword name) {
	dword i;
	for (i=0; i<count; i++) {
		if (strcmp(data_start + element_offset[i], name)==0) return i;
	}
	return -1;
}

:void collection::drop() {
	if (data_start) free(data_start);
	data_size = 0;
	data_start = 0;
	element_offset[count] = 0;
	count = 0;
}

:bool collection::pop() {
	if (count>0) count--;
}

/*========================================================
=                                                        =
=                       Integer                          =
=                                                        =
========================================================*/

struct collection_int
{
	dword buf;
	dword buf_size;
	unsigned count;
	void alloc();
	void add();
	dword get();
	dword get_last();
	void pop();
	void drop();
};

:void collection_int::alloc() {
	if (!buf) {
		buf_size = 4096;
		buf = malloc(4096);
	} else {
		buf_size += 4096;
		buf = realloc(buf, buf_size);
	}
}

:void collection_int::add(dword _in) {
	if (!buf) || (count * sizeof(dword) >= buf_size) alloc();
	EAX = count * sizeof(dword) + buf;
	ESDWORD[EAX] = _in;
	count++;
}

:dword collection_int::get(dword pos) {
	if (pos<0) || (pos>=count) return 0;
	return ESDWORD[pos * sizeof(dword) + buf];
}

:dword collection_int::get_last() {
	return get(count-1);
}

:void collection_int::pop() {
	if (count>0) count--;
}

:void collection_int::drop() {
	count = 0;
}

#endif