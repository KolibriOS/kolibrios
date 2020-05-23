
struct PAGES_CACHE
{
	dword current_page_buf;
	dword current_page_size;
	collection url;
	collection_int data;
	collection_int size;
	void add();
	bool has();
	void clear();
} pages_cache=0;

void PAGES_CACHE::add(dword _url, _data, _size)
{
	dword data_pointer;
	data_pointer = malloc(_size);
	memmov(data_pointer, _data, _size);
	data.add(data_pointer);

	url.add(_url);
	size.add(_size);
}

bool PAGES_CACHE::has(dword _link)
{
	int pos;
	pos = url.get_pos_by_name(_link);
	if (pos != -1) {
		current_page_buf = data.get(pos);
		current_page_size = size.get(pos);
		return true;
	}
	return false;
}

void PAGES_CACHE::clear()
{
	url.drop();
	data.drop();
	size.drop();
	current_page_buf = NULL;
	current_page_size = NULL;
}