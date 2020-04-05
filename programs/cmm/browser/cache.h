
struct PAGES_CACHE
{
	dword current_page_buf;
	dword current_page_size;
	collection url;
	collection data; //it has to be int
	collection size; //it has to be int
	void add();
	bool has();
} pages_cache;

void PAGES_CACHE::add(dword _url, _data, _size)
{
	dword data_pointer;
	data_pointer = malloc(_size);
	memmov(data_pointer, _data, _size);
	data.add(itoa(data_pointer));

	url.add(_url);
	size.add(itoa(_size));
}

bool PAGES_CACHE::has(dword _link)
{
	int pos;
	pos = url.get_pos_by_name(_link);
	if (pos != -1) {
		current_page_buf = atoi(data.get(pos));
		current_page_size = atoi(size.get(pos));
		return true;
	}
	return false;
}