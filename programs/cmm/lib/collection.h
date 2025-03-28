#ifndef INCLUDE_COLLECTION_H
#define INCLUDE_COLLECTION_H
#print "[include <collection.h>]\n"

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
	void add();
	dword get();
	dword set();
	void swap();
	dword len();
	dword get_last();
	void pop();
	void drop();
	#define DWSIZE4 4
};

:void collection_int::add(dword _in) {
	unsigned i;
	if (!buf) || (count + 1 * DWSIZE4 >= buf_size) {
		buf_size += 4096 * 5;
		buf = realloc(buf, buf_size);
	}
	i = count * DWSIZE4 + buf;
	ESDWORD[i] = _in;
	count++;
}

:dword collection_int::get(dword pos) {
	if (!buf) || (pos<0) || (pos>=count) return 0;
	return ESDWORD[pos * DWSIZE4 + buf];
}


:dword collection_int::set(dword pos, _in) {
	while (pos >= count) add(0);
	EAX = pos * DWSIZE4 + buf;
	ESDWORD[EAX] = _in;
	return ESDWORD[EAX];
}

:void collection_int::swap(dword pos1, pos2) {
	while (pos1 >= count) add(0);
	while (pos2 >= count) add(0);
	EAX = pos1 * DWSIZE4 + buf;
	EBX = pos2 * DWSIZE4 + buf;
	ESDWORD[EAX] >< ESDWORD[EBX];
}

:dword collection_int::len(dword pos) {
	if (pos<0) || (pos+1>=count) return 0;
	return get(pos+1) - get(pos);
}

:dword collection_int::get_last() {
	return get(count-1);
}

:void collection_int::pop() {
	if (count>0) count--;
}

:void collection_int::drop() {
	count = 0;
	//if (buf) buf = free(buf);
	//buf_size = 0;
}

/*========================================================
=                                                        =
=                       String                           =
=                                                        =
========================================================*/

struct collection
{
	unsigned int realloc_size, count;
	dword data_start;
	dword data_size;
	collection_int offset;
	dword add();
	dword get(); //get_name_by_pos
	dword get_pos_by_name();
	void drop();
	dword get_last();
	bool pop();
};

:dword collection::add(dword in) {
	dword len = strlen(in);
	unsigned cur_buf_size;

	if (!count) {
		cur_buf_size = 0;
	} else {
		cur_buf_size = offset.get(count);
	}

	if (realloc_size<4096) realloc_size = 4096;
	while (cur_buf_size + len + 4 > data_size) {
		data_size += realloc_size;
		data_start = realloc(data_start, data_size);
	}
	
	strncpy(data_start+cur_buf_size, in, len);
	count++;
	offset.set(count, cur_buf_size + len + 1);
	return data_start + cur_buf_size;
}

:dword collection::get(dword pos) {
	if (pos<0) || (pos>=count) return 0;
	return data_start + offset.get(pos);
}

:dword collection::get_last() {
	return get(count-1);
}

:dword collection::get_pos_by_name(dword name) {
	dword i;
	for (i=0; i<count; i++) {
		if (streq(data_start + offset.get(i), name)) {
			return i;
		}
	}
	return -1;
}

:void collection::drop() {
	if (data_start) free(data_start);
	data_size = 0;
	data_start = 0;
	offset.drop();
	count = 0;
}

:bool collection::pop() {
	if (count>0) count--;
}

#endif